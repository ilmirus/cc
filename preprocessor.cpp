#include "preprocessor.h"
#include <algorithm>
#include <ctype.h>
#include <assert.h>
#include <stdexcept>

static Locus calculate_locus(
    const std::string &file,
    const std::string &initial,
    const std::string &input
) {
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
		std::string value;
		auto locus = calculate_locus(file, initial, input);

		#define NEXT do { \
            value += c; \
            input = skip(input); \
            c = peek(input); \
		} while(0)

		// new-line
		if (c == '\n') {
			input = skip(input);
			result.push_back(PPToken { locus, PPToken::kNewLine, "" });
			continue;
		}
		// character
		if (c == '\'' || c == 'L' && peek(input, 1) == '\'') {
		    if (c == 'L') NEXT;
		    do NEXT; while (c != '\0' && c != '\'');
		    if (c == '\0') throw std::logic_error("Unclosed character constant");
		    NEXT;
		    result.push_back(PPToken { locus, PPToken::kCharacterConstant, value });
		    continue;
		}
		// identifier
		if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
			while (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9')
			    NEXT;
			result.push_back(PPToken { locus, PPToken::kIdentifier, value });
			continue;
		}
		assert(false && "unimplemented");

		#undef NEXT
	}
	if (!result.empty() && result.back().kind != PPToken::kNewLine) {
		result.push_back(PPToken { calculate_locus(file, initial, input), PPToken::kNewLine, "" });
	}
	return result;
}