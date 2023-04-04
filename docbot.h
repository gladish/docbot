
#pragma once

#include <regex>
#include <string>
#include <vector>

namespace docbot {
struct Options {
    std::regex FunctionNameRegex;
    std::string ApiKey;
    std::vector<std::string> CompilerArgs;
    std::string InputFilename;
    std::string Personality;
};
}