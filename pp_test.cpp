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
    case PPToken::kAuto:
        return "keyword auto";
    case PPToken::kBreak:
        return "keyword break";
    case PPToken::kCase:
        return "keyword case";
    case PPToken::kChar:
        return "keyword char";
    case PPToken::kConst:
        return "keyword const";
    case PPToken::kContinue:
        return "keyword continue";
    case PPToken::kDefault:
        return "keyword default";
    case PPToken::kDo:
        return "keyword do";
    case PPToken::kDouble:
        return "keyword double";
    case PPToken::kElse:
        return "keyword else";
    case PPToken::kEnum:
        return "keyword enum";
    case PPToken::kExtern:
        return "keyword extern";
    case PPToken::kFloat:
        return "keyword float";
    case PPToken::kFor:
        return "keyword for";
    case PPToken::kGoto:
        return "keyword goto";
    case PPToken::kIf:
        return "keyword if";
    case PPToken::kInt:
        return "keyword int";
    case PPToken::kLong:
        return "keyword long";
    case PPToken::kRegister:
        return "keyword register";
    case PPToken::kReturn:
        return "keyword return";
    case PPToken::kShort:
        return "keyword short";
    case PPToken::kSigned:
        return "keyword signed";
    case PPToken::kSizeof:
        return "keyword sizeof";
    case PPToken::kStatic:
        return "keyword static";
    case PPToken::kStruct:
        return "keyword struct";
    case PPToken::kSwitch:
        return "keyword switch";
    case PPToken::kTypedef:
        return "keyword typedef";
    case PPToken::kUnion:
        return "keyword union";
    case PPToken::kUnsigned:
        return "keyword unsigned";
    case PPToken::kVoid:
        return "keyword void";
    case PPToken::kVolatile:
        return "keyword volatile";
    case PPToken::kWhile:
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
