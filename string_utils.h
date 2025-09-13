#pragma once

#include <string>
#include <vector>

std::string join_lines(const std::vector<std::string>& strings);

std::string string_replace(std::string haystack, const std::string &needle, const std::string &replacement);

std::string string_trim(const std::string &str);

std::vector<std::string> string_split_and_trim(const std::string &str, const std::string &delimeter);
