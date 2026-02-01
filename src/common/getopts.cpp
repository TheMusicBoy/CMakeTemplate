#include <common/getopts.h>

#include <cstring>
#include <iostream>
#include <sstream>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

std::string GetOpts::Version() {
    return "";
}

void GetOpts::Parse(int argc, const char* const argv[]) {
    if (Options_.empty() && Subcommands_.empty()) {
        Register();
    }

    if (argc > 0) {
        ProgramName_ = argv[0];
    }

    bool parseOptions = true;
    bool subcommandFound = false;

    for (int i = 1; i < argc; ++i) {
        size_t arg_size = std::strlen(argv[i]);
        const char* arg = argv[i];

        // Check for builtin commands (help, version) first
        if (!subcommandFound && parseOptions && TryProcessBuiltinCommand(arg)) {
            return;
        }

        // Check for subcommands
        if (!subcommandFound && parseOptions && arg[0] != '-' && !Subcommands_.empty()) {
            auto it = SubcommandIndex_.find(arg);
            if (it != SubcommandIndex_.end()) {
                ActiveSubcommand_ = Subcommands_[it->second].Instance;
                ActiveSubcommandName_ = Subcommands_[it->second].Name;
                ActiveSubcommand_->Parse(argc - i, argv + i);
                return;
            }
        }

        // Stop parsing options after "--"
        if (parseOptions && arg_size == 2 && arg[0] == '-' && arg[1] == '-') {
            parseOptions = false;
            continue;
        }

        // Process options
        if (parseOptions && arg_size > 1 && arg[0] == '-') {
            if (arg[1] == '-') {
                ProcessLongOption(argc, argv, i);
            } else {
                ProcessShortOption(argc, argv, i);
            }
        } else {
            ProcessPositional(arg);
        }
    }

    ApplyDefaults();
    ValidateRequired();
    ValidateArguments();
}

void GetOpts::ProcessShortOption(int argc, const char* const argv[], int& i) {
    size_t arg_size = std::strlen(argv[i]);
    const char* arg = argv[i];

    for (size_t pos = 1; pos < arg_size; ++pos) {
        char c = arg[pos];
        ASSERT(ShortIndex_.count(c), "Unknown option: -{}", c);

        size_t idx = ShortIndex_[c];
        TOptionInfo& optInfo = Options_[idx];
        optInfo.Present = true;

        if (optInfo.Registration->IsFlag()) {
            optInfo.Registration->SetFlag();
        } else {
            // Option requires an argument
            std::string value;
            if (pos + 1 < arg_size) {
                value = arg + pos + 1;
                pos = arg_size - 1;
            } else {
                ASSERT(i + 1 < argc, "Missing argument for: -{}", c);
                value = argv[++i];
            }
            
            // Use ParseAndAdd for variadic options to accumulate values
            if (optInfo.Registration->IsVariadic()) {
                optInfo.Registration->ParseAndAdd(value);
            } else {
                optInfo.Registration->ParseAndSet(value);
            }
        }
    }
}

void GetOpts::ProcessLongOption(int argc, const char* const argv[], int& i) {
    const char* arg = argv[i] + 2; // Skip "--"
    const char* eq_pos = strchr(arg, '=');
    std::string name;
    
    if (eq_pos) {
        name.assign(arg, eq_pos - arg);
    } else {
        name = arg;
    }

    ASSERT(LongIndex_.count(name), "Unknown option: --{}", name);
    
    size_t idx = LongIndex_[name];
    TOptionInfo& optInfo = Options_[idx];
    optInfo.Present = true;

    if (optInfo.Registration->IsFlag()) {
        ASSERT(eq_pos == nullptr, "Unexpected argument for flag: --{}", name);
        optInfo.Registration->SetFlag();
    } else {
        // Option requires an argument
        std::string value;
        if (eq_pos) {
            // Value provided with "="
            value = eq_pos + 1;
        } else {
            // Value should be in the next argument
            ASSERT(i + 1 < argc, "Missing argument for: --{}", name);
            value = argv[++i];
        }
        
        // Use ParseAndAdd for variadic options to accumulate values
        if (optInfo.Registration->IsVariadic()) {
            optInfo.Registration->ParseAndAdd(value);
        } else {
            optInfo.Registration->ParseAndSet(value);
        }
    }
}

void GetOpts::ProcessPositional(const std::string& arg) {
    Positional_.push_back(arg);
}

void GetOpts::ValidateOption(char shortName, const std::string& longName) const {
    ASSERT(shortName != 0 || !longName.empty(), "Option must have at least one name");
    ASSERT(shortName == 0 || !ShortIndex_.count(shortName), "Duplicate short option: -{}", shortName);
    ASSERT(longName.empty() || !LongIndex_.count(longName), "Duplicate long option: --{}", longName);
}

size_t GetOpts::RegisterOptionInternal(
    char shortName,
    const std::string& longName,
    std::shared_ptr<TOptionRegistrationBase> registration)
{
    ValidateOption(shortName, longName);
    
    size_t idx = Options_.size();
    
    TOptionInfo info;
    info.ShortName = shortName;
    info.LongName = longName;
    info.Registration = registration;
    info.Present = false;
    
    Options_.push_back(info);

    if (shortName != 0) {
        ShortIndex_[shortName] = idx;
    }
    if (!longName.empty()) {
        LongIndex_[longName] = idx;
    }
    
    return idx;
}

const std::vector<std::string>& GetOpts::GetPositional() const {
    return Positional_;
}

GetOpts* GetOpts::GetActiveSubcommand() const {
    return ActiveSubcommand_;
}

const std::string& GetOpts::GetActiveSubcommandName() const {
    return ActiveSubcommandName_;
}

void GetOpts::SetArgumentsCount(size_t count) {
    ValidateNoArgumentsAndSubcommands();
    ArgsMinCount_ = count;
    ArgsMaxCount_ = count;
    HasArgumentsConstraints_ = true;
}

void GetOpts::SetArgumentsMinCount(size_t minCount) {
    ValidateNoArgumentsAndSubcommands();
    ArgsMinCount_ = minCount;
    HasArgumentsConstraints_ = true;
}

void GetOpts::SetArgumentsMaxCount(size_t maxCount) {
    ValidateNoArgumentsAndSubcommands();
    ArgsMaxCount_ = maxCount;
    HasArgumentsConstraints_ = true;
}

void GetOpts::SetArgumentsRange(size_t minCount, size_t maxCount) {
    ValidateNoArgumentsAndSubcommands();
    ASSERT(minCount <= maxCount, "Min count ({}) must be <= max count ({})", minCount, maxCount);
    ArgsMinCount_ = minCount;
    ArgsMaxCount_ = maxCount;
    HasArgumentsConstraints_ = true;
}

void GetOpts::SetDescription(const std::string& description) {
    Description_ = description;
}

void GetOpts::AddExample(const std::string& example, const std::string& description) {
    TExample ex;
    ex.Command = example;
    ex.Description = description;
    Examples_.push_back(ex);
}

bool GetOpts::WasHelpShown() const {
    return HelpShown_;
}

bool GetOpts::WasVersionShown() const {
    return VersionShown_;
}

bool GetOpts::IsVersionOrHelp() const {
    // Check current level
    if (HelpShown_ || VersionShown_) {
        return true;
    }
    
    // Check active subcommand recursively
    if (ActiveSubcommand_ != nullptr) {
        return ActiveSubcommand_->IsVersionOrHelp();
    }
    
    return false;
}

void GetOpts::ApplyDefaults() {
    for (auto& optInfo : Options_) {
        if (!optInfo.Present && optInfo.Registration->HasDefault()) {
            optInfo.Registration->SetDefault();
        }
    }
}

void GetOpts::ValidateRequired() {
    for (const auto& optInfo : Options_) {
        if (optInfo.Registration->IsRequired() && !optInfo.Present) {
            std::string name;
            if (optInfo.ShortName != 0) {
                name = std::string("-") + optInfo.ShortName;
            }
            if (!optInfo.LongName.empty()) {
                if (!name.empty()) {
                    name += "/--";
                } else {
                    name = "--";
                }
                name += optInfo.LongName;
            }
            THROW("Required option not provided: {}", name);
        }
    }
}

void GetOpts::ValidateArguments() {
    if (!HasArgumentsConstraints_) {
        return;
    }

    size_t argsCount = Positional_.size();

    if (ArgsMinCount_.has_value() && argsCount < *ArgsMinCount_) {
        if (ArgsMaxCount_.has_value() && *ArgsMinCount_ == *ArgsMaxCount_) {
            THROW("Expected exactly {} argument(s), but got {}", *ArgsMinCount_, argsCount);
        } else {
            THROW("Expected at least {} argument(s), but got {}", *ArgsMinCount_, argsCount);
        }
    }

    if (ArgsMaxCount_.has_value() && argsCount > *ArgsMaxCount_) {
        if (ArgsMinCount_.has_value() && *ArgsMinCount_ == *ArgsMaxCount_) {
            THROW("Expected exactly {} argument(s), but got {}", *ArgsMaxCount_, argsCount);
        } else {
            THROW("Expected at most {} argument(s), but got {}", *ArgsMaxCount_, argsCount);
        }
    }
}

void GetOpts::ValidateNoArgumentsAndSubcommands() const {
    ASSERT(Subcommands_.empty() || !HasArgumentsConstraints_,
           "Cannot have both subcommands and positional arguments constraints");
}

std::string GetOpts::GenerateHelp() const {
    std::ostringstream oss;
    
    // Program name and usage
    oss << "Usage: " << ProgramName_;
    if (!Options_.empty()) {
        oss << " [OPTIONS]";
    }
    if (!Subcommands_.empty()) {
        oss << " <COMMAND>";
    }
    if (HasArgumentsConstraints_) {
        if (ArgsMinCount_.has_value() && ArgsMaxCount_.has_value()) {
            if (*ArgsMinCount_ == *ArgsMaxCount_) {
                for (size_t i = 0; i < *ArgsMinCount_; ++i) {
                    oss << " <arg" << (i + 1) << ">";
                }
            } else {
                for (size_t i = 0; i < *ArgsMinCount_; ++i) {
                    oss << " <arg" << (i + 1) << ">";
                }
                oss << " [arg" << (*ArgsMinCount_ + 1) << "...arg" << *ArgsMaxCount_ << "]";
            }
        } else if (ArgsMinCount_.has_value()) {
            for (size_t i = 0; i < *ArgsMinCount_; ++i) {
                oss << " <arg" << (i + 1) << ">";
            }
            oss << " [args...]";
        } else if (ArgsMaxCount_.has_value()) {
            oss << " [arg1...arg" << *ArgsMaxCount_ << "]";
        }
    }
    // Don't show [args...] if no constraints are set
    oss << "\n\n";
    
    // Description
    if (!Description_.empty()) {
        oss << Description_ << "\n\n";
    }
    
    // Options
    if (!Options_.empty()) {
        oss << "Options:\n";
        for (const auto& opt : Options_) {
            oss << "  ";
            if (opt.ShortName != 0) {
                oss << "-" << opt.ShortName;
                if (!opt.LongName.empty()) {
                    oss << ", ";
                }
            }
            if (!opt.LongName.empty()) {
                oss << "--" << opt.LongName;
            }
            if (!opt.Registration->IsFlag()) {
                oss << " <value>";
            }
            if (!opt.Registration->HelpText.empty()) {
                oss << "\n      " << opt.Registration->HelpText;
            }
            if (opt.Registration->IsVariadic()) {
                oss << "\n      (can be specified multiple times)";
            }
            oss << "\n";
        }
        oss << "\n";
    }
    
    // Subcommands
    if (!Subcommands_.empty()) {
        oss << "Commands:\n";
        for (const auto& sub : Subcommands_) {
            oss << "  " << sub.Name;
            if (!sub.HelpText.empty()) {
                oss << "\n      " << sub.HelpText;
            }
            oss << "\n";
        }
        oss << "\n";
    }
    
    // Examples
    if (!Examples_.empty()) {
        oss << "Examples:\n";
        for (const auto& example : Examples_) {
            oss << "  " << example.Command << "\n";
            if (!example.Description.empty()) {
                oss << "      " << example.Description << "\n";
            }
        }
        oss << "\n";
    }
    
    return oss.str();
}

void GetOpts::ShowHelp() const {
    std::cout << GenerateHelp();
    const_cast<GetOpts*>(this)->HelpShown_ = true;
}

void GetOpts::ShowVersion() const {
    std::string version = const_cast<GetOpts*>(this)->Version();
    if (!version.empty()) {
        std::cout << version << "\n";
    } else {
        std::cout << ProgramName_ << " (no version info)\n";
    }
    const_cast<GetOpts*>(this)->VersionShown_ = true;
}

bool GetOpts::TryProcessBuiltinCommand(const std::string& arg) {
    if (arg == "help" || arg == "--help" || arg == "-h") {
        ShowHelp();
        return true;
    }
    if (arg == "version" || arg == "--version" || arg == "-v") {
        ShowVersion();
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

#include "getopts_impl.h"
