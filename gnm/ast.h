#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

struct Match;
using Sequence = std::vector<Match>;
using Expression = std::vector<Sequence>;
struct Rule;

struct Name {
  std::string value;
  Rule* resolved_to = nullptr;

  bool operator==(const Name& other) const {
    return value == other.value;
  }
};

struct Dot {};

struct Square {
  bool negation = false;
  std::vector<std::pair<char, char>> ranges;
};

struct Primary {
  enum Kind {
    kName,
    kDot,
    kGrouping,
    kNumber,
    kSquare,
    kChar
  };

  std::variant<Name, Dot, Expression, std::string, Square, char> value;

  enum Suffix {
    kNone,
    kZeroOrOne,
    kZeroOrMore,
    kOneOrMore
  } suffix = kNone;
};

struct Match {
  std::string binding;
  Primary primary;
};

struct PayloadUnpack {
  Name mapping_name;
  std::optional<Expression> expr;
  std::string action;
};

struct OrExpression {
  Expression expr;
  std::string action;
};

struct Mapping {
  std::string condition;
  std::string payload_type;
  std::string unpack_action;
};

struct Rule {
  Name name;
  std::string color;

  enum Kind {
    kOrExpression,
    kPayloadUnpack,
    kMapping
  };

  std::variant<OrExpression, PayloadUnpack, Mapping> value;
};

struct Grammar {
  std::string initial_color;
  std::vector<Name> names;
  std::vector<Rule> rules;
};