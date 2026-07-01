#include "ast.h"

Rule *Name::as_rule(Grammar &grammar) const {
  if (grammar.symbol_table.contains(*this)) {
    return grammar.symbol_table.at(*this);
  }
  return nullptr;
}

const Rule *Name::as_rule(const Grammar &grammar) const {
  if (grammar.symbol_table.contains(*this)) {
    return grammar.symbol_table.at(*this);
  }
  return nullptr;
}

Name *Primary::as_name()  {
  return std::get_if<Name>(&value);
}

const Name *Primary::as_name() const {
  return std::get_if<Name>(&value);
}

Rule *Primary::as_rule(Grammar &grammar) const {
  if (auto *name = as_name()) {
    if (grammar.symbol_table.contains(*name)) {
      return grammar.symbol_table.at(*name);
    }
  }
  return nullptr;
}

const Rule *Primary::as_rule(const Grammar &grammar) const {
  if (auto *name = as_name()) {
    return name->as_rule(grammar);
  }
  return nullptr;
}

bool Primary::is_dot() const {
  return std::get_if<Dot>(&value) != nullptr;
}

Expression *Primary::as_grouping() {
  return std::get_if<Expression>(&value);
}

const Expression *Primary::as_grouping() const {
  return std::get_if<Expression>(&value);
}

const std::string *Primary::as_number() const {
  return std::get_if<std::string>(&value);
}

const Square *Primary::as_square() const {
  return std::get_if<Square>(&value);
}

const char *Primary::as_char() const {
  return std::get_if<char>(&value);
}

BoundExpression *Rule::as_bound_expression() {
  return std::get_if<BoundExpression>(&value);
}

const BoundExpression *Rule::as_bound_expression() const {
  return std::get_if<BoundExpression>(&value);
}

OrExpression *Rule::as_alternative() {
  return std::get_if<OrExpression>(&value);
}

const OrExpression *Rule::as_alternative() const {
  return std::get_if<OrExpression>(&value);
}

PayloadUnpack *Rule::as_unpack() {
  return std::get_if<PayloadUnpack>(&value);
}

const PayloadUnpack *Rule::as_unpack() const {
  return std::get_if<PayloadUnpack>(&value);
}

Mapping *Rule::as_mapping() {
  return std::get_if<Mapping>(&value);
}

const Mapping *Rule::as_mapping() const {
  return std::get_if<Mapping>(&value);
}
