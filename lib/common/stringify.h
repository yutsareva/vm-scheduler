#pragma once

#include <string>


template <typename ...P> std::string toString(const P &... params)
{
    std::stringstream stream;

    (stream << ... << params);

    return stream.str();
}