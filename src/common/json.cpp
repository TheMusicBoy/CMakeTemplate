#include "json.h"

#include <common/exception.h>
#include <cstring>
#include <cstdio>

namespace NJson {

// TJsonNode::TValue implementation
TJsonNode::TValue::TValue()
    : Number(0.0)
{
}

TJsonNode::TValue::~TValue() {
    // Cleanup is handled by TJsonNode::Clear()
}

// Default constructors
TJsonNode::TJsonNode()
    : Type_(Null)
{
    Value_.Number = 0.0;
}

TJsonNode::TJsonNode(std::nullptr_t)
    : Type_(Null)
{
    Value_.Number = 0.0;
}

// Primitive type constructors
TJsonNode::TJsonNode(bool value)
    : Type_(Boolean)
{
    Value_.Boolean = value;
}

TJsonNode::TJsonNode(int value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(long value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(long long value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(unsigned int value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(unsigned long value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(unsigned long long value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(float value)
    : Type_(Number)
{
    Value_.Number = static_cast<double>(value);
}

TJsonNode::TJsonNode(double value)
    : Type_(Number)
{
    Value_.Number = value;
}

TJsonNode::TJsonNode(const char* value)
    : Type_(String)
{
    Value_.String = new std::string(value);
}

TJsonNode::TJsonNode(const std::string& value)
    : Type_(String)
{
    Value_.String = new std::string(value);
}

TJsonNode::TJsonNode(std::string&& value)
    : Type_(String)
{
    Value_.String = new std::string(std::move(value));
}

TJsonNode::~TJsonNode() {
    Clear();
}

TJsonNode::TJsonNode(const TJsonNode& other)
    : Type_(Null)
{
    CopyFrom(other);
}

TJsonNode::TJsonNode(TJsonNode&& other) noexcept
    : Type_(other.Type_)
    , Value_(other.Value_)
{
    other.Type_ = Null;
    other.Value_.Number = 0.0;
}

TJsonNode& TJsonNode::operator=(const TJsonNode& other) {
    if (this != &other) {
        Clear();
        CopyFrom(other);
    }
    return *this;
}

TJsonNode& TJsonNode::operator=(TJsonNode&& other) noexcept {
    if (this != &other) {
        Clear();
        Type_ = other.Type_;
        Value_ = other.Value_;
        other.Type_ = Null;
        other.Value_.Number = 0.0;
    }
    return *this;
}

void TJsonNode::Clear() {
    switch (Type_) {
        case String:
            delete Value_.String;
            break;
        case Array:
            delete Value_.Array;
            break;
        case Object:
            delete Value_.Object;
            break;
        default:
            break;
    }
    Type_ = Null;
    Value_.Number = 0.0;
}

void TJsonNode::CopyFrom(const TJsonNode& other) {
    Type_ = other.Type_;
    switch (Type_) {
        case Null:
            break;
        case Boolean:
            Value_.Boolean = other.Value_.Boolean;
            break;
        case Number:
            Value_.Number = other.Value_.Number;
            break;
        case String:
            Value_.String = new std::string(*other.Value_.String);
            break;
        case Array:
            Value_.Array = new std::vector<TJsonNode>(*other.Value_.Array);
            break;
        case Object:
            Value_.Object = new std::unordered_map<std::string, TJsonNode>(*other.Value_.Object);
            break;
    }
}

// Type queries
TJsonNode::EType TJsonNode::GetType() const {
    return Type_;
}

bool TJsonNode::IsNull() const {
    return Type_ == Null;
}

bool TJsonNode::IsBoolean() const {
    return Type_ == Boolean;
}

bool TJsonNode::IsNumber() const {
    return Type_ == Number;
}

bool TJsonNode::IsString() const {
    return Type_ == String;
}

bool TJsonNode::IsArray() const {
    return Type_ == Array;
}

bool TJsonNode::IsObject() const {
    return Type_ == Object;
}

// Container operations
size_t TJsonNode::size() const {
    if (Type_ == Array) {
        return Value_.Array->size();
    } else if (Type_ == Object) {
        return Value_.Object->size();
    }
    THROW("TJsonNode is not a container (Array or Object)");
}

bool TJsonNode::empty() const {
    if (Type_ == Array) {
        return Value_.Array->empty();
    } else if (Type_ == Object) {
        return Value_.Object->empty();
    } else if (Type_ == Null) {
        return true;
    }
    return false;
}

bool TJsonNode::contains(const std::string& key) const {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->find(key) != Value_.Object->end();
}

void TJsonNode::clear() {
    if (Type_ == Array) {
        Value_.Array->clear();
    } else if (Type_ == Object) {
        Value_.Object->clear();
    } else {
        Clear();
    }
}

// Access methods with bounds checking
TJsonNode& TJsonNode::at(const std::string& key) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto it = Value_.Object->find(key);
    if (it == Value_.Object->end()) {
        THROW("Key not found in object: {}", key);
    }
    return it->second;
}

const TJsonNode& TJsonNode::at(const std::string& key) const {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto it = Value_.Object->find(key);
    if (it == Value_.Object->end()) {
        THROW("Key not found in object: {}", key);
    }
    return it->second;
}

TJsonNode& TJsonNode::at(size_t idx) {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    if (idx >= Value_.Array->size()) {
        THROW("Array index out of bounds: {} >= {}", idx, Value_.Array->size());
    }
    return (*Value_.Array)[idx];
}

const TJsonNode& TJsonNode::at(size_t idx) const {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    if (idx >= Value_.Array->size()) {
        THROW("Array index out of bounds: {} >= {}", idx, Value_.Array->size());
    }
    return (*Value_.Array)[idx];
}

// Operator[] access
TJsonNode& TJsonNode::operator[](const std::string& key) {
    if (Type_ == Null) {
        // Auto-vivify to object
        Type_ = Object;
        Value_.Object = new TObject();
    }
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return (*Value_.Object)[key];
}

TJsonNode& TJsonNode::operator[](size_t idx) {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    if (idx >= Value_.Array->size()) {
        THROW("Array index out of bounds: {} >= {}", idx, Value_.Array->size());
    }
    return (*Value_.Array)[idx];
}

const TJsonNode& TJsonNode::operator[](size_t idx) const {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    if (idx >= Value_.Array->size()) {
        THROW("Array index out of bounds: {} >= {}", idx, Value_.Array->size());
    }
    return (*Value_.Array)[idx];
}

// Emplace for objects (non-template versions)
auto TJsonNode::emplace(const std::string& key, const TJsonNode& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->emplace(key, value);
}

auto TJsonNode::emplace(const std::string& key, TJsonNode&& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->emplace(key, std::move(value));
}

// Emplace for arrays (non-template versions)
void TJsonNode::emplace_back(const TJsonNode& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->emplace_back(value);
}

void TJsonNode::emplace_back(TJsonNode&& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->emplace_back(std::move(value));
}

// Insert for objects (non-template versions)
auto TJsonNode::insert(const std::string& key, const TJsonNode& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto [it, inserted] = Value_.Object->insert({key, value});
    return std::pair{it, inserted};
}

auto TJsonNode::insert(const std::string& key, TJsonNode&& value) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    auto [it, inserted] = Value_.Object->insert({key, std::move(value)});
    return std::pair{it, inserted};
}

// Push back for arrays (non-template versions)
void TJsonNode::push_back(const TJsonNode& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->push_back(value);
}

void TJsonNode::push_back(TJsonNode&& value) {
    if (Type_ == Null) {
        // Auto-vivify to array
        Type_ = Array;
        Value_.Array = new TArray();
    }
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    Value_.Array->push_back(std::move(value));
}

// Erase operations
bool TJsonNode::erase(const std::string& key) {
    if (Type_ != Object) {
        THROW("TJsonNode is not an object");
    }
    return Value_.Object->erase(key) > 0;
}

void TJsonNode::erase(size_t idx) {
    if (Type_ != Array) {
        THROW("TJsonNode is not an array");
    }
    if (idx >= Value_.Array->size()) {
        THROW("Array index out of bounds: {} >= {}", idx, Value_.Array->size());
    }
    Value_.Array->erase(Value_.Array->begin() + idx);
}

// Conversion operators
TJsonNode::operator bool() const {
    if (Type_ != Boolean) {
        THROW("TJsonNode is not a boolean");
    }
    return Value_.Boolean;
}

TJsonNode::operator int() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<int>(Value_.Number);
}

TJsonNode::operator long() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<long>(Value_.Number);
}

TJsonNode::operator long long() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<long long>(Value_.Number);
}

TJsonNode::operator unsigned int() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<unsigned int>(Value_.Number);
}

TJsonNode::operator unsigned long() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<unsigned long>(Value_.Number);
}

TJsonNode::operator unsigned long long() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<unsigned long long>(Value_.Number);
}

TJsonNode::operator float() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return static_cast<float>(Value_.Number);
}

TJsonNode::operator double() const {
    if (Type_ != Number) {
        THROW("TJsonNode is not a number");
    }
    return Value_.Number;
}

TJsonNode::operator std::string() const {
    if (Type_ != String) {
        THROW("TJsonNode is not a string");
    }
    return *Value_.String;
}

// Serialization methods
std::string TJsonNode::ToString(bool pretty, int indent) const {
    std::string result;
    ToStringImpl(result, pretty, indent);
    return result;
}

void TJsonNode::ToStringImpl(std::string& result, bool pretty, int currentIndent) const {
    const std::string indentStr = pretty ? std::string(currentIndent * 2, ' ') : "";
    const std::string nextIndentStr = pretty ? std::string((currentIndent + 1) * 2, ' ') : "";
    const std::string newline = pretty ? "\n" : "";
    const std::string space = pretty ? " " : "";

    switch (Type_) {
        case Null:
            result += "null";
            break;
            
        case Boolean:
            result += Value_.Boolean ? "true" : "false";
            break;
            
        case Number: {
            // Check if it's an integer
            if (Value_.Number == static_cast<long long>(Value_.Number)) {
                result += std::to_string(static_cast<long long>(Value_.Number));
            } else {
                result += std::to_string(Value_.Number);
            }
            break;
        }
            
        case String:
            result += '"';
            result += EscapeString(*Value_.String);
            result += '"';
            break;
            
        case Array: {
            result += '[';
            if (!Value_.Array->empty()) {
                result += newline;
                for (size_t i = 0; i < Value_.Array->size(); ++i) {
                    result += nextIndentStr;
                    (*Value_.Array)[i].ToStringImpl(result, pretty, currentIndent + 1);
                    if (i < Value_.Array->size() - 1) {
                        result += ',';
                    }
                    result += newline;
                }
                result += indentStr;
            }
            result += ']';
            break;
        }
            
        case Object: {
            result += '{';
            if (!Value_.Object->empty()) {
                result += newline;
                size_t count = 0;
                for (const auto& [key, value] : *Value_.Object) {
                    result += nextIndentStr;
                    result += '"';
                    result += EscapeString(key);
                    result += '"';
                    result += ':';
                    result += space;
                    value.ToStringImpl(result, pretty, currentIndent + 1);
                    if (count < Value_.Object->size() - 1) {
                        result += ',';
                    }
                    result += newline;
                    ++count;
                }
                result += indentStr;
            }
            result += '}';
            break;
        }
    }
}

std::string TJsonNode::EscapeString(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (char ch : str) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 32) {
                    // Control characters
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(ch));
                    result += buf;
                } else {
                    result += ch;
                }
                break;
        }
    }
    
    return result;
}

// Parsing methods
TJsonNode TJsonNode::Parse(const std::string& json) {
    return Parse(json.c_str());
}

TJsonNode TJsonNode::Parse(const char* json) {
    const char* ptr = json;
    SkipWhitespace(ptr);
    
    if (*ptr == '\0') {
        THROW("Empty JSON string");
    }
    
    TJsonNode result = ParseValue(ptr);
    
    SkipWhitespace(ptr);
    if (*ptr != '\0') {
        THROW("Unexpected characters after JSON value");
    }
    
    return result;
}

void TJsonNode::SkipWhitespace(const char*& ptr) {
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
        ++ptr;
    }
}

TJsonNode TJsonNode::ParseValue(const char*& ptr) {
    SkipWhitespace(ptr);
    
    switch (*ptr) {
        case 'n':
            return ParseLiteral(ptr, "null", TJsonNode());
        case 't':
            return ParseLiteral(ptr, "true", TJsonNode(true));
        case 'f':
            return ParseLiteral(ptr, "false", TJsonNode(false));
        case '"':
            return ParseString(ptr);
        case '[':
            return ParseArray(ptr);
        case '{':
            return ParseObject(ptr);
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return ParseNumber(ptr);
        default:
            THROW("Unexpected character in JSON: '{}'", *ptr);
    }
}

TJsonNode TJsonNode::ParseLiteral(const char*& ptr, const char* literal, TJsonNode value) {
    size_t len = strlen(literal);
    if (strncmp(ptr, literal, len) != 0) {
        THROW("Expected '{}' in JSON", literal);
    }
    ptr += len;
    return value;
}

TJsonNode TJsonNode::ParseString(const char*& ptr) {
    if (*ptr != '"') {
        THROW("Expected '\"' at start of string");
    }
    ++ptr;
    
    std::string result;
    while (*ptr != '"' && *ptr != '\0') {
        if (*ptr == '\\') {
            ++ptr;
            switch (*ptr) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    // Unicode escape sequence
                    ++ptr;
                    unsigned int codepoint = 0;
                    for (int i = 0; i < 4; ++i) {
                        char ch = *ptr;
                        if (ch >= '0' && ch <= '9') {
                            codepoint = codepoint * 16 + (ch - '0');
                        } else if (ch >= 'a' && ch <= 'f') {
                            codepoint = codepoint * 16 + (ch - 'a' + 10);
                        } else if (ch >= 'A' && ch <= 'F') {
                            codepoint = codepoint * 16 + (ch - 'A' + 10);
                        } else {
                            THROW("Invalid unicode escape sequence");
                        }
                        ++ptr;
                    }
                    // Simple handling: only ASCII range
                    if (codepoint < 128) {
                        result += static_cast<char>(codepoint);
                    } else {
                        result += '?'; // Placeholder for non-ASCII
                    }
                    continue;
                }
                default:
                    THROW("Invalid escape sequence: \\{}", *ptr);
            }
            ++ptr;
        } else {
            result += *ptr;
            ++ptr;
        }
    }
    
    if (*ptr != '"') {
        THROW("Unterminated string");
    }
    ++ptr;
    
    return TJsonNode(result);
}

TJsonNode TJsonNode::ParseNumber(const char*& ptr) {
    const char* start = ptr;
    
    // Optional minus sign
    if (*ptr == '-') {
        ++ptr;
    }
    
    // Integer part
    if (*ptr == '0') {
        ++ptr;
    } else if (*ptr >= '1' && *ptr <= '9') {
        while (*ptr >= '0' && *ptr <= '9') {
            ++ptr;
        }
    } else {
        THROW("Invalid number format");
    }
    
    // Fractional part
    bool isFloat = false;
    if (*ptr == '.') {
        isFloat = true;
        ++ptr;
        if (*ptr < '0' || *ptr > '9') {
            THROW("Expected digit after decimal point");
        }
        while (*ptr >= '0' && *ptr <= '9') {
            ++ptr;
        }
    }
    
    // Exponent
    if (*ptr == 'e' || *ptr == 'E') {
        isFloat = true;
        ++ptr;
        if (*ptr == '+' || *ptr == '-') {
            ++ptr;
        }
        if (*ptr < '0' || *ptr > '9') {
            THROW("Expected digit in exponent");
        }
        while (*ptr >= '0' && *ptr <= '9') {
            ++ptr;
        }
    }
    
    std::string numStr(start, ptr - start);
    double value = std::stod(numStr);
    
    return TJsonNode(value);
}

TJsonNode TJsonNode::ParseArray(const char*& ptr) {
    if (*ptr != '[') {
        THROW("Expected '[' at start of array");
    }
    ++ptr;
    
    TJsonNode result;
    result.Type_ = Array;
    result.Value_.Array = new TArray();
    
    SkipWhitespace(ptr);
    
    if (*ptr == ']') {
        ++ptr;
        return result;
    }
    
    while (true) {
        result.Value_.Array->push_back(ParseValue(ptr));
        
        SkipWhitespace(ptr);
        
        if (*ptr == ']') {
            ++ptr;
            break;
        }
        
        if (*ptr != ',') {
            THROW("Expected ',' or ']' in array");
        }
        ++ptr;
        
        SkipWhitespace(ptr);
    }
    
    return result;
}

TJsonNode TJsonNode::ParseObject(const char*& ptr) {
    if (*ptr != '{') {
        THROW("Expected '{' at start of object");
    }
    ++ptr;
    
    TJsonNode result;
    result.Type_ = Object;
    result.Value_.Object = new TObject();
    
    SkipWhitespace(ptr);
    
    if (*ptr == '}') {
        ++ptr;
        return result;
    }
    
    while (true) {
        SkipWhitespace(ptr);
        
        if (*ptr != '"') {
            THROW("Expected '\"' for object key");
        }
        
        TJsonNode keyNode = ParseString(ptr);
        std::string key = static_cast<std::string>(keyNode);
        
        SkipWhitespace(ptr);
        
        if (*ptr != ':') {
            THROW("Expected ':' after object key");
        }
        ++ptr;
        
        TJsonNode value = ParseValue(ptr);
        
        result.Value_.Object->emplace(std::move(key), std::move(value));
        
        SkipWhitespace(ptr);
        
        if (*ptr == '}') {
            ++ptr;
            break;
        }
        
        if (*ptr != ',') {
            THROW("Expected ',' or '}' in object");
        }
        ++ptr;
    }
    
    return result;
}

std::string TJsonNode::UnescapeString(const std::string& str) {
    // Not currently used, but kept for potential future use
    return str;
}

} // namespace NJson
