#include "file_utils.h"

#include <iostream>
#include <fstream>
#include <sstream>

std::string slurp(const std::string& path) {
	std::ifstream file(path);
	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::vector<std::string> slurp_lines(const std::string& path) {
	std::vector<std::string> lines;
	std::ifstream file(path);

	std::string line;
	while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.erase(line.size() - 1);
        }

		lines.push_back(line);
	}

	return lines;
}
