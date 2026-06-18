#include "pretty_print.h"

#include <stdexcept>

using namespace std::string_literals;

std::ostream &operator<<(std::ostream &os, const Name &name) {
  if (name.resolved_to != nullptr) {
    os << "<" << name.value << "(" << name.resolved_to->color << ")>";
  } else {
    os << name.value;
  }
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Expression &expression);

static std::ostream &operator<<(std::ostream &os, const Square &square) {
  os << "[";
  if (square.negation) {
    os << "^";
  }
  for (const auto &[start, end]: square.ranges) {
    os << start;
    if (end != 0) {
      os << "-";
    }
    os << end;
  }
  os << "]";
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Primary &primary) {
  switch (primary.value.index()) {
    case Primary::kName:
      os << std::get<Primary::kName>(primary.value);
      break;
    case Primary::kDot:
      os << ".";
      break;
    case Primary::kGrouping:
      os << "(" << std::get<Primary::kGrouping>(primary.value) << ")";
      break;
    case Primary::kNumber:
      os << std::get<Primary::kNumber>(primary.value);
      break;
    case Primary::kSquare:
      os << std::get<Primary::kSquare>(primary.value);
      break;
    case Primary::kChar:
      os << std::get<Primary::kChar>(primary.value);
      break;
    default:
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
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Match &match) {
  if (!match.binding.empty()) {
    os << match.binding << ":";
  }
  os << match.primary;
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Sequence &sequence) {
  for (size_t i = 0, first = true; i < sequence.size(); i++, first = false) {
    if (!first) {
      os << " ";
    }
    os << sequence[i];
  }
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Expression &expression) {
  for (size_t i = 0, first = true; i < expression.size(); i++, first = false) {
    if (!first) {
      os << " | ";
    }
    os << expression[i];
  }
  return os;
}

static std::ostream &operator<<(std::ostream &os, const OrExpression &expr) {
  os << expr.expr << " " << expr.action;
  return os;
}

static std::ostream &operator<<(std::ostream &os, const PayloadUnpack &pa) {
  os << pa.mapping_name << " ~ ";
  if (pa.expr.has_value()) {
    os << pa.expr.value() << " ";
  }
  os << pa.action;
  return os;
}

static std::ostream &operator<<(std::ostream &os, const Mapping &m) {
  os << " if " << m.condition;
  if (!m.payload_type.empty()) {
    os << " ~ " << m.payload_type << " " << m.unpack_action;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Rule &rule) {
  os << rule.name;

  switch (rule.value.index()) {
    case Rule::kOrExpression:
      os << " = " << std::get<Rule::kOrExpression>(rule.value);
      break;
    case Rule::kPayloadUnpack:
      os << " = " << std::get<Rule::kPayloadUnpack>(rule.value);
      break;
    case Rule::kMapping:
      os << std::get<Rule::kMapping>(rule.value);
      break;
    default:
      throw std::logic_error("unreachable " + rule.name.value);
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Grammar &grammar) {
  for (const auto &name: grammar.names) {
    os << *name.resolved_to << "\n";
  }
  return os;
}