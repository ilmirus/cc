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
	    kReserved,
		kNewLine,
		kCharacterConstant,
		kIdentifier,
		kWhitespace,
		kPPNumber,
		kOperator,
		kNonWhiteSpace,
		kHeaderName,
		kStringLiteral,
		kPunctuator
	} kind;

	std::string value;
};

std::vector<PPToken> pp_scan(const std::string &file, std::string input);