#pragma once

#include "json.h"

namespace json {
    
class Builder {
    Node root_{};
    std::vector<Node*> nodes_stack_;
    
    class DictItemContext;
    class KeyItemContext;
    class ValueAfterKeyItemContext;
    class ArrayItemContext;

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

class Builder::DictItemContext {
    Builder& builder_;
    
public:
    DictItemContext(Builder& builder) : builder_(builder) {}
    KeyItemContext Key(std::string str);
    Builder& EndDict();
};
    
class Builder::KeyItemContext {
    Builder& builder_;
    
public:
    KeyItemContext(Builder& builder) : builder_(builder) {}
    
    ValueAfterKeyItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};
    
class Builder::ValueAfterKeyItemContext {
    Builder& builder_;
    
public:
    ValueAfterKeyItemContext(Builder& builder) : builder_(builder) {}
    
    KeyItemContext Key(std::string str);
    Builder& EndDict();
};
 
class Builder::ArrayItemContext {
    Builder& builder_;
    
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}
    
    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};
    
}