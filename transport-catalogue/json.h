/*
 * Место для вашей JSON-библиотеки
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    Node() = default;
 
    template <typename Type>
    Node(Type value) : value_(value) {}
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const Value& GetValue() const;
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const; 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool operator==(const Node& node_) const;
    bool operator!=(const Node& node_) const;

private:
    Value value_;
};
    
class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;
    
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json