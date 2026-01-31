#include <common/config.h>

#include <fstream>
#include <sstream>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

void TConfigBase::LoadFromFile(const std::filesystem::path& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            THROW("Failed to open config file: {}", filePath.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonStr = buffer.str();

        NJson::TJsonNode configJson = NJson::TJsonNode::Parse(jsonStr);

        this->Load(configJson);
    } catch (const std::exception& e) {
        RETHROW(e, "Config loading failed from file: {}", filePath.string());
    }
}

void TConfigBase::Load(const NJson::TJsonNode& data) {
    if (RegisteredObjects_.empty()) {
        RegisterConfig();
    }

    std::unordered_map<std::string, NJson::TJsonNode> mappedData(data);
    
    // Load fields that are present in JSON
    for (const auto& [fieldName, fieldData] : mappedData) {
        try {
            auto it = RegisteredObjects_.find(fieldName);
            ASSERT(it != RegisteredObjects_.end(), "unknown field");
            it->second->LoadFromJson(fieldData);
        } catch (std::exception& ex) {
            THROW("Failed to process field \"{}\": {}", fieldName, ex.what());
        }
    }

    // Load default values for fields not present in JSON
    for (auto& [fieldName, registeredObject] : RegisteredObjects_) {
        try {
            if (mappedData.find(fieldName) != mappedData.end()) {
                continue;
            }

            registeredObject->LoadFromDefault();
        } catch (std::exception& ex) {
            THROW("Failed to process field \"{}\": {}", fieldName, ex.what());
        }
    }

    Postprocess();
}

void TConfigBase::Postprocess()
{}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
