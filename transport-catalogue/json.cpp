#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

void CheckInput(istream& input) {
    if (!input) {
        throw ParsingError("Failed to read number from stream"s);
    }
}
    
char CharNoSpace(istream& input) {
    char c;
    while (input >> c) {
        CheckInput(input);
        if ((c != ' ') && (c != '\r') && (c != '\n') && (c != '\t')) {
            break;
        } 
    }
    return c;
}
    
Node LoadArray(istream& input) {
    Array result;
    for (char c; (input >> c) && (input) && c != ']';) {
        CheckInput(input);
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    CheckInput(input);
    return Node(move(result));
}

Node LoadNull(istream& input) {
    char c;
    string str{""};
    for (int i = 0; i < 3; ++i) {
        c = CharNoSpace(input);
        CheckInput(input);
        str += c;
    }
    if (str == "ull") {
        c = CharNoSpace(input);
        if (!input) {
            return Node();
        }
        if ((c != ']') && (c != '}') && (c != ',')) {
            throw ParsingError("Failed to read number from stream"s);
        }
        input.putback(c);
        return Node();
    }
    throw ParsingError("Wrong name!"s);
}

bool LoadBool(istream& input) {
    char c;
    string str{""};
    for (int i = 0; i < 4; ++i) {
        c = CharNoSpace(input);
        CheckInput(input);
        str += c;
    }
    if (str == "true") {
        c = CharNoSpace(input);
        if (!input) {
            return true;
        }
        if ((c != ']') && (c != '}') && (c != ',')) {
            throw ParsingError("Failed to read number from stream"s);
        }
        input.putback(c);
        return true;
    }
    else if (str == "fals") {
        c = CharNoSpace(input);
        CheckInput(input);
        str += c;
    }
    else {
        throw ParsingError("Wrong name!"s);
    }
    if (str == "false") {
        c = CharNoSpace(input);
        if (!input) {
            return false;
        }
        if ((c != ']') && (c != '}') && (c != ',')) {
            throw ParsingError("Failed to read number from stream"s);
        }
        input.putback(c);
        return false;
    }
    throw ParsingError("Wrong name!"s);
}
    
using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;
    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        CheckInput(input);
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}    
    
Node LoadDict(istream& input) {
    Dict result;

    for (char c; (input >> c) && (input) && (c != '}');) {
        CheckInput(input);
        if (c == ',') {
            c = CharNoSpace(input);
        }

        string key = LoadString(input);
        c = CharNoSpace(input);
        result.insert({move(key), LoadNode(input)});
    }
    CheckInput(input);
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    c = CharNoSpace(input);
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node(LoadString(input));
    } else if (c == 'n') {
        return Node(LoadNull(input));
    } else if ((c == 't') or (c == 'f')) {
        input.putback(c);
        return Node(LoadBool(input));
    } else {
        input.putback(c);
        auto tmp = LoadNumber(input);
        const auto* root1 = get_if<int>(&tmp);
        if (root1) {
            return Node(*root1);
        }
        else {
            return Node(*(get_if<double>(&tmp)));
        }
    }
}

}  // namespace

const Node::Value& Node::GetValue() const {
    return value_;
}
    
int Node::AsInt() const {
    if (const auto* root = get_if<int>(&value_)) {
        return *root;
    }
    else {
        throw std::logic_error("wrong type!");
    }
}
    
bool Node::AsBool() const {
    if (const auto* root = get_if<bool>(&value_)) {
        return *root;
    }
    else {
        throw std::logic_error("wrong type!");
    }
}
    
double Node::AsDouble() const {
    const auto* root1 = get_if<double>(&value_);
    const auto* root2 = get_if<int>(&value_);
    if (root1) {
        return *root1;
    }
    else if (root2) {
        return *root2;
    }
    throw std::logic_error("wrong type!");
}

const string& Node::AsString() const {
    if (const auto* root = get_if<string>(&value_)) {
        return *root;
    }
    else {
        throw std::logic_error("wrong type!");
    }
}
    
const Array& Node::AsArray() const {
    if (const auto* root = get_if<Array>(&value_)) {
        return *root;
    }
    else {
        throw std::logic_error("wrong type!");
    }
}

const Dict& Node::AsMap() const {
    if (const auto* root = get_if<Dict>(&value_)) {
        return *root;
    }
    else {
        throw std::logic_error("wrong type!");
    }
}

bool Node::IsInt() const {
    const auto* root = get_if<int>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}
    
bool Node::IsDouble() const {
    const auto* root1 = get_if<double>(&value_);
    const auto* root2 = get_if<int>(&value_);
    if ((root1) or (root2)) {
        return true;
    }
    else {
        return false;
    }
}
   
bool Node::IsPureDouble() const {
    const auto* root = get_if<double>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsBool() const {
    const auto* root = get_if<bool>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}
 
bool Node::IsString() const {
    const auto* root = get_if<string>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}
 
bool Node::IsNull() const {
    const auto* root = get_if<nullptr_t>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}
    
bool Node::IsArray() const {
    const auto* root = get_if<Array>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}
    
bool Node::IsMap() const {
    const auto* root = get_if<Dict>(&value_);
    if (root) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::operator==(const Node& node_) const {
    return value_ == node_.value_;
}
    
bool Node::operator!=(const Node& node_) const {
    return !(*this == node_);
}
    
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return GetRoot() == other.GetRoot();
}
    
bool Document::operator!=(const Document& other) const {
    return GetRoot() != other.GetRoot();
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// В специализации шаблона PrintValue для типа bool параметр value передаётся
// по константной ссылке, как и в основном шаблоне.
// В качестве альтернативы можно использовать перегрузку:
// void PrintValue(bool value, const PrintContext& ctx);
template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) {
            PrintValue(value, ctx);
        },
        node.GetValue());
}
    
void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output, 4 , 0});
    output << "\n";
}

}  // namespace json