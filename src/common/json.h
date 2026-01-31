#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <utility>
#include <type_traits>

namespace NJson {

class TJsonNode {
public:
    enum EType {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    using TArray = std::vector<TJsonNode>;
    using TObject = std::unordered_map<std::string, TJsonNode>;

public:
    // Default constructors
    TJsonNode();
    TJsonNode(std::nullptr_t);
    
    // Primitive type constructors
    TJsonNode(bool value);
    TJsonNode(int value);
    TJsonNode(long value);
    TJsonNode(long long value);
    TJsonNode(unsigned int value);
    TJsonNode(unsigned long value);
    TJsonNode(unsigned long long value);
    TJsonNode(float value);
    TJsonNode(double value);
    TJsonNode(const char* value);
    TJsonNode(const std::string& value);
    TJsonNode(std::string&& value);
    
    // Container constructors
    template <typename Type>
    TJsonNode(const std::vector<Type>& value);
    
    template <typename Type>
    TJsonNode(std::vector<Type>&& value);
    
    template <typename Type>
    TJsonNode(const std::unordered_map<std::string, Type>& value);
    
    template <typename Type>
    TJsonNode(std::unordered_map<std::string, Type>&& value);
    
    // Copy and move semantics
    TJsonNode(const TJsonNode& other);
    TJsonNode(TJsonNode&& other) noexcept;
    TJsonNode& operator=(const TJsonNode& other);
    TJsonNode& operator=(TJsonNode&& other) noexcept;
    
    ~TJsonNode();
    
    // Type queries
    EType GetType() const;
    bool IsNull() const;
    bool IsBoolean() const;
    bool IsNumber() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsObject() const;
    
    // Container operations
    size_t size() const;
    bool empty() const;
    bool contains(const std::string& key) const;
    void clear();
    
    // Access methods with bounds checking
    TJsonNode& at(const std::string& key);
    const TJsonNode& at(const std::string& key) const;
    TJsonNode& at(size_t idx);
    const TJsonNode& at(size_t idx) const;
    
    // Operator[] access (creates element if not exists for objects)
    TJsonNode& operator[](const std::string& key);
    TJsonNode& operator[](size_t idx);
    const TJsonNode& operator[](size_t idx) const;
    
    // Emplace for objects (implementation defined after class)
    template <typename Type>
    auto emplace(const std::string& key, const Type& value);
    
    template <typename Type>
    auto emplace(const std::string& key, Type&& value);
    
    auto emplace(const std::string& key, const TJsonNode& value);
    auto emplace(const std::string& key, TJsonNode&& value);
    
    // Emplace for arrays
    template <typename Type>
    void emplace_back(const Type& value);
    
    template <typename Type>
    void emplace_back(Type&& value);
    
    void emplace_back(const TJsonNode& value);
    void emplace_back(TJsonNode&& value);
    
    // Insert for objects (implementation defined after class)
    template <typename Type>
    auto insert(const std::string& key, const Type& value);
    
    template <typename Type>
    auto insert(const std::string& key, Type&& value);
    
    auto insert(const std::string& key, const TJsonNode& value);
    auto insert(const std::string& key, TJsonNode&& value);
    
    // Insert for arrays
    template <typename Type>
    void push_back(const Type& value);
    
    template <typename Type>
    void push_back(Type&& value);
    
    void push_back(const TJsonNode& value);
    void push_back(TJsonNode&& value);
    
    // Erase operations
    bool erase(const std::string& key);
    void erase(size_t idx);
    
    // Conversion operators
    explicit operator bool() const;
    explicit operator int() const;
    explicit operator long() const;
    explicit operator long long() const;
    explicit operator unsigned int() const;
    explicit operator unsigned long() const;
    explicit operator unsigned long long() const;
    explicit operator float() const;
    explicit operator double() const;
    explicit operator std::string() const;
    
    template <typename Type>
    explicit operator std::vector<Type>() const;
    
    template <typename Type>
    explicit operator std::unordered_map<std::string, Type>() const;

    // Serialization
    std::string ToString(bool pretty = false, int indent = 0) const;
    
    // Parsing
    static TJsonNode Parse(const std::string& json);
    static TJsonNode Parse(const char* json);

private:
    // Helper methods for serialization
    void ToStringImpl(std::string& result, bool pretty, int indent) const;
    static std::string EscapeString(const std::string& str);
    
    // Helper methods for parsing
    static TJsonNode ParseValue(const char*& ptr);
    static TJsonNode ParseObject(const char*& ptr);
    static TJsonNode ParseArray(const char*& ptr);
    static TJsonNode ParseString(const char*& ptr);
    static TJsonNode ParseNumber(const char*& ptr);
    static TJsonNode ParseLiteral(const char*& ptr, const char* literal, TJsonNode value);
    static void SkipWhitespace(const char*& ptr);
    static std::string UnescapeString(const std::string& str);

private:
    void Clear();
    void CopyFrom(const TJsonNode& other);
    
    template <typename Type>
    void InitFromVector(const std::vector<Type>& vec);
    
    template <typename Type>
    void InitFromVector(std::vector<Type>&& vec);
    
    template <typename Type>
    void InitFromMap(const std::unordered_map<std::string, Type>& map);
    
    template <typename Type>
    void InitFromMap(std::unordered_map<std::string, Type>&& map);

private:
    EType Type_;
    
    union TValue {
        bool Boolean;
        double Number;
        std::string* String;
        TArray* Array;
        TObject* Object;
        
        TValue();
        ~TValue();
    } Value_;
};

} // namespace NJson

#include "json_impl.h"
