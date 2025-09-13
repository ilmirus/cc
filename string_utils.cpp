#include "string_utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <ranges>

std::string join_lines(const std::vector<std::string> &strings) {
    std::string result;
    bool first = true;
    for (const auto &str : strings) {
      	if (!first) {
      	   result += "\n";
      	}
      	result += str;
      	first = false;
    }
    return result;
}

std::string string_replace(std::string haystack, const std::string &needle, const std::string &replacement) {
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    	  haystack.replace(pos, needle.length(), replacement);
    	  pos += replacement.length();
    }
    return haystack;
}

std::string string_trim(const std::string &str) {
    auto is_space = [](unsigned char ch) { return std::isspace(ch); };
    auto start = std::ranges::find_if_not(str, is_space);
    auto end = std::ranges::find_if_not(str.rbegin(), str.rend(), is_space).base();
    return (start < end) ? std::string(start, end) : "";
}

std::vector<std::string> string_split_and_trim(const std::string &str, const std::string &delimeter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimeter);

    while (end != std::string::npos) {
        auto trimmed = string_trim(str.substr(start, end - start));
        if (!trimmed.empty()) {
            result.push_back(trimmed);
        }

        start = end + delimeter.length();
        end = str.find(delimeter, start);
    }

    auto trimmed = string_trim(str.substr(start));
    if (!trimmed.empty()) {
        result.push_back(trimmed);
    }
    return result;
}
