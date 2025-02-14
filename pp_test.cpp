#include "pp_test.h"

#include <iostream>
#include <assert.h>

static std::string stringify(const PPToken &pp_token) {
	using namespace std::string_literals;

	switch (pp_token.kind) {
	case PPToken::kWhitespace:
		return "whitespace\n";
	case PPToken::kNewLine: 
		return "new-line\n";
	case PPToken::kHeaderName: 
		return "header-name "s + pp_token.value + "\n";
	case PPToken::kIdentifier: 
		return "identifier "s + pp_token.value + "\n";
	case PPToken::kPPNumber: 
		return "pp-number "s + pp_token.value + "\n";
	case PPToken::kCharacterConstant: 
		return "character-constant "s + pp_token.value + "\n";
	case PPToken::kStringLiteral: 
		return "string-literal "s + pp_token.value + "\n";
	case PPToken::kOperator: 
		return "operator "s + pp_token.value + "\n";
	case PPToken::kPunctuator: 
		return "punctuator "s + pp_token.value + "\n";
	case PPToken::kNonWhiteSpace: 
		return "non-whitespace "s + pp_token.value + "\n";
	default:
		assert(0 && "unreachable");
	}
}

static std::string stringify(const std::vector<PPToken> &pp_tokens) {
	std::string result;
	for (auto &pp_token: pp_tokens) {
		result += stringify(pp_token);
	}
	return result;
}

void pp_test() {
	assert (stringify(pp_scan("fake", "\n")) == "new-line\n" && "FAIL 1");
	assert (stringify(pp_scan("fake", "a")) == "identifier a\nnew-line\n" && "FAIL 2");
}
