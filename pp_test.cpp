#include "pp_test.h"

#include <iostream>
#include <assert.h>

#include "file_utils.h"
#include "string_utils.h"

static std::string stringify(const PPToken &pp_token) {
	using namespace std::string_literals;

	switch (pp_token.kind) {
	case PPToken::kWhitespace:
		return "whitespace";
	case PPToken::kNewLine: 
		return "new-line";
	case PPToken::kHeaderName: 
		return "header-name "s + pp_token.value;
	case PPToken::kIdentifier: 
		return "identifier "s + pp_token.value;
	case PPToken::kPPNumber: 
		return "pp-number "s + pp_token.value;
	case PPToken::kCharacterConstant: 
		return "character-constant "s + pp_token.value;
	case PPToken::kStringLiteral: 
		return "string-literal "s + pp_token.value;
	case PPToken::kOperator: 
		return "operator "s + pp_token.value;
	case PPToken::kPunctuator: 
		return "punctuator "s + pp_token.value;
	case PPToken::kNonWhiteSpace: 
		return "non-whitespace "s + pp_token.value;
	default:
		assert(0 && "unreachable");
	}
}

static std::string stringify(const std::vector<PPToken> &pp_tokens) {
	std::string result;
	for (auto &pp_token: pp_tokens) {
		result += stringify(pp_token) + "\n";
	}
	return result;
}

void pp_test() {
	std::vector<std::string> tests = {
		"100-a",
		"100-character-literals",
		"100-comments",
		"100-empty",
		"100-floating",
		"100-integer-zero"
	};

	for (auto &test: tests) {
		auto input = slurp("tests/" + test + ".c");
		auto expected = slurp_lines("tests/" + test + ".ref");
		auto got = pp_scan(test, input);

		for (size_t i = 0; i < std::min(got.size(), expected.size()); i++) {
			if (expected[i] != stringify(got[i])) {
				throw std::logic_error(
					test + ": mismatch on line " + std::to_string(i + 1) +
						" expected:\n" + expected[i] + "\ngot\n" + stringify(got[i])
				);
			}
		}

		if (got.size() != expected.size()) {
			throw std::logic_error(
				test + ": size mismatch expected:\n" + join_lines(expected) + "\ngot\n" + stringify(got)
			);
		}
	}
}
