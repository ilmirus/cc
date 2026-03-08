#pragma once

#include <string>
#include <vector>

std::string slurp(const std::string &path);

std::vector<std::string> slurp_lines(const std::string &path);

void file_dump(const std::string &path, const std::string &content);
