#pragma once

#include "config.h"

#include <map>
#include <memory>

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
concept SequenceContainer = requires(T& t, typename T::value_type v) {
    typename T::value_type;
    { t.clear() } -> std::same_as<void>;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.push_back(v) };
} && !std::is_same_v<T, std::string>;

template <typename T>
concept SetContainer = requires(T& t, typename T::value_type v) {
    typename T::value_type;
    { t.clear() } -> std::same_as<void>;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.insert(v) };
} && !SequenceContainer<T> && !requires { typename T::mapped_type; };

template <typename T>
concept MapContainer = requires(T& t) {
    typename T::key_type;
    typename T::mapped_type;
    requires std::is_same_v<typename T::key_type, std::string>;
    { t.clear() } -> std::same_as<void>;
    { t.emplace(std::declval<std::string>(), std::declval<typename T::mapped_type>()) };
} && !SequenceContainer<T> && !SetContainer<T>;

template <typename T>
concept ConfigBase = std::is_base_of_v<TConfigBase, std::remove_pointer_t<T>>;

template <typename T>
concept Reservable = requires(T& t, std::size_t n) {
    { t.reserve(n) } -> std::same_as<void>;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Type>
void Load(Type* variable, const NJson::TJsonNode& data);

template <typename Type>
    requires ConfigBase<Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    variable->Load(data);
}

template <Boolean Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    *variable = static_cast<bool>(data);
}

template <Arithmetic Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    if constexpr (std::is_integral_v<Type>) {
        if constexpr (std::is_signed_v<Type>) {
            if constexpr (sizeof(Type) <= sizeof(int)) {
                *variable = static_cast<Type>(static_cast<int>(data));
            } else {
                *variable = static_cast<Type>(static_cast<long long>(data));
            }
        } else {
            if constexpr (sizeof(Type) <= sizeof(unsigned int)) {
                *variable = static_cast<Type>(static_cast<unsigned int>(data));
            } else {
                *variable = static_cast<Type>(static_cast<unsigned long long>(data));
            }
        }
    } else {
        *variable = static_cast<Type>(static_cast<double>(data));
    }
}

template <String Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    *variable = static_cast<std::string>(data);
}

template <SequenceContainer Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    if (!data.IsArray()) {
        THROW("Expected array for sequence container");
    }
    
    variable->clear();
    
    if constexpr (Reservable<Type>) {
        variable->reserve(data.size());
    }
    
    for (size_t i = 0; i < data.size(); ++i) {
        typename Type::value_type tmp;
        Load(&tmp, data[i]);
        variable->push_back(std::move(tmp));
    }
}

template <SetContainer Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    if (!data.IsArray()) {
        THROW("Expected array for set container");
    }
    
    variable->clear();
    
    if constexpr (Reservable<Type>) {
        variable->reserve(data.size());
    }
    
    for (size_t i = 0; i < data.size(); ++i) {
        typename Type::value_type tmp;
        Load(&tmp, data[i]);
        variable->insert(std::move(tmp));
    }
}

template <MapContainer Type>
void Load(Type* variable, const NJson::TJsonNode& data) {
    if (!data.IsObject()) {
        THROW("Expected object for map container");
    }
    
    auto mappedData = static_cast<std::unordered_map<std::string, NJson::TJsonNode>>(data);
    variable->clear();
    
    if constexpr (Reservable<Type>) {
        variable->reserve(mappedData.size());
    }
    
    for (const auto& [key, value] : mappedData) {
        typename Type::mapped_type tmp;
        Load(&tmp, value);
        variable->emplace(key, std::move(tmp));
    }
}

} // namespace NDetails

////////////////////////////////////////////////////////////////////////////////

template <typename Type>
TConfigBase::TRegisteredObject<Type>::TRegisteredObject(Type* variable)
    : VariableAddress_(variable)
    , Default_(std::nullopt)
{
}

template <typename Type>
TConfigBase::TRegisteredObject<Type>& TConfigBase::TRegisteredObject<Type>::Default(Type fallback)
{
    this->Default_ = fallback;
    return *this;
}

template <typename Type>
void TConfigBase::TRegisteredObject<Type>::LoadFromJson(const NJson::TJsonNode& data)
{
    NDetails::Load<Type>(VariableAddress_, data);
}

template <typename Type>
void TConfigBase::TRegisteredObject<Type>::LoadFromDefault()
{
    if constexpr (NDetails::ConfigBase<Type>) {
        // For nested config objects, load with empty JSON to initialize with their defaults
        NJson::TJsonNode emptyJson = NJson::TJsonNode::TObject();
        VariableAddress_->Load(emptyJson);
    } else {
        ASSERT(Default_.has_value(), "data is required!");
        *VariableAddress_ = *Default_;
    }
}

template <typename Type>
TConfigBase::TRegisteredObject<Type>& TConfigBase::Register(const std::string& name, Type* variable)
{
    auto obj = std::make_shared<TRegisteredObject<Type>>(variable);
    this->RegisteredObjects_[name] = obj;
    return *obj;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
