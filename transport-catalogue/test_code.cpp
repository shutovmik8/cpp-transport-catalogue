#include <iostream>
#include <string>

using namespace std::literals;

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


int main() {
    std::string line;
    std::string str = "";
    char c;
    while(std::getline(std::cin, line)) {
        str += line;
        str += '\n';
        line.clear();
    }
    PrintString(str, std::cout);
}