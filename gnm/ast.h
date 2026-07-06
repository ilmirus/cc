#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Primary;
using Sequence = std::vector<Primary>;
using Expression = std::vector<Sequence>;
struct Rule;
struct Grammar;

struct Name {
  std::string value;

  bool operator==(const Name& other) const {
    return value == other.value;
  }

  Rule *as_rule(Grammar &) const;
  const Rule *as_rule(const Grammar &) const;

  struct Hash {
    std::size_t operator()(const Name &name) const noexcept {
      return std::hash<std::string>{}(name.value);
    }
  };
};

struct Dot {};

struct Square {
  bool negation = false;
  std::vector<std::pair<char, char>> ranges;
};

struct Primary {
  std::variant<
    Name,
    Dot,
    Expression,
    std::string,
    Square,
    char
  > value;

  enum Suffix {
    kNone,
    kZeroOrOne,
    kZeroOrMore,
    kOneOrMore
  } suffix = kNone;

  Name *as_name();
  const Name *as_name() const;

  Rule *as_rule(Grammar &) const;
  const Rule *as_rule(const Grammar &) const;

  bool is_dot() const;

  Expression *as_grouping();
  const Expression *as_grouping() const;

  const std::string *as_number() const;

  const Square *as_square() const;

  const char* as_char() const;
};

struct Binding {
  std::string binding;
  Primary primary;
};

struct BoundExpression {
  std::vector<Binding> bindings;
  std::string action;
};

struct PayloadUnpack {
  Name mapping_name;
  std::optional<BoundExpression> expr;
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

  std::variant<
    BoundExpression,
    OrExpression,
    PayloadUnpack,
    Mapping
  > value;

  BoundExpression *as_bound_expression();
  const BoundExpression *as_bound_expression() const;

  OrExpression *as_alternative();
  const OrExpression *as_alternative() const;

  PayloadUnpack *as_unpack();
  const PayloadUnpack *as_unpack() const;

  Mapping *as_mapping();
  const Mapping *as_mapping() const;
};

struct Grammar {
  std::string initial_color;
  std::unordered_map<Name, Rule*, Name::Hash> symbol_table;
  std::vector<Rule> rules;
};