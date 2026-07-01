#include "pretty_print.h"

#include <stdexcept>

using namespace std::string_literals;

void pretty_print(std::ostream &os, const Name &name, const Grammar &grammar) {
  if (grammar.symbol_table.contains(name)) {
    os << "<" << name.value << "(" << grammar.symbol_table.at(name)->color << ")>";
  } else {
    os << name.value;
  }
}

static void pretty_print(std::ostream &os, const Expression &expression, const Grammar &grammar);

static void pretty_print(std::ostream &os, const Square &square) {
  os << "[";
  if (square.negation) {
    os << "^";
  }
  for (const auto &[start, end]: square.ranges) {
    os << start;
    if (end != 0) {
      os << "-" << end;
    }
  }
  os << "]";
}

static void pretty_print(std::ostream &os, const Primary &primary, const Grammar &grammar) {
  if (auto *name = primary.as_name()) {
    pretty_print(os, *name, grammar);
  } else if (primary.is_dot()) {
    os << ".";
  } else if (auto *grouping = primary.as_grouping()) {
    os << "(";
    pretty_print(os, *grouping, grammar);
    os << ")";
  } else if (auto *number = primary.as_number()) {
    os << *number;
  } else if (auto *square = primary.as_square()) {
    pretty_print(os, *square);
  } else if (auto *c = primary.as_char()) {
    os << *c;
  } else {
    throw std::logic_error("unreachable");
  }

  switch (primary.suffix) {
    case Primary::kNone:
      break;
    case Primary::kZeroOrOne:
      os << "?";
      break;
    case Primary::kZeroOrMore:
      os << "*";
      break;
    case Primary::kOneOrMore:
      os << "+";
      break;
  }
}

static void pretty_print(std::ostream &os, const Binding &binding, const Grammar &grammar) {
  if (!binding.binding.empty()) {
    os << binding.binding << ":";
  }
  pretty_print(os, binding.primary, grammar);
}

static void pretty_print(std::ostream &os, const Sequence &sequence, const Grammar &grammar) {
  for (size_t i = 0, first = true; i < sequence.size(); i++, first = false) {
    if (!first) {
      os << " ";
    }
    pretty_print(os, sequence[i], grammar);
  }
}

static void pretty_print(std::ostream &os, const Expression &expression, const Grammar &grammar) {
  for (size_t i = 0, first = true; i < expression.size(); i++, first = false) {
    if (!first) {
      os << " | ";
    }
    pretty_print(os, expression[i], grammar);
  }
}

static void pretty_print(std::ostream &os, const OrExpression &expr, const Grammar &grammar) {
  pretty_print(os, expr.expr, grammar);
  if (!expr.action.empty()) {
    os << " " << expr.action;
  }
}

static void pretty_print(std::ostream &os, const BoundExpression &expr, const Grammar &grammar) {
  for (size_t i = 0, first = true; i < expr.bindings.size(); i++, first = false) {
    if (!first) {
      os << " ";
    }
    pretty_print(os, expr.bindings[i], grammar);
  }
  if (!expr.action.empty()) {
    os << " " << expr.action;
  }
}

static void pretty_print(std::ostream &os, const PayloadUnpack &pa, const Grammar &grammar) {
  pretty_print(os, pa.mapping_name, grammar);
  os << " ~ ";
  if (pa.expr.has_value()) {
    pretty_print(os, pa.expr.value(), grammar);
  }
  if (!pa.action.empty()) {
    os << " " << pa.action;
  }
}

static void pretty_print(std::ostream &os, const Mapping &m) {
  os << "if " << m.condition;
  if (!m.payload_type.empty()) {
    os << " ~ " << m.payload_type << " " << m.unpack_action;
  }
}

void pretty_print(std::ostream &os, const Rule &rule, const Grammar &grammar) {
  pretty_print(os, rule.name, grammar);

  os << " = ";

  if (auto *bound_expr = rule.as_bound_expression()) {
    pretty_print(os, *bound_expr, grammar);
  } else if (auto *alternative = rule.as_alternative()) {
    pretty_print(os, *alternative, grammar);
  } else if (auto *unpack = rule.as_unpack()) {
    pretty_print(os, *unpack, grammar);
  } else if (auto *mapping = rule.as_mapping()) {
    pretty_print(os, *mapping);
  } else {
    throw std::logic_error("unreachable " + rule.name.value);
  }
}

void pretty_print(std::ostream &os, const Grammar &grammar) {
  for (const auto &rule: grammar.rules) {
    pretty_print(os, rule, grammar);
    os << "\n";
  }
}