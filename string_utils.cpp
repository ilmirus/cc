#include "string_utils.h"

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
