#pragma once

#include <common/refcounted.h>
#include <common/intrusive_ptr.h>
#include <common/exception.h>
#include <common/json.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TConfigBase
{
public:
    void LoadFromFile(const std::filesystem::path& filePath);

    void Load(const NJson::TJsonNode& data);

    virtual void RegisterConfig() = 0;

    virtual void Postprocess();

protected:
    class TRegisteredObjectBase {
    public:
        virtual void LoadFromJson(const NJson::TJsonNode& data) = 0;

        virtual void LoadFromDefault() = 0;
    };

    template <typename Type>
    class TRegisteredObject
        : public TRegisteredObjectBase
    {
    public:
        TRegisteredObject(Type* variable);

        TRegisteredObject& Default(Type fallback);

        void LoadFromJson(const NJson::TJsonNode& data) override;

        void LoadFromDefault() override;

    private:
        Type* VariableAddress_;
        std::optional<Type> Default_;
    };

    template <typename Type>
    TRegisteredObject<Type>& Register(const std::string& name, Type* variable);
    
private:
    std::unordered_map<std::string, std::shared_ptr<TRegisteredObjectBase>> RegisteredObjects_;

};

////////////////////////////////////////////////////////////////////////////////

namespace NDetails {

} // namespace NDetails

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

// Template implementations
#include "config_impl.h"
