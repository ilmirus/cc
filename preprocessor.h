#pragma once

#include <string>
#include <vector>

struct Locus {
	std::string file;
	int row, column;
};

struct PPToken {
	Locus locus;

	enum {
		kWhitespace,
		kNewLine,
		kHeaderName,
		kIdentifier,
		kPPNumber,
		kCharacterConstant,
		kStringLiteral,
		kOperator,
		kPunctuator,
		kNonWhiteSpace
	} kind;

	std::string value;
};

std::vector<PPToken> pp_scan(const std::string &file, std::string input);