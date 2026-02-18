#pragma once

#include <string_view>

struct IField
{
    virtual ~IField() {}

    virtual size_t parse(const std::vector<std::string_view> splitter, size_t index) = 0;
};