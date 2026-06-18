#include "parse.h"

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


// mapping_name = ` [^`]+ `
static Name parse_mapping_name(Input &input) {
  if (input.peek() != '`')
    throw std::logic_error("Expected `, but got "s + input.peek() + " mapping name");
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
  return Name{result};
}

static Name parse_name(Input &input) {
  input.skip_ws();
  if (is_identifier(input.peek()))
    return Name{parse_identifier(input)};
  if (input.peek() == '`')
    return parse_mapping_name(input);
  throw std::runtime_error("Expected identifier or mapping name, but got " + input.rest());
}

static bool is_name_start(const Input &input) { return is_identifier_start(input.peek()) || input.peek() == '`'; }

static Expression parse_expression(Input &input);

// TODO: Support \d \s \w, control escapes, hex escapes, unicode escapes
static char unescape(char c) {
  switch (c) {
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    default:
      return c;
  }
}

// escaped_char = \\ . | .
static char parse_escaped_char(Input &input) {
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
static Square parse_square(Input &input) {
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

  if (input.peek() != ']')
    throw std::runtime_error("Unclosed square " + input.rest());
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
static Primary parse_primary(Input &input) {
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
    if (input.peek() != ')')
      throw std::runtime_error("Expected ')' at the end of grouping: " + input.rest());
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
static Match parse_match(Input &input) {
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

static std::optional<Name> parse_rule_start_safe(Input &input) {
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
static Sequence parse_sequence(Input &input) {
  Sequence result;
  while (true) {
    auto safepoint = input;
    auto rule_start = parse_rule_start_safe(input);
    if (rule_start.has_value()) {
      input = safepoint;
      return result;
    }
    input.skip_ws();
    if (input.peek() == '|' || input.peek() == '{' || input.peek() == ')')
      return result;
    result.emplace_back(parse_match(input));
  }
}

// expression = sequence (\| sequence)*
static Expression parse_expression(Input &input) {
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

static std::optional<PayloadUnpack> parse_payload_unpack_safe(Input &input, const Name &name) {
  if (input.peek() != '`')
    throw std::runtime_error("Payload unpack should start by mapping name " + input.rest());

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

  return PayloadUnpack{mapping_name, expr, action};
}

static OrExpression parse_or_expression(Input &input, const Name &name) {
  auto expr = parse_expression(input);
  input.skip_ws();
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name.value);
  }
  return OrExpression{expr, action};
}

// rule = rule_start (expression action?) | (mapping_name ~ expression? action)
static std::optional<Rule> parse_rule_safe(Input &input) {
  auto rule_start = parse_rule_start_safe(input);
  if (!rule_start.has_value())
    return {};
  auto name = rule_start.value();

  input.skip_ws();
  // Parse payload unpack first and then, if no `~` found, parse expression
  if (input.peek() == '`') {
    auto payload_unpack = parse_payload_unpack_safe(input, name);
    if (payload_unpack.has_value()) {
      return Rule{name, "", payload_unpack.value()};
    }
  }

  return Rule{name, "", parse_or_expression(input, name)};
}

static std::string parse_initial_color(Input &input) {
  if (input.peek() != '%')
    throw std::logic_error("Expected % at the start of directive " + input.rest());
  input.skip();

  if (!input.starts_with("input"))
    throw std::runtime_error("Unknown directive: %" + input.rest());
  input.skip(5);

  input.skip_ws();
  if (input.peek() != '=')
    throw std::runtime_error("Expected '=' after %input directive: " + input.rest());
  input.skip();

  input.skip_ws();
  return parse_identifier(input);
}

// mapping_rule = mapping_name if ([^)+) (~ identifier action)?
static Rule parse_mapping_rule(Input &input) {
  auto name = parse_mapping_name(input);

  input.skip_ws();
  if (input.peek() != 'i' && input.peek(1) != 'f')
    throw std::runtime_error("Expected 'if' after mapping name " + input.rest());
  input.skip(2);

  input.skip_ws();
  auto condition = parse_grouping(input, name.value, '(', ')');

  input.skip_ws();
  if (input.peek() != '~') {
    return Rule{name, "", Mapping{condition, "", ""}};
  }

  input.skip();
  input.skip_ws();
  auto type = parse_identifier(input);
  input.skip_ws();
  auto action = parse_action(input, name.value);
  return Rule{name, "", Mapping{condition, type, action}};
}

Grammar parse_grammar(Input &input) {
  Grammar result;
  while (true) {
    input.skip_ws();
    if (input.peek() == 0)
      break;
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