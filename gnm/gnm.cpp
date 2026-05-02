#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>
#include <string>

#include "lex/grammar_common.h"

using namespace std::string_literals;

// grammar file

// file = rule*
// rule = rule_start (mapping_name ~ expression? action?) | (expression action?)
// rule_start = ^ name \=
// name = identifier | mapping_name
// expression = sequence (\| sequence)*
// sequence = (!rule_start match)*
// match = (identifier :)? primary primary_suffix
// primary = name
//         | \.
//         | \( expression \)
// primary_suffix = (\? | \* | \+)?
// action = { [^}]+ }
// mapping_name = ` [^`]+ `

bool trace = false;

struct Match;
using Sequence = std::vector<Match>;
using Expression = std::vector<Sequence>;

struct Name {
  std::string value;
  bool is_mapping;
};

struct Dot {};

struct Primary {
  enum Kind {
    kName,
    kDot,
    kGrouping
  };

  std::variant<Name, Dot, Expression> value;

  enum Suffix {
    kNone,
    kZeroOrOne,
    kZeroOrMore,
    kOneOrMore
  } suffix;
};

struct Match {
  std::string binding;
  Primary primary;
};

struct Rule {
  std::optional<Expression> expr;
  std::string mapping_name;
  bool is_payload_unpack;
  std::string action;
};

// mapping_name = ` [^`]+ `
std::string parse_mapping_name(Input& input) {
  if (input.peek() != '`') throw std::logic_error("Expected `, but got "s + input.peek() + " mapping name");
  std::string result;
  result += input.peek();
  input.skip();
  while (input.peek() != '`') {
    switch (input.peek()) {
      case '\n':
      case 0:
        throw std::runtime_error("Unclosed mapping name " + result);
      default:
        result += input.peek();
        input.skip();
    }
  }
  result += input.peek();
  input.skip();
  return result;
}

Name parse_name(Input& input) {
  input.skip_ws();
  if (is_identifier(input.peek())) return Name { parse_identifier(input), false };
  else if (input.peek() == '`') return Name { parse_mapping_name(input), true };
  else throw std::runtime_error("Expected identifier or mapping name, but got " + input.rest());
}

bool is_name_start(Input& input) {
  return is_identifier_start(input.peek()) || input.peek() == '`';
}

Expression parse_expression(Input& input);

// primary = name
//         | \.
//         | \( expression \)
// primary_suffix = (\? | \* | \+)?
Primary parse_primary(Input& input) {
  Primary result;

  input.skip_ws();
  if (is_name_start(input)) {
    result.value = parse_name(input);
  } else if (input.peek() == '.') {
    input.skip();
    result.value = Dot();
  } else if (input.peek() == '(') {
    input.skip();
    input.skip_ws();
    auto expr = parse_expression(input);
    input.skip_ws();
    if (input.peek() != ')') throw std::runtime_error("Expected ')' at the end of grouping: " + input.rest());
    input.skip();
    result.value = expr;
  } else {
    throw std::runtime_error("Exprected primary: " + input.rest());
  }

  input.skip_ws();
  switch (input.peek()) {
    case '?':
      result.suffix = Primary::kZeroOrOne;
      input.skip();
      break;
    case '*':
      result.suffix = Primary::kZeroOrMore;
      input.skip();
      break;
    case '+':
      result.suffix = Primary::kOneOrMore;
      input.skip();
      break;
    default:
      result.suffix = Primary::kNone;
  }
  return result;
}

// match = (identifier :)? primary (\? | \* | \+)?
Match parse_match(Input& input) {
  Match result;

  input.skip_ws();
  if (is_identifier_start(input.peek())) {
    auto safepoint = input;
    result.binding = parse_identifier(input);
    input.skip_ws();
    if (input.peek() == ':') {
      input.skip();
      input.skip_ws();
    } else {
      input = safepoint;
    }
  }

  result.primary = parse_primary(input);
  return result;
}

std::optional<Name> parse_rule_start_safe(Input& input) {
  auto safepoint = input;
  input.skip_ws();
  if (!input.is_line_start) {
    input = safepoint;
    return {};
  }
  if (!is_name_start(input)) {
    input = safepoint;
    return {};
  }
  auto name = parse_name(input);
  input.skip_ws();
  if (input.peek() != '=') {
    input = safepoint;
    return {};
  }
  input.skip();

  return name;
}

// sequence = (!rule_start match)*
Sequence parse_sequence(Input& input) {
  Sequence result;
  while (true) {
    auto rule_start = parse_rule_start_safe(input);
    if (rule_start.has_value()) return result;
    result.emplace_back(parse_match(input));
  }
}

// expression = sequence (\| sequence)*
Expression parse_expression(Input& input) {
  std::vector<Sequence> result;
  input.skip_ws();
  result.emplace_back(parse_sequence(input));
  input.skip_ws();
  while (input.peek() == '|') {
    input.skip();
    input.skip_ws();
    result.emplace_back(parse_sequence(input));
  }
  return result;
}

// rule = rule_start (expression action?) | (mapping_name ~ expression? action)
Rule parse_rule(Input& input) {
  auto rule_start = parse_rule_start_safe(input);
  if (!rule_start.has_value()) throw std::runtime_error("Expected rule start: " + input.rest());
  auto name = rule_start.value();

  input.skip_ws();
  if (input.peek() != '=') throw std::runtime_error("Expected '=' a"s);

  input.skip_ws();
  // Parse payload unpack first and then, if no `~` found, parse expression
  if (input.peek() == '`') {
    auto safepoint = input;
    auto mappint_name = parse_mapping_name(input);

    input.skip_ws();
    if (input.peek() != '~') {
      input = safepoint;
      goto EXPRESSION;
    }
    input.skip();

    input.skip_ws();
    std::optional<Expression> expr;
    if (input.peek() != '{') {
      expr.emplace(parse_expression(input));
    }

    input.skip_ws();
    std::string action;
    if (input.peek() == '{') {
      action = parse_action(input, name.value);
    }
    return Rule { expr, mappint_name, true, action };
  }

EXPRESSION:
  std::optional<Expression> expr = parse_expression(input);
  input.skip_ws();
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name.value);
  }
  return Rule { expr, "", false, action };
}

std::vector<Rule> parse(Input& input) {
  std::vector<Rule> result;
  while (input.peek() != 0) {
    input.skip_ws();
    if (input.peek() == '\n') {
      input.skip();
      continue;
    }
    if (input.peek() == 0) break;
    result.emplace_back(parse_rule(input));
  }
  return result;
}

int main() {

}
