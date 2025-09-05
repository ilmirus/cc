#include "preprocessor.h"
#include <algorithm>
#include <ctype.h>
#include <assert.h>
#include <stdexcept>
#include <regex>
#include <iostream>

#include "pp_input.h"

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
        auto last_line = prefix.substr(last_new_line + 1);
        last_line_size = last_line.size();
    } else {
        last_line_size = prefix.size();
    }
    return Locus { file, lines + 1, last_line_size + 1 };
}

std::vector<PPToken> pp_scan(const std::string &file, std::string input) {
    const static std::regex patterns(
#include "pptoken.regex.generated.cpp"
    );

    std::match_results<PPInput::Iterator> match;
    std::vector<PPToken> result;
    PPInput pp_input { input };
    auto iter = pp_input.begin(), end = pp_input.end();

    auto locus = Locus { file, 1, 1 };
    while (std::regex_search(iter, end, match, patterns)) {
        PPToken token { locus, PPToken::kInvalid, match.str(0) };
#include "pptoken.match.generated.cpp"
        result.push_back(token);

        iter += match.length();
        locus = calculate_locus(file, input, iter.ptr);
    }
    if (!result.empty() && result.back().kind != PPToken::kNewLine) {
        result.push_back(PPToken { calculate_locus(file, input, iter.ptr), PPToken::kNewLine, "" });
    }
    return result;
}
