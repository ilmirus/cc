#include "file_utils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string slurp(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file for reading: " + path);
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  if (!file.good() && !file.eof()) {
    throw std::runtime_error("Failed while reading file: " + path);
  }
  return buffer.str();
}

std::vector<std::string> slurp_lines(const std::string &path) {
  std::vector<std::string> lines;
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file for reading: " + path);
  }

  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.erase(line.size() - 1);
    }

    lines.push_back(line);
  }
  if (!file.eof()) {
    throw std::runtime_error("Failed while reading file: " + path);
  }

  return lines;
}

void file_dump(const std::string &path, const std::string &content) {
  std::ofstream os(path);
  if (!os.is_open()) {
    throw std::runtime_error("Cannot open file for writing: " + path);
  }
  os << content;
  if (!os.good()) {
    throw std::runtime_error("Failed while writing file: " + path);
  }
}
