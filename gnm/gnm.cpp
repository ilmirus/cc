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
// sequence = match*
// match = !rule_start (identifier :)? primary (\? | \* | \+)?
// primary = name
//         | \.
//         | \( expression \)
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

Primary parse_primary(Input& input) {}

// sequence = ((identifier :)? primary (\? | \* | \+)?)*
Sequence parse_sequence(Input& input) {
  std::string binding;
  if (is_identifier_start(input.peek())) {
    auto safepoint = input;
    binding = parse_identifier(input);
    input.skip_ws();
    if (input.peek() == ':') {
      input.skip();
      input.skip_ws();
    } else {
      input = safepoint;
    }
  }

  parse_primary(input);
  input.skip_ws();
  Primary::Suffix suffix;
  switch (input.peek()) {
    case '?':
      suffix = Primary::kZeroOrOne;
      input.skip();
      break;
    case '*':
      suffix = Primary::kZeroOrMore;
      input.skip();
      break;
    case '+':
      suffix = Primary::kOneOrMore;
      input.skip();
      break;
    default:
      suffix = Primary::kNone;
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

// rule = name \= (expression action?) | (mapping_name ~ expression? action)
Rule parse_rule(Input& input) {
  auto name = parse_name(input);

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
