#include <optional>
#include <stdexcept>
#include <vector>
#include <string>

#include "lex/grammar_common.h"

using namespace std::string_literals;

// grammar file

// file = rule*
// rule = name \= (mapping_name ~ expression? action?) | (expression action?)
// name = identifier | mapping_name
// expression = sequence (\| sequence)*
// sequence = ((identifier :)? primary (\? | \* | \+)?)*
// primary = name
//         | terminal
//         | \.
//         | \( expression \)
// action = { [^}]+ }
// mapping_name = ` [^`]+ `

bool trace = false;

struct Expression {};

struct Rule {
  std::optional<Expression> expr;
  std::string mapping_name;
  bool is_payload_unpack;
  std::string action;
};

struct Name {
  std::string value;
  bool is_mapping;
};

// mapping_name = ` [^`]+ `
std::string parse_mapping_name(Input& input) {
  if (input.peek() != '`') throw std::logic_error("Expected `, but got "s + input.peek() + " mapping name");
  std::string result;
  result += input.next();
  while (input.peek() != '`') {
    switch (input.peek()) {
      case '\n':
      case 0:
        throw std::runtime_error("Unclosed mapping name " + result);
      default:
        result += input.next();
    }
  }
  result += input.next();
  return result;
}

Name parse_name(Input& input) {
  skip_ws(input);
  if (is_identifier(input.peek())) return Name { parse_identifier(input), false };
  else if (input.peek() == '`') return Name { parse_mapping_name(input), true };
  else throw std::runtime_error("Expected identifier or mapping name, but got " + input.rest());
}

Expression parse_expression(Input& input) {}

// rule = name \= (expression action?) | (mapping_name ~ expression? action)
Rule parse_rule(Input& input) {
  auto name = parse_name(input);

  skip_ws(input);
  if (input.peek() != '=') throw std::runtime_error("Expected '=' a"s);

  skip_ws(input);
  // Parse payload unpack first and then, if no `~` found, parse expression
  if (input.peek() == '`') {
    auto safepoint = input.offset;
    auto mappint_name = parse_mapping_name(input);

    skip_ws(input);
    if (input.peek() != '~') {
      input.offset = safepoint;
      goto EXPRESSION;
    }
    input.skip();

    skip_ws(input);
    std::optional<Expression> expr;
    if (input.peek() != '{') {
      expr.emplace(parse_expression(input));
    }

    skip_ws(input);
    std::string action;
    if (input.peek() == '{') {
      action = parse_action(input, name.value);
    }
    return Rule { expr, mappint_name, true, action };
  }

EXPRESSION:
  std::optional<Expression> expr = parse_expression(input);
  skip_ws(input);
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name.value);
  }
  return Rule { expr, "", false, action };
}

std::vector<Rule> parse(Input& input) {
  std::vector<Rule> result;
  while (input.peek() != 0) {
    skip_ws(input);
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
