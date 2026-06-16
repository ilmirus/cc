#include <algorithm>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "lex/grammar_common.h"
#include "utils/file_utils.h"
#include "ast.h"

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
  return Name { result };
}

Name parse_name(Input& input) {
  input.skip_ws();
  if (is_identifier(input.peek())) return Name { parse_identifier(input) };
  if (input.peek() == '`') return parse_mapping_name(input);
  throw std::runtime_error("Expected identifier or mapping name, but got " + input.rest());
}

bool is_name_start(const Input& input) {
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

std::optional<PayloadUnpack> parse_payload_unpack_safe(Input& input, const Name& name) {
  if (input.peek() != '`') throw std::runtime_error("Payload unpack should start by mapping name " + input.rest());

  auto safepoint = input;
  auto mapping_name = parse_mapping_name(input);

  input.skip_ws();
  if (input.peek() != '~') {
    input = safepoint;
    return {};
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

  return PayloadUnpack { mapping_name, expr, action };
}

OrExpression parse_alternative_expression(Input& input, const Name& name) {
  auto expr = parse_expression(input);
  input.skip_ws();
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name.value);
  }
  return OrExpression { expr, action };
}

// rule = rule_start (expression action?) | (mapping_name ~ expression? action)
std::optional<Rule> parse_rule_safe(Input& input) {
  auto rule_start = parse_rule_start_safe(input);
  if (!rule_start.has_value()) return {};
  auto name = rule_start.value();

  input.skip_ws();
  // Parse payload unpack first and then, if no `~` found, parse expression
  if (input.peek() == '`') {
    auto payload_unpack = parse_payload_unpack_safe(input, name);
    if (payload_unpack.has_value()) {
      return Rule { name, "", payload_unpack.value() };
    }
  }

  return Rule { name, "", parse_alternative_expression(input, name) };
}

std::string parse_initial_color(Input &input) {
  if (input.peek() != '%') throw std::logic_error("Expected % at the start of directive " + input.rest());
  input.skip();

  if (!input.starts_with("input")) throw std::runtime_error("Unknown directive: %" + input.rest());
  input.skip(5);

  input.skip_ws();
  if (input.peek() != '=') throw std::runtime_error("Expected '=' after %input directive: " + input.rest());
  input.skip();

  input.skip_ws();
  return parse_identifier(input);
}

// mapping_rule = mapping_name if ([^)+) (~ identifier action)?
Rule parse_mapping_rule(Input& input) {
  auto name = parse_mapping_name(input);

  input.skip_ws();
  if (input.peek() != 'i' && input.peek(1) != 'f')
    throw std::runtime_error("Expected 'if' after mapping name " + input.rest());
  input.skip(2);

  input.skip_ws();
  auto condition = parse_grouping(input, name.value, '(', ')');

  input.skip_ws();
  if (input.peek() != '~') {
    return Rule { name, "", Mapping { condition, "", "" } };
  }

  input.skip();
  input.skip_ws();
  auto type = parse_identifier(input);
  input.skip_ws();
  auto action = parse_action(input, name.value);
  return Rule{name, "", Mapping{condition, type, action}};
}

Grammar parse_grammar(Input& input) {
  Grammar result;
  while (true) {
    input.skip_ws();
    if (input.peek() == 0) break;
    if (input.peek() == '%') {
      result.initial_color = parse_initial_color(input);
    } else {
      if (auto rule = parse_rule_safe(input); rule.has_value()) {
        result.rules.emplace_back(rule.value());
      } else {
        result.rules.emplace_back(parse_mapping_rule(input));
      }
    }
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Name& name) {
  if (name.resolved_to != nullptr) {
    os << "<" << name.value << "(" << name.resolved_to->color << ")>";
  } else {
    os << name.value;
  }
  return os;
}

void extract_names(Grammar &grammar) {
  std::set<std::string> seen;
  for (auto &rule: grammar.rules) {
    if (seen.contains(rule.name.value))
      throw std::runtime_error("Duplicate rule "s + rule.name.value);
    seen.insert(rule.name.value);
    rule.name.resolved_to = &rule;
    grammar.names.emplace_back(rule.name);
  }
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
  for (size_t i = 0, first = true; i < sequence.size(); i++, first = false) {
    if (!first) {
      os << " ";
    }
    os << sequence[i];
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Expression& expression) {
  for (size_t i = 0, first = true; i < expression.size(); i++, first = false) {
    if (!first) {
      os << " | ";
    }
    os << expression[i];
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const OrExpression& expr) {
  os << expr.expr << " " << expr.action;
  return os;
}

std::ostream& operator<<(std::ostream& os, const PayloadUnpack& pa) {
  os << pa.mapping_name << " ~ ";
  if (pa.expr.has_value()) {
    os << pa.expr.value() << " ";
  }
  os << pa.action;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Mapping& m) {
  os << " if " << m.condition;
  if (!m.payload_type.empty()) {
    os << " ~ " << m.payload_type << " " << m.unpack_action;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Rule& rule) {
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

std::ostream& operator<<(std::ostream& os, const Grammar& grammar) {
  for (const auto& name: grammar.names) {
    os << *name.resolved_to << "\n";
  }
  return os;
}

void resolve(Name* name, const Grammar& grammar) {
  for (auto &cursor: grammar.names) {
    if (cursor == *name) {
      name->resolved_to = cursor.resolved_to;
    }
  }
}

void resolve(Expression* expression, const Grammar& grammar) {
  for (auto& sequence: *expression) {
    for (auto&[_, primary]: sequence) {
      switch (primary.value.index()) {
        case Primary::kName:
          resolve(&std::get<Name>(primary.value), grammar);
          break;
        case Primary::kGrouping:
          resolve(&std::get<Expression>(primary.value), grammar);
          break;
        default:
          // Nothing to do
          break;
      }
    }
  }
}

void resolve(const Grammar& grammar) {
  for (const auto&[_, rule]: grammar.names) {
    switch (rule->value.index()) {
      case Rule::kOrExpression:
        resolve(&std::get<OrExpression>(rule->value).expr, grammar);
        break;
      case Rule::kPayloadUnpack: {
        resolve(&std::get<PayloadUnpack>(rule->value).mapping_name, grammar);
        if (auto& expr = std::get<PayloadUnpack>(rule->value).expr; expr.has_value()) {
          resolve(&expr.value(), grammar);
        }
        break;
      }
      default:
        // Nothing to do
        break;
    }
  }
}

void color(Rule *, const std::string&);

void color(Expression *expr, const std::string& new_color) {
  std::vector<Name *> result;
  for (auto& seq: *expr) {
    for (auto&[_, primary]: seq) {
      switch (primary.value.index()) {
        case Primary::kName: {
          auto &[_, rule] = std::get<Name>(primary.value);
          if (rule != nullptr) {
            color(rule, new_color);
          }
          break;
        }
        case Primary::kGrouping:
          color(&std::get<Expression>(primary.value), new_color);
          break;
        default:
          break;
      }
    }
  }
}

void color(Rule* rule, const std::string& initial_color) {
  if (!rule->color.empty()) {
    if (rule->color != initial_color) {
      std::stringstream ss;
      ss << "Rule: " << *rule << " already has color different to " << initial_color;
      throw std::runtime_error(ss.str());
    }
    return;
  }
  rule->color = initial_color;
  switch (rule->value.index()) {
    case Rule::kOrExpression:
      color(&std::get<Rule::kOrExpression>(rule->value).expr, initial_color);
      break;
    case Rule::kPayloadUnpack: {
      if (auto &[mapping_name, unpacking, _] = std::get<PayloadUnpack>(rule->value); mapping_name.resolved_to != nullptr) {
        const auto new_color = std::get<Mapping>(mapping_name.resolved_to->value).payload_type;
        color(mapping_name.resolved_to, initial_color);
        if (unpacking.has_value()) {
          color(&unpacking.value(), new_color);
        }
      }
    }
    default:
      break;
  }
}

void color(const Grammar& grammar) {
  if (grammar.names.empty()) return;
  color(grammar.names[0].resolved_to, grammar.initial_color);
}

void prepend_indent(std::stringstream &ss, size_t indent) {
  for (size_t i = 0; i < indent; i++) {
    ss << ' ';
  }
}

void generate(std::stringstream &ss, const Expression &expr, const std::string &action, size_t indent) {
  prepend_indent(ss, indent);
  ss << "auto safepoint = input;\n";
  for (auto &seq: expr) {

  }
}

void generate(std::stringstream &ss, const OrExpression &expr, size_t indent) {
  generate(ss, expr.expr, expr.action, indent);
}

void generate(std::stringstream &ss, const Rule &rule) {
  ss << "// " << rule << "\n";
  ss << "auto " << rule.name << "(" << rule.color << " &input) {\n";
  switch (rule.value.index()) {
    case Rule::kOrExpression:
      generate(ss, std::get<Rule::kOrExpression>(rule.value), 2);
      break;
    default:
      throw std::logic_error("unreachable");
  }
}

void generate(std::stringstream &ss, const Grammar &grammar) {
  for (auto &rule: grammar.rules) {
    generate(ss, rule);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) throw std::runtime_error("Expected grammar name as an argument");
  auto name = std::string(argv[1]);
  auto raw_grammar = slurp(name);
  auto input = Input(raw_grammar);
  auto grammar = parse_grammar(input);
  extract_names(grammar);
  std::cout << "Unresolved:\n"<< grammar << "\n";
  resolve(grammar);
  color(grammar);
  std::cout << "Resolved:\n"<< grammar << "\n";
  return 0;
}
