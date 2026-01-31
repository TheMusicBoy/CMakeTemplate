#pragma once

#include <common/exception.h>

namespace NJson {

// Container constructors
template <typename Type>
TJsonNode::TJsonNode(const std::vector<Type>& value)
    : Type_(Array)
{
    Value_.Array = new TArray();
    InitFromVector(value);
}

template <typename Type>
TJsonNode::TJsonNode(std::vector<Type>&& value)
    : Type_(Array)
{
    Value_.Array = new TArray();
    InitFromVector(std::move(value));
}

template <typename Type>
TJsonNode::TJsonNode(const std::unordered_map<std::string, Type>& value)
    : Type_(Object)
{
    Value_.Object = new TObject();
    InitFromMap(value);
}

template <typename Type>
TJsonNode::TJsonNode(std::unordered_map<std::string, Type>&& value)
    : Type_(Object)
{
    Value_.Object = new TObject();
    InitFromMap(std::move(value));
}

// Emplace for objects
template <typename Type>
auto TJsonNode::emplace(const std::string& key, const Type& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->emplace(key, TJsonNode(value));
}

template <typename Type>
auto TJsonNode::emplace(const std::string& key, Type&& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->emplace(key, TJsonNode(std::forward<Type>(value)));
}

// Emplace for arrays
template <typename Type>
void TJsonNode::emplace_back(const Type& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->emplace_back(TJsonNode(value));
}

template <typename Type>
void TJsonNode::emplace_back(Type&& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->emplace_back(TJsonNode(std::forward<Type>(value)));
}

// Insert for objects
template <typename Type>
auto TJsonNode::insert(const std::string& key, const Type& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto [it, inserted] = Value_.Object->insert({key, TJsonNode(value)});
    return std::pair{it, inserted};
}

template <typename Type>
auto TJsonNode::insert(const std::string& key, Type&& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto [it, inserted] = Value_.Object->insert({key, TJsonNode(std::forward<Type>(value))});
    return std::pair{it, inserted};
}

// Push back for arrays
template <typename Type>
void TJsonNode::push_back(const Type& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->push_back(TJsonNode(value));
}

template <typename Type>
void TJsonNode::push_back(Type&& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->push_back(TJsonNode(std::forward<Type>(value)));
}

// Conversion operators
template <typename Type>
TJsonNode::operator std::vector<Type>() const {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    
    std::vector<Type> result;
    result.reserve(Value_.Array->size());
    
    for (const auto& item : *Value_.Array) {
        result.push_back(static_cast<Type>(item));
    }
    
    return result;
}

template <typename Type>
TJsonNode::operator std::unordered_map<std::string, Type>() const {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    
    std::unordered_map<std::string, Type> result;
    
    for (const auto& [key, value] : *Value_.Object) {
        result.emplace(key, static_cast<Type>(value));
    }
    
    return result;
}

// Private helper methods
template <typename Type>
void TJsonNode::InitFromVector(const std::vector<Type>& vec) {
    Value_.Array->reserve(vec.size());
    for (const auto& item : vec) {
        Value_.Array->emplace_back(TJsonNode(item));
    }
}

template <typename Type>
void TJsonNode::InitFromVector(std::vector<Type>&& vec) {
    Value_.Array->reserve(vec.size());
    for (auto&& item : vec) {
        Value_.Array->emplace_back(TJsonNode(std::move(item)));
    }
}

template <typename Type>
void TJsonNode::InitFromMap(const std::unordered_map<std::string, Type>& map) {
    for (const auto& [key, value] : map) {
        Value_.Object->emplace(key, TJsonNode(value));
    }
}

template <typename Type>
void TJsonNode::InitFromMap(std::unordered_map<std::string, Type>&& map) {
    for (auto&& [key, value] : map) {
        Value_.Object->emplace(key, TJsonNode(std::move(value)));
    }
}

} // namespace NJson
