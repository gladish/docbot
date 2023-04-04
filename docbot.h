
#pragma once

#include <regex>
#include <string>

namespace docbot {
struct Options {
    std::regex FunctionNameRegex;
    std::string ApiKey;
};
}