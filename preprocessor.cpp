#include "preprocessor.h"
#include <algorithm>
#include <ctype.h>
#include <assert.h>
#include <stdexcept>
#include <regex>

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
    const static std::regex patterns(
        R"((\n)|)" // new-line
        R"((L?'.*(?:'|$))|)" // character-constant
        R"(([_a-zA-Z][_a-zA-Z0-9]*))" // identifier
    );

    std::string initial = input;
    std::smatch match;
    std::vector<PPToken> result;

    auto locus = Locus { file, 1, 1 };
    while (std::regex_search(input, match, patterns)) {
        PPToken token { locus, PPToken::kReserved, match.str(0) };
        if (match[PPToken::kNewLine].matched) {
            token.kind = PPToken::kNewLine;
        } else if (match[PPToken::kCharacterConstant].matched) {
            if (!match.str(0).ends_with("'")) throw std::runtime_error("Unclosed character constant");
            token.kind = PPToken::kCharacterConstant;
        } else if (match[PPToken::kIdentifier].matched) {
            token.kind = PPToken::kIdentifier;
        }

        result.push_back(token);

        input = input.substr(match.position() + match.length());
        locus = calculate_locus(file, initial, input);
    }
    if (!result.empty() && result.back().kind != PPToken::kNewLine) {
        result.push_back(PPToken { calculate_locus(file, initial, input), PPToken::kNewLine, "" });
    }
    return result;
}
