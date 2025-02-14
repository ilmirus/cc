#include "preprocessor.h"
#include <algorithm>
#include <ctype.h>
#include <assert.h>

static Locus locus(const std::string &file, const std::string &initial, const std::string &input) {
	auto prefix = initial.substr(0, initial.size() - input.size());
	auto lines = std::count(prefix.begin(), prefix.end(), '\n');
	int last_line_size = 0;
	auto last_new_line = prefix.find_last_of('\n');
	if (last_new_line != std::string::npos) {
		auto last_line = prefix.substr(last_new_line);
		last_line_size = last_line.size();
	}
	return Locus { file, lines + 1, last_line_size + 1 };
}

#define LOCUS locus(file, initial, input)

static char peek(const std::string &input, int i = 0) {
	if (input.size() <= i) return '\0';
	else return input[i];
}

static std::string skip(const std::string &input, int i = 1) {
	if (input.empty()) return "";
	return input.substr(i);
}

std::vector<PPToken> pp_scan(const std::string &file, std::string input) {
	std::string initial = input;
	std::vector<PPToken> result;
	while (true) {
		if (input.empty()) break;

		auto c = peek(input);
		// new-line
		if (c == '\n') {
			input = skip(input);
			result.push_back(PPToken { LOCUS, PPToken::kNewLine, "" });
			continue;
		}
		// identifier
		if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
			std::string value;
			while (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9') {
				value += c;
				input = skip(input);
				c = peek(input);
			}
			result.push_back(PPToken { LOCUS, PPToken::kIdentifier, value });
			continue;
		}
		assert(false);
	}
	if (!result.empty() && result.back().kind != PPToken::kNewLine) {
		result.push_back(PPToken { LOCUS, PPToken::kNewLine, "" });
	}
	return result;
}