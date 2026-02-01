#pragma once

#include "getopts.h"

#include <sstream>
#include <type_traits>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

namespace NDetails {

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

template <typename T>
concept Boolean = std::is_same_v<T, bool>;

template <typename T>
concept String = std::is_same_v<T, std::string>;

template <typename T>
concept GetOptsBase = std::is_base_of_v<GetOpts, std::remove_pointer_t<T>>;

template <typename T>
struct IsContainer : std::false_type {};

template <typename T>
struct IsContainer<std::vector<T>> : std::true_type {
    using ValueType = T;
};

template <typename T>
struct IsContainer<std::list<T>> : std::true_type {
    using ValueType = T;
};

template <typename T>
struct IsContainer<std::set<T>> : std::true_type {
    using ValueType = T;
};

template <typename T>
struct IsContainer<std::unordered_set<T>> : std::true_type {
    using ValueType = T;
};

template <typename T>
concept Container = IsContainer<T>::value;

////////////////////////////////////////////////////////////////////////////////

template <typename Type>
void ParseValue(Type* variable, const std::string& value);

template <Boolean Type>
void ParseValue(Type* variable, const std::string& value) {
    if (value.empty() || value == "true" || value == "1" || value == "yes") {
        *variable = true;
    } else if (value == "false" || value == "0" || value == "no") {
        *variable = false;
    } else {
        THROW("Invalid boolean value: {}", value);
    }
}

template <Arithmetic Type>
void ParseValue(Type* variable, const std::string& value) {
    std::istringstream iss(value);
    Type result;
    iss >> result;
    ASSERT(!iss.fail() && iss.eof(), "Invalid numeric value: {}", value);
    *variable = result;
}

template <String Type>
void ParseValue(Type* variable, const std::string& value) {
    *variable = value;
}

template <GetOptsBase Type>
void ParseValue(Type* variable, const std::string& value) {
    THROW("Cannot parse subcommand from string value. Subcommands should be invoked directly, not parsed from strings.");
}

template <Container Type>
void ParseValue(Type* variable, const std::string& value) {
    using ValueType = typename IsContainer<Type>::ValueType;
    ValueType item;
    ParseValue(&item, value);
    
    // For set/unordered_set, insert doesn't take iterator hint
    if constexpr (std::is_same_v<Type, std::set<ValueType>> || 
                  std::is_same_v<Type, std::unordered_set<ValueType>>) {
        variable->insert(item);
    } else {
        // For vector/list, use hint iterator
        variable->insert(variable->end(), item);
    }
}

} // namespace NDetails

////////////////////////////////////////////////////////////////////////////////

template <typename Type>
GetOpts::TOptionRegistration<Type>::TOptionRegistration(Type* variable)
    : VariableAddress_(variable)
    , Default_(std::nullopt)
    , Required_(false)
{
}

template <typename Type>
GetOpts::TOptionRegistration<Type>& GetOpts::TOptionRegistration<Type>::Help(const std::string& help)
{
    this->HelpText = help;
    return *this;
}

template <typename Type>
GetOpts::TOptionRegistration<Type>& GetOpts::TOptionRegistration<Type>::Default(Type fallback)
{
    this->Default_ = fallback;
    return *this;
}

template <typename Type>
GetOpts::TOptionRegistration<Type>& GetOpts::TOptionRegistration<Type>::Required()
{
    this->Required_ = true;
    return *this;
}

template <typename Type>
void GetOpts::TOptionRegistration<Type>::ParseAndSet(const std::string& value)
{
    if constexpr (NDetails::Container<Type>) {
        VariableAddress_->clear();
    }
    NDetails::ParseValue<Type>(VariableAddress_, value);
}

template <typename Type>
void GetOpts::TOptionRegistration<Type>::ParseAndAdd(const std::string& value)
{
    if constexpr (NDetails::Container<Type>) {
        using ValueType = typename NDetails::IsContainer<Type>::ValueType;
        ValueType item;
        NDetails::ParseValue(&item, value);
        
        // For set/unordered_set, insert doesn't take iterator hint
        if constexpr (std::is_same_v<Type, std::set<ValueType>> || 
                      std::is_same_v<Type, std::unordered_set<ValueType>>) {
            VariableAddress_->insert(item);
        } else {
            // For vector/list, use hint iterator
            VariableAddress_->insert(VariableAddress_->end(), item);
        }
    } else {
        // For non-containers, ParseAndAdd behaves like ParseAndSet
        NDetails::ParseValue<Type>(VariableAddress_, value);
    }
}

template <typename Type>
void GetOpts::TOptionRegistration<Type>::SetFlag()
{
    if constexpr (NDetails::Boolean<Type>) {
        *VariableAddress_ = true;
    } else {
        THROW("Cannot use flag syntax for non-boolean option");
    }
}

template <typename Type>
bool GetOpts::TOptionRegistration<Type>::IsFlag() const
{
    return NDetails::Boolean<Type>;
}

template <typename Type>
bool GetOpts::TOptionRegistration<Type>::IsVariadic() const
{
    return NDetails::Container<Type>;
}

template <typename Type>
void GetOpts::TOptionRegistration<Type>::SetDefault()
{
    if constexpr (NDetails::GetOptsBase<Type>) {
        // For nested GetOpts objects, just parse empty args
        const char* empty_argv[] = {""};
        VariableAddress_->Parse(0, empty_argv);
    } else {
        if (Default_.has_value()) {
            *VariableAddress_ = *Default_;
        }
    }
}

template <typename Type>
bool GetOpts::TOptionRegistration<Type>::HasDefault() const
{
    if constexpr (NDetails::GetOptsBase<Type>) {
        return true; // Subcommands always have "default" (empty parse)
    } else {
        return Default_.has_value();
    }
}

template <typename Type>
bool GetOpts::TOptionRegistration<Type>::IsRequired() const
{
    return Required_;
}

////////////////////////////////////////////////////////////////////////////////

template <typename Type>
GetOpts::TOptionRegistration<Type>& GetOpts::AddOption(
    char shortName,
    const std::string& longName,
    Type* variable)
{
    auto reg = std::make_shared<TOptionRegistration<Type>>(variable);
    RegisterOptionInternal(shortName, longName, reg);
    return *reg;
}

template <typename Type>
GetOpts::TOptionRegistration<Type>& GetOpts::AddSubcommand(
    const std::string& name,
    Type* subcommand)
{
    static_assert(NDetails::GetOptsBase<Type>, "Subcommand must inherit from GetOpts");
    
    ASSERT(!name.empty(), "Subcommand name cannot be empty");
    ASSERT(!SubcommandIndex_.count(name), "Duplicate subcommand: {}", name);
    
    size_t idx = Subcommands_.size();
    SubcommandIndex_[name] = idx;
    
    TSubcommandInfo info;
    info.Name = name;
    info.Instance = subcommand;
    Subcommands_.push_back(info);
    
    // Create a special registration that stores help text in TSubcommandInfo
    class TSubcommandRegistration : public TOptionRegistration<Type> {
    public:
        TSubcommandRegistration(Type* variable, TSubcommandInfo* subInfo)
            : TOptionRegistration<Type>(variable)
            , SubInfo_(subInfo)
        {}
        
        TSubcommandRegistration& Help(const std::string& help) {
            SubInfo_->HelpText = help;
            this->HelpText = help;
            return *this;
        }
        
    private:
        TSubcommandInfo* SubInfo_;
    };
    
    auto reg = std::make_shared<TSubcommandRegistration>(subcommand, &Subcommands_[idx]);
    return *reg;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
