#pragma once

#include <common/exception.h>

#include <list>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class GetOpts {
public:
    GetOpts() = default;
    virtual ~GetOpts() = default;

    void Parse(int argc, const char* const argv[]);

    virtual void Register() = 0;

    virtual std::string Version();

    const std::vector<std::string>& GetPositional() const;

    GetOpts* GetActiveSubcommand() const;
    const std::string& GetActiveSubcommandName() const;

    // Configure positional arguments constraints
    void SetArgumentsCount(size_t count);
    void SetArgumentsMinCount(size_t minCount);
    void SetArgumentsMaxCount(size_t maxCount);
    void SetArgumentsRange(size_t minCount, size_t maxCount);

    // Set program description and examples
    void SetDescription(const std::string& description);
    void AddExample(const std::string& example, const std::string& description = "");

    // Check if help or version was displayed
    bool WasHelpShown() const;
    bool WasVersionShown() const;
    bool IsVersionOrHelp() const;  // Returns true if help or version was shown anywhere (including subcommands)

protected:
    class TOptionRegistrationBase {
    public:
        virtual ~TOptionRegistrationBase() = default;

        virtual void ParseAndSet(const std::string& value) = 0;
        virtual void ParseAndAdd(const std::string& value) = 0;
        virtual void SetFlag() = 0;
        virtual bool IsFlag() const = 0;
        virtual bool IsVariadic() const = 0;
        virtual void SetDefault() = 0;
        virtual bool HasDefault() const = 0;
        virtual bool IsRequired() const = 0;

        std::string HelpText;
    };

    template <typename Type>
    class TOptionRegistration
        : public TOptionRegistrationBase
    {
    public:
        TOptionRegistration(Type* variable);

        TOptionRegistration& Help(const std::string& help);
        TOptionRegistration& Default(Type fallback);
        TOptionRegistration& Required();

        void ParseAndSet(const std::string& value) override;
        void ParseAndAdd(const std::string& value) override;
        void SetFlag() override;
        bool IsFlag() const override;
        bool IsVariadic() const override;
        void SetDefault() override;
        bool HasDefault() const override;
        bool IsRequired() const override;

    private:
        Type* VariableAddress_;
        std::optional<Type> Default_;
        bool Required_ = false;
    };

    struct TOptionInfo {
        char ShortName = 0;
        std::string LongName;
        std::shared_ptr<TOptionRegistrationBase> Registration;
        bool Present = false;
    };

    struct TSubcommandInfo {
        std::string Name;
        GetOpts* Instance;
        std::string HelpText;
    };

    template <typename Type>
    TOptionRegistration<Type>& AddOption(
        char shortName,
        const std::string& longName,
        Type* variable);

    template <typename Type>
    TOptionRegistration<Type>& AddSubcommand(
        const std::string& name,
        Type* subcommand);

private:
    std::vector<TOptionInfo> Options_;
    std::unordered_map<char, size_t> ShortIndex_;
    std::unordered_map<std::string, size_t> LongIndex_;
    std::vector<std::string> Positional_;
    std::vector<TSubcommandInfo> Subcommands_;
    std::unordered_map<std::string, size_t> SubcommandIndex_;
    GetOpts* ActiveSubcommand_ = nullptr;
    std::string ActiveSubcommandName_;
    std::string ProgramName_;

    // Arguments constraints
    std::optional<size_t> ArgsMinCount_;
    std::optional<size_t> ArgsMaxCount_;
    bool HasArgumentsConstraints_ = false;

    // Program description and examples
    std::string Description_;
    struct TExample {
        std::string Command;
        std::string Description;
    };
    std::vector<TExample> Examples_;

    // Track if help or version was shown
    bool HelpShown_ = false;
    bool VersionShown_ = false;

    void ValidateOption(char shortName, const std::string& longName) const;
    size_t RegisterOptionInternal(
        char shortName,
        const std::string& longName,
        std::shared_ptr<TOptionRegistrationBase> registration);
    void ProcessShortOption(int argc, const char* const argv[], int& i);
    void ProcessLongOption(int argc, const char* const argv[], int& i);
    void ProcessPositional(const std::string& arg);
    void ApplyDefaults();
    void ValidateRequired();
    void ValidateArguments();
    void ValidateNoArgumentsAndSubcommands() const;
    std::string GenerateHelp() const;
    void ShowHelp() const;
    void ShowVersion() const;
    bool TryProcessBuiltinCommand(const std::string& arg);
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

// Template implementations
#include "getopts_impl.h"
