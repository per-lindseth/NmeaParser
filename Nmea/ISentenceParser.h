#pragma once

#include <vector>
#include <string_view>

struct ISentenceParser
{
    virtual void parse(const std::vector<std::string_view> splitter) = 0;
};