#include "string_utils.h"

std::string join_lines(const std::vector<std::string>& strings) {
	std::string result;
	bool first = true;
	for (const auto& str : strings) {
		if (!first) {
			result += "\n";
		}
		result += str;
		first = false;
	}
	return result;
}
