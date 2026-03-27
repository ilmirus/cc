#include <cstdint>
#include <vector>
#include "pptoken/pptoken.h"

struct CtrlExpr {
  enum Kind {
    kError,
    kSigned,
    kUnsigned
  } kind;

  union {
    uint64_t u;
    int64_t s;
  };
};

CtrlExpr ctrlexpr_evaluate(const std::vector<PPToken>&);
