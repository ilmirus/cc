#include "pp_test.h"

#include <iostream>
#include <sstream>
#include <assert.h>

#include "file_utils.h"
#include "string_utils.h"

static std::string stringify(const PPToken &pp_token) {
    using namespace std::string_literals;

    switch (pp_token.kind) {
    case PPToken::kWhitespace:
        return "whitespace";
    case PPToken::kKwAuto:
        return "keyword auto";
    case PPToken::kKwBreak:
        return "keyword break";
    case PPToken::kKwCase:
        return "keyword case";
    case PPToken::kKwChar:
        return "keyword char";
    case PPToken::kKwConst:
        return "keyword const";
    case PPToken::kKwContinue:
        return "keyword continue";
    case PPToken::kKwDefault:
        return "keyword default";
    case PPToken::kKwDo:
        return "keyword do";
    case PPToken::kKwDouble:
        return "keyword double";
    case PPToken::kKwElse:
        return "keyword else";
    case PPToken::kKwEnum:
        return "keyword enum";
    case PPToken::kKwExtern:
        return "keyword extern";
    case PPToken::kKwFloat:
        return "keyword float";
    case PPToken::kKwFor:
        return "keyword for";
    case PPToken::kKwGoto:
        return "keyword goto";
    case PPToken::kKwIf:
        return "keyword if";
    case PPToken::kKwInt:
        return "keyword int";
    case PPToken::kKwLong:
        return "keyword long";
    case PPToken::kKwRegister:
        return "keyword register";
    case PPToken::kKwReturn:
        return "keyword return";
    case PPToken::kKwShort:
        return "keyword short";
    case PPToken::kKwSigned:
        return "keyword signed";
    case PPToken::kKwSizeof:
        return "keyword sizeof";
    case PPToken::kKwStatic:
        return "keyword static";
    case PPToken::kKwStruct:
        return "keyword struct";
    case PPToken::kKwSwitch:
        return "keyword switch";
    case PPToken::kKwTypedef:
        return "keyword typedef";
    case PPToken::kKwUnion:
        return "keyword union";
    case PPToken::kKwUnsigned:
        return "keyword unsigned";
    case PPToken::kKwVoid:
        return "keyword void";
    case PPToken::kKwVolatile:
        return "keyword volatile";
    case PPToken::kKwWhile:
        return "keyword while";
    case PPToken::kNewLine:
        return "new-line";
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
    case PPToken::kNonWhiteSpace:
        return "non-whitespace "s + pp_token.value;
    case PPToken::kInvalid:
        return "non-whitespace "s + pp_token.value;
    default:
        assert(0 && "unreachable");
    }
}

static std::string stringify(const std::vector<PPToken> &pp_tokens) {
    std::string result;
    for (auto &pp_token : pp_tokens) {
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
        "100-example",
        "100-floating",
        "100-integer-zero",
        "100-line-splice",
        "100-partial-comment",
        "100-partial-string-literal",
        "100-keywords"
    };

    for (auto &test : tests) {
        std::cout << "Running " << test << "...";

        auto input = slurp("tests/" + test + ".c");
        auto expected = slurp_lines("tests/" + test + ".ref");
        std::vector<PPToken> got;
        try {
            got = pp_scan(test, input);
        } catch (const std::runtime_error& ex) {
            if (expected[0].starts_with("ERROR")) {
                std::cout << " OK\n";
                continue;
            }
        }

        for (size_t i = 0; i < std::min(got.size(), expected.size()); i++) {
            if (expected[i] != stringify(got[i])) {
                std::stringstream message;
                message << test << ": mismatch on line " << std::to_string(i + 1);
                message << " expected:\n" + expected[i] + "\ngot\n" + stringify(got[i]) << "\n";
                message << "EXPECTED:\n";
                for (auto &line: expected) {
                    message << line << "\n";
                }
                message << "GOT:\n";
                for (auto &token: got) {
                    message << stringify(token) << "\n";
                }
                throw std::logic_error(message.str());
            }
        }

        if (got.size() != expected.size()) {
            throw std::logic_error(
                test + ": size mismatch expected:\n" + join_lines(expected) + "\ngot\n" + stringify(got));
        }

        std::cout << " OK\n";
    }
}
