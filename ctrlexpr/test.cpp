#include <format>
#include <iostream>
#include <vector>

#include "ctrlexpr.h"
#include "utils/file_utils.h"
#include "pptoken/pptoken.h"

std::string stringify(const CtrlExpr &ce) {
  switch (ce.kind) {
    case CtrlExpr::kError:
      return "error";
    case CtrlExpr::kSigned:
      return std::to_string(ce.s);
    case CtrlExpr::kUnsigned:
      return std::to_string(ce.u) + "u";
  }
  throw std::logic_error("unreachable");
}

void test() {
  std::vector<std::string> tests = {
    "100-primary"
  };

  for (auto &test: tests) {
    std::cout << "Running " << test << "...";

    auto input = slurp_lines("ctrlexpr/tests/" + test + ".c");
    auto expected = slurp_lines("ctrlexpr/tests/" + test + ".ref");
    if (input.size() != expected.size()) {
      throw std::runtime_error("Size mismatch on " + test);
    }
    for (size_t i = 0; i < input.size(); i++) {
      auto pptokens = pp_scan(test, input[i]);
      auto got = ctrlexpr_evaluate(pptokens);
      if (stringify(got) != expected[i]) {
        throw std::runtime_error(
          std::format(
            "Error in test {} in line {} expected {} got {}",
            test, i, expected[i], stringify(got)
          )
        );
      }
    }
    std::cout << " OK\n";
  }
}

int main() {
  test();
}
