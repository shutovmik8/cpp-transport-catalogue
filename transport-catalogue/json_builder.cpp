#include "json_builder.h"

namespace json {

Builder::Builder() {
    root_ = Node(nullptr);
    nodes_stack_.push_back(&root_);
}  
    
Builder::KeyItemContext Builder::Key(std::string str) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Two times use Value()");
    }
    if (!nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Key() for not Map Node");
    }
    nodes_stack_.push_back(&(std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(str)] = Node(nullptr)));
    return KeyItemContext(*this);
}
    
Builder& Builder::Value(Node::Value value) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Two times use Value()");
    }
    else if (nodes_stack_.back()->IsNull()) {
        *nodes_stack_.back() = Node(std::move(value));
        nodes_stack_.pop_back();
        return *this;
    }
    else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(Node(std::move(value)));
        return *this;
    }
    throw std::logic_error("Invalid use of Value()");
}
    
Builder::DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Usage StartDict() with complite Node");
    }
    else if (nodes_stack_.back()->IsNull()) {
        *nodes_stack_.back() = Node(Dict());
        return DictItemContext(*this);
    }
    else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Dict());
        nodes_stack_.push_back(&(std::get<Array>(nodes_stack_.back()->GetValue()).back()));
        return DictItemContext(*this);
    }
    throw std::logic_error("Invalid use of StartDict()");
}
    
Builder& Builder::EndDict() {
    if ((nodes_stack_.empty()) or (!nodes_stack_.back()->IsMap())) {
        throw std::logic_error("Usage EndDict() without StartDict()");
    }
    nodes_stack_.pop_back();
    return *this;
}
    
Builder::ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Usage StartArray() with complite Node");
    }
    else if (nodes_stack_.back()->IsNull()) {
        *nodes_stack_.back() = Node(Array());
        return ArrayItemContext(*this);
    }
    else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Array());
        nodes_stack_.push_back(&(std::get<Array>(nodes_stack_.back()->GetValue()).back()));
        return ArrayItemContext(*this);
    }
    throw std::logic_error("Invalid use of StartArray()");
}
    
Builder& Builder::EndArray() {

    if ((nodes_stack_.empty()) or (!nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Usage EndArray() without StartArray()");
    }
    nodes_stack_.pop_back();
    return *this;
}
    
json::Node Builder::Build() {  
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Invalid use of Build()");
    }
    return root_;
}

Builder::KeyItemContext Builder::DictItemContext::Key(std::string str) {
    return builder_.Key(std::move(str));
}
    
Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDict();
}
    
Builder::ValueAfterKeyItemContext Builder::KeyItemContext::Value(Node::Value value) {
    return ValueAfterKeyItemContext(builder_.Value(std::move(value)));
}
    
Builder::DictItemContext Builder::KeyItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
    return builder_.StartArray();
}    

Builder::KeyItemContext Builder::ValueAfterKeyItemContext::Key(std::string str) {
    return builder_.Key(std::move(str));
}
    
Builder& Builder::ValueAfterKeyItemContext::EndDict() {
    return builder_.EndDict();
}
    
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return ArrayItemContext(builder_.Value(std::move(value)));
}
    
Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_.StartDict();
}
    
Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_.StartArray();
}
    
Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArray();
}    
    
}