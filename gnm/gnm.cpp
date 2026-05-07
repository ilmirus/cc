#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "lex/grammar_common.h"
#include "utils/file_utils.h"

using namespace std::string_literals;

// grammar file

// file = rule*
// rule = rule_start (mapping_name ~ expression? action?) | (expression action?)
// rule_start = ^ name \=
// name = identifier | mapping_name
// expression = sequence (\| sequence)*
// sequence = (!sequence_terminator match)*
// sequence_terminator = rule_start | \| | \{ | \)
// match = (identifier :)? primary primary_suffix
// primary = name
//         | \.
//         | \( expression \)
//         | number
//         | square
//         | escaped_char
// primary_suffix = (\? | \* | \+)?
// action = { [^}]+ }
// mapping_name = ` [^`]+ `
// number = [0-9]+
// square = \[ \^? square_item* \]
// square_item = square_range
//            | square_char
//            | -
// square_range = square_char - square_char
// square_char = escaped_char | [^\]\\\\n-]
// escaped_char = \\ . | .

bool trace = false;

struct Match;
using Sequence = std::vector<Match>;
using Expression = std::vector<Sequence>;
struct Rule;

struct Name {
  std::string value;
  bool is_mapping = false;
  const Rule* rule = nullptr;

  auto operator<=>(const Name& other) const {
    return value<=>other.value;
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

struct Rule {
  Name name;
  std::optional<Expression> expr;
  std::optional<Name> mapping_name;
  std::string action;
};

// mapping_name = ` [^`]+ `
Name parse_mapping_name(Input& input) {
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
  return Name { result, true };
}

Name parse_name(Input& input) {
  input.skip_ws();
  if (is_identifier(input.peek())) return Name { parse_identifier(input), false };
  if (input.peek() == '`') return parse_mapping_name(input);
  throw std::runtime_error("Expected identifier or mapping name, but got " + input.rest());
}

bool is_name_start(Input& input) {
  return is_identifier_start(input.peek()) || input.peek() == '`';
}

Expression parse_expression(Input& input);

// TODO: Support \d \s \w, control escapes, hex escapes, unicode escapes
char unescape(char c) {
  switch (c) {
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    default: return c;
  }
}

// escaped_char = \\ . | .
char parse_escaped_char(Input& input) {
  if (input.peek() == '\\') {
    input.skip();
    auto result = unescape(input.peek());
    input.skip();
    return result;
  }

  auto result = input.peek();
  input.skip();
  return result;
}

// square = \[ \^? square_item* \]
// square_item = square_range
//            | square_char
//            | -
// square_range = square_char - square_char
// square_char = escaped_char | [^\]\\\\n-]
Square parse_square(Input& input) {
  if (input.peek() != '[')
    throw std::logic_error("Expected [ at the square start " + input.rest());
  input.skip();
  Square result;
  if (input.peek() == '^') {
    input.skip();
    result.negation = true;
  } else {
    result.negation = false;
  }

  while (input.peek() != ']' && input.peek() != '\n') {
    auto start = parse_escaped_char(input);
    if (input.peek() == '-') {
      input.skip();
      auto end = parse_escaped_char(input);
      result.ranges.emplace_back(start, end);
    } else {
      result.ranges.emplace_back(start, 0);
    }
  }

  if (input.peek() != ']') throw std::runtime_error("Unclosed square " + input.rest());
  input.skip();

  return result;
}

// primary = name
//         | \.
//         | \( expression \)
//         | number
//         | square
//         | escaped_char
// primary_suffix = (\? | \* | \+)?
// number = [0-9]+
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
  } else if (isdigit(input.peek())) {
    std::string number;
    while (isdigit(input.peek())) {
      number += input.peek();
      input.skip();
    }
    result.value = number;
  } else if (input.peek() == '[') {
    result.value = parse_square(input);
  } else {
    result.value = parse_escaped_char(input);
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
      result.binding.clear();
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

// sequence = (!sequence_terminator match)*
// sequence_terminator = rule_start | \| | \{ | \)
Sequence parse_sequence(Input& input) {
  Sequence result;
  while (true) {
    auto safepoint = input;
    auto rule_start = parse_rule_start_safe(input);
    if (rule_start.has_value()) {
      input = safepoint;
      return result;
    }
    input.skip_ws();
    if (input.peek() == '|' || input.peek() == '{' || input.peek() == ')') return result;
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
  // Parse payload unpack first and then, if no `~` found, parse expression
  if (input.peek() == '`') {
    auto safepoint = input;
    auto mapping_name = parse_mapping_name(input);

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
    return Rule { name, expr, mapping_name, action };
  }

EXPRESSION:
  auto expr = parse_expression(input);
  input.skip_ws();
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name.value);
  }
  return Rule { name, expr, {}, action };
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

struct Grammar {
  std::vector<Name> rules;
};

Grammar extract_names(std::vector<Rule>& rules) {
  Grammar result;
  std::set<std::string> seen;
  for (Rule& rule: rules) {
    if (seen.contains(rule.name.value))
      throw std::runtime_error("Duplicate rule "s + rule.name.value);
    seen.insert(rule.name.value);
    rule.name.rule = &rule;
    result.rules.emplace_back(rule.name);
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Name& name) {
  if (name.rule != nullptr) {
    os << "<";
  }
  os << name.value << "(" << name.is_mapping << ")";
  if (name.rule != nullptr) {
    os << ">";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Expression& expression);

std::ostream& operator<<(std::ostream& os, const Square& square) {
  os << "[";
  if (square.negation) {
    os << "^";
  }
  for (const auto& [start, end]: square.ranges) {
    os << start;
    if (end != 0) {
      os << "-";
    }
    os << end;
  }
  os << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Primary& primary) {
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
  os << " ";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Match& match) {
  if (!match.binding.empty()) {
    os << match.binding << ":";
  }
  os << match.primary;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Sequence& sequence) {
  for (const auto& match: sequence) {
    os << match;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Expression& expression) {
  for (size_t i = 0, first = true; i < expression.size(); i++, first = false) {
    if (!first) {
      os << "| ";
    }
    os << expression[i];
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Rule& rule) {
  os << rule.name << " = ";
  if (rule.mapping_name.has_value()) {
    os << rule.mapping_name.value() << " ~ ";
    if (rule.expr.has_value()) {
      os << rule.expr.value();
    }
  } else {
    if (rule.expr.has_value()) {
      os << rule.expr.value();
    }
  }
  if (!rule.action.empty()) {
    os << rule.action;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Grammar& grammar) {
  for (const auto& name: grammar.rules) {
    os << *name.rule << "\n";
  }
  return os;
}

int main(int argc, char **argv) {
  if (argc < 2) throw std::runtime_error("Expected grammar name as an argument");
  auto name = std::string(argv[1]);
  auto raw_grammar = slurp(name + ".grammar");
  auto raw_mapping = slurp(name + ".mapping");
  auto input = Input(raw_grammar);
  auto parsed_rules = parse(input);
  auto grammar = extract_names(parsed_rules);
  std::cout << grammar << "\n";
  return 0;
}
