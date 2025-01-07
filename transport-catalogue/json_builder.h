#pragma once

#include "json.h"

namespace json {

class DictItemContext;
class KeyItemContext;
class ValueAfterKeyItemContext;
class ArrayItemContext;
    
class Builder {
    Node root_{};
    std::vector<Node*> nodes_stack_;
    
public:
    Builder();
    KeyItemContext Key(std::string);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    json::Node Build();
};  

class DictItemContext {
    Builder& builder_;
    
public:
    DictItemContext(Builder& builder) : builder_(builder) {}
    KeyItemContext Key(std::string str);
    Builder& EndDict();
};
    
class KeyItemContext {
    Builder& builder_;
    
public:
    KeyItemContext(Builder& builder) : builder_(builder) {}
    
    ValueAfterKeyItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};
    
class ValueAfterKeyItemContext {
    Builder& builder_;
    
public:
    ValueAfterKeyItemContext(Builder& builder) : builder_(builder) {}
    
    KeyItemContext Key(std::string str);
    Builder& EndDict();
};
 
class ArrayItemContext {
    Builder& builder_;
    
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}
    
    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};
    
}