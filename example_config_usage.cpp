// Example usage of the config system

#include <common/config.h>
#include <iostream>

namespace NExample {

////////////////////////////////////////////////////////////////////////////////
// Custom object with Load method
////////////////////////////////////////////////////////////////////////////////

struct TDatabaseConfig {
    std::string Host;
    int Port;
    std::string Username;
    std::string Password;

    void Load(const NJson::TJsonNode& data) {
        Host = data["host"].AsString();
        Port = data["port"].AsInt();
        Username = data["username"].AsString();
        Password = data["password"].AsString();
    }
};

////////////////////////////////////////////////////////////////////////////////
// Another custom object
////////////////////////////////////////////////////////////////////////////////

struct TServiceConfig {
    std::string Name;
    bool Enabled;
    int Timeout;

    void Load(const NJson::TJsonNode& data) {
        Name = data["name"].AsString();
        Enabled = data["enabled"].AsBool();
        Timeout = data["timeout"].AsInt();
    }
};

////////////////////////////////////////////////////////////////////////////////
// Main application config
////////////////////////////////////////////////////////////////////////////////

struct TAppConfig
    : public NCommon::TConfigBase
{
    // Simple types
    std::string ApplicationName;
    int MaxConnections;
    double Threshold;
    bool DebugMode;

    // Object with Load method
    TDatabaseConfig Database;

    // Map of objects with Load method
    std::map<std::string, TServiceConfig> Services;

    // Map of simple types
    std::map<std::string, std::string> Environment;

    void RegisterConfig() override {
        // Register with default values
        Register("application_name", &ApplicationName)
            .Default("MyApp");

        Register("max_connections", &MaxConnections)
            .Default(100);

        Register("threshold", &Threshold)
            .Default(0.75);

        Register("debug_mode", &DebugMode)
            .Default(false);

        // Register object (will call Database.Load())
        Register("database", &Database);

        // Register map of objects (will iterate and call Load() on each)
        Register("services", &Services);

        // Register map of simple types
        Register("environment", &Environment);
    }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NExample

////////////////////////////////////////////////////////////////////////////////
// Example JSON config file:
////////////////////////////////////////////////////////////////////////////////
/*
{
    "application_name": "ProductionApp",
    "max_connections": 500,
    "threshold": 0.9,
    "debug_mode": true,
    "database": {
        "host": "localhost",
        "port": 5432,
        "username": "admin",
        "password": "secret"
    },
    "services": {
        "auth": {
            "name": "AuthService",
            "enabled": true,
            "timeout": 30
        },
        "cache": {
            "name": "CacheService",
            "enabled": true,
            "timeout": 10
        }
    },
    "environment": {
        "LOG_LEVEL": "INFO",
        "DATA_DIR": "/var/data"
    }
}
*/

////////////////////////////////////////////////////////////////////////////////
// Usage example
////////////////////////////////////////////////////////////////////////////////

int main() {
    using namespace NExample;

    // Create config instance
    NCommon::TIntrusivePtr<TAppConfig> config = NCommon::MakeIntrusive<TAppConfig>();

    // Load from file
    config->LoadFromFile("config.json");

    // Access loaded values
    std::cout << "App Name: " << config->ApplicationName << std::endl;
    std::cout << "Max Connections: " << config->MaxConnections << std::endl;
    std::cout << "Threshold: " << config->Threshold << std::endl;
    std::cout << "Debug Mode: " << (config->DebugMode ? "true" : "false") << std::endl;

    std::cout << "\nDatabase Config:" << std::endl;
    std::cout << "  Host: " << config->Database.Host << std::endl;
    std::cout << "  Port: " << config->Database.Port << std::endl;

    std::cout << "\nServices:" << std::endl;
    for (const auto& [name, service] : config->Services) {
        std::cout << "  " << name << ": " << service.Name
                  << " (enabled: " << (service.Enabled ? "true" : "false") << ")" << std::endl;
    }

    std::cout << "\nEnvironment:" << std::endl;
    for (const auto& [key, value] : config->Environment) {
        std::cout << "  " << key << ": " << value << std::endl;
    }

    return 0;
}
