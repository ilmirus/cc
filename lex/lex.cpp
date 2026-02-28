#include "file_utils.h"
#include "string_utils.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <format>
#include <map>
#include <cctype>
#include <ranges>

// program = (rule | new-line)*
// rule = identifier \s* '=' \s* pattern (\s '{' action '}')? new-line
// identifier = ...
// pattern = (grouping | [^\s\n])*
// grouping = parens | square
// parens = '(' (grouping | [^)])* ')'
// square = '[' (grouping | [^\]])* ']'
// action = [^}]*
// new-line = \n

struct Rule {
  std::string name;
  std::string pattern;
  std::string action;
  std::string metarule;
  bool is_inline;
};

bool trace = false;

struct Input {
  std::string raw;
  int offset;

  [[nodiscard]] char peek(int i = 0) const {
    if (offset + i >= raw.size())
      return 0;
    return raw[offset + i];
  }

  void skip(int i = 1) {
    offset += i;
  }

  char next() {
    skip();
    return peek();
  }

  [[nodiscard]] std::string rest() const {
    return raw.substr(offset);
  }
};

void skip_ws(Input &input) {
  while (input.peek() == ' ') {
    input.skip();
  }
}

bool is_identifier_start(char c) {
  return isalpha(c) || c == '_';
}

bool is_identifier(char c) {
  return is_identifier_start(c) || isdigit(c);
}

std::string parse_identifier(Input &input) {
  if (trace)
    std::cout << "\n====\nparse_identifier: " << input.rest() << std::endl;

  assert(is_identifier_start(input.peek()));
  std::string result;
  char c = input.peek();
  while (is_identifier(c)) {
    result += c;
    c = input.next();
  }
  return result;
}

std::string parse_pattern_char(Input &input, const std::string &rule_name) {
  if (trace)
    std::cout << "\n====\nparse_pattern_char: " << input.rest() << std::endl;

  assert(input.peek() != 0);
  char c = input.peek();
  std::string result;
  result += c;
  if (c == '\\') {
    c = input.next();
    if (c == 0)
      throw std::runtime_error("Unclosed escape in " + rule_name);
    result += c;
  }
  input.skip();
  return result;
}

std::string parse_grouping(Input &input, const std::string &rule_name, char begin, char end) {
  if (trace)
    std::cout << "\n====\nparse_grouping: " << input.rest() << std::endl;

  assert(input.peek() == begin);
  std::stringstream result;
  result << begin;
  char c = input.next();
  do {
    switch (c) {
      case 0:
      case '\n':
        throw std::runtime_error(
          std::format(
            "Unclosed grouping {}..{} in {}",
            begin, end, rule_name
          )
        );
      case '(':
        result << parse_grouping(input, rule_name, '(', ')');
        break;
      case '[':
        result << parse_grouping(input, rule_name, '[', ']');
        break;
      default:
        result << parse_pattern_char(input, rule_name);
    }
    c = input.peek();
  } while (c != end);
  result << c;
  input.skip();
  return result.str();
}

// pattern = (grouping | [^{\n])*
// grouping = parens | square
// parens = '(' (grouping | [^)])* ')'
// square = '[' (grouping | [^\]])* ']'
std::string parse_pattern(Input &input, const std::string &rule_name) {
  if (trace)
    std::cout << "\n====\nparse_pattern: " << input.rest() << std::endl;

  char c = input.peek();
  std::stringstream result;
  while (c != '{' && c != '\n' && c != 0) {
    switch (c) {
      case '(':
        result << parse_grouping(input, rule_name, '(', ')');
        break;
      case '[':
        result << parse_grouping(input, rule_name, '[', ']');
        break;
      default:
        result << parse_pattern_char(input, rule_name);
    }
    c = input.peek();
  }
  skip_ws(input);
  return string_trim(result.str());
}

std::string parse_action(Input &input, const std::string &name) {
  if (trace)
    std::cout << "\n====\nparse_action: " << input.rest() << std::endl;

  assert(input.peek() == '{');
  std::stringstream result;
  result << '{';
  char c = input.next();
  while (c != '}') {
    switch (c) {
      case '{':
        result << parse_action(input, name);
        break;
      case 0:
        throw std::runtime_error("Unclosed action in " + name);
      default:
        result << c;
        input.skip();
    }
    c = input.peek();
  }
  result << c;
  input.skip();
  return result.str();
}

// rule = inline? identifier \s* '=' \s* pattern (\s '{' action '}')? new-line
Rule parse_rule(Input &input) {
  if (trace)
    std::cout << "\n====\nparse_rule: " << input.rest() << std::endl;

  assert(is_identifier_start(input.peek()));
  auto name = parse_identifier(input);
  bool is_inline = name == "inline";
  skip_ws(input);
  if (is_inline) {
    name = parse_identifier(input);
  }
  skip_ws(input);
  if (input.peek() != '=') {
    throw std::runtime_error("Expected '=' after name in rule: " + name);
  }
  input.skip();
  skip_ws(input);
  auto pattern = parse_pattern(input, name);
  skip_ws(input);
  std::string action;
  if (input.peek() == '{') {
    action = parse_action(input, name);
  }
  skip_ws(input);
  if (input.peek() != '\n' && input.peek() != 0) {
    throw std::runtime_error(
      "Expected new-line after rule " + name + " but rest is: " + input.rest()
    );
  }
  while (input.peek() == '\n') {
    input.skip();
    skip_ws(input);
  }
  return Rule{name, pattern, action, "", is_inline};
}

std::vector<Rule> parse(Input &input) {
  if (trace)
    std::cout << "\n====\nparse: " << input.rest() << std::endl;

  std::vector<Rule> result;
  while (input.peek() != 0) {
    result.push_back(parse_rule(input));
  }
  return result;
}

void pretty_print(const Rule &rule) {
  std::cout << rule.name << " = " << rule.pattern << " " << rule.action << std::endl;
}

void pretty_print(const std::vector<Rule> &rules) {
  for (auto &rule: rules) {
    pretty_print(rule);
  }
}

void analyze_and_inline(std::vector<Rule> &rules) {
  std::map<std::string, Rule*> nameToRule;
  // Analyze for duplicates and recursion
  for (auto &rule: rules) {
    if (nameToRule.contains(rule.name)) {
      throw std::runtime_error("Duplicate rule: " + rule.name);
    }
    if (rule.pattern.find(rule.name) != std::string::npos) {
      throw std::runtime_error("Recursion detected in rule " + rule.name);
    }
    if (rule.pattern.empty()) {
      throw std::runtime_error("Empty pattern in rule " + rule.name);
    }
    nameToRule[rule.name] = &rule;
  }

  // inline
  for (auto &rule: rules) {
    bool inlined;
    do {
      inlined = false;
      for (const auto &[inlinee_name, inlinee]: nameToRule) {
        if (!inlinee->is_inline) {
          continue;
        }
        auto pos = rule.pattern.find(inlinee_name);
        if (pos != std::string::npos) {
          if (!inlinee->action.empty()) {
            throw std::runtime_error(
              std::format(
                "Cannot inline {} into {}: {} has action {}",
                inlinee_name, rule.name, inlinee_name, inlinee->action
              )
            );
          }
          // remove whitespaces surrounding subrule
          while (pos > 0 && rule.pattern[pos - 1] == ' ') {
            rule.pattern.erase(--pos, 1);
          }
          auto len = inlinee_name.length();
          while (pos + len < rule.pattern.length() && rule.pattern[pos + len] == ' ') {
            rule.pattern.erase(pos + len, 1);
          }
          rule.pattern.replace(pos, len, inlinee->pattern);
          inlined = true;
        }
      }
    } while (inlined);
  }

  // Move action from metarule to subrules and remove its pattern and action.
  for (auto &rule: rules) {
    bool is_metarule = false;
    for (const auto &key: nameToRule | std::views::keys) {
      if (rule.pattern.find(key) != std::string::npos) {
        is_metarule = true;
        break;
      }
    }

    if (is_metarule) {
      auto subrules = string_split_and_trim(rule.pattern, "|");
      for (auto &sub: subrules) {
        if (!nameToRule.contains(sub)) {
          throw std::runtime_error("Cannot find subrule '" + sub + "' in rule " + rule.name);
        }
        auto *subrule = nameToRule[sub];
        if (subrule->is_inline) {
          throw std::runtime_error("Inline rule '" + sub + "' cannot be subrule of " + rule.name);
        }
        subrule->action += "\n" + rule.action;
        subrule->metarule = rule.name;
      }
      rule.pattern = "";
      rule.action = "";
    }
  }
}

std::string prepare_regex(const std::vector<Rule> &rules) {
  std::stringstream result;
  // Replace ( with (?: and make final string with regex
  for (size_t i = 0; i < rules.size(); i++) {
    auto &rule = rules[i];
    // Skip metarules
    if (rule.pattern.empty()) {
      continue;
    }
    // Skip inline rules
    if (rule.is_inline) {
      continue;
    }
    result << "std::regex(R\"(^";
    auto input = Input{rule.pattern, 0};
    while (input.peek() != 0) {
      char c = input.peek();
      switch (c) {
        case '\\':
          result << c;
          result << input.next();
          break;
        case '(':
          result << c;
          if (input.peek(1) != '?') {
            result << "?:";
          }
          break;
        default:
          result << c;
      }
      input.skip();
    }
    result << ")\")";
    if (i + 1 != rules.size()) {
      result << ',';
    }
    result << " // " << rule.name << "\n";
  }
  return result.str();
}

std::string prepare_enum(const std::vector<Rule> &rules) {
  std::stringstream result;
  for (auto &rule: rules) {
    if (!std::isupper(rule.name[0])) {
      continue;
    }
    // Skip inline rules
    if (rule.is_inline) {
      continue;
    }
    result << "k" << rule.name << ",\n";
  }
  result << "kInvalid\n";
  return result.str();
}

std::string prepare_match(const std::vector<Rule> &rules) {
  std::stringstream result;
  bool first = true;
  size_t index = 0;
  for (auto &rule: rules) {
    // Skip metarules
    if (rule.pattern.empty()) {
      continue;
    }
    // Skip inline rules
    if (rule.is_inline) {
      continue;
    }
    if (!first) {
      result << "} else ";
    } else {
      first = false;
    }
    result << "if(std::regex_search(iter, end, match, patterns[" << index++ << "])) { // " << rule.name << "\n";
    //        result << "  std::cout << \"matched " << rule.name << "\\n\";\n";
    if (!string_trim(rule.action).empty()) {
      result << "  const auto &it = match.str(0);\n";
      auto lines = string_split_and_trim(rule.action, "\n");
      for (auto &line: lines) {
        result << "  " << line << "\n";
      }
    }
    result << "  kind = PPToken::k" << (rule.metarule.empty() ? rule.name : rule.metarule) << ";\n";
  }
  result << "}\n";
  return result.str();
}

int main(int argc, char **argv) {
  if (argc < 2)
    throw std::runtime_error("expecting filename as argument");
  auto raw = slurp(argv[1]);
  auto input = Input{raw, 0};
  auto rules = parse(input);
  analyze_and_inline(rules);
  file_dump(string_replace(argv[1], ".lex", ".regex.generated.cpp"), prepare_regex(rules));
  file_dump(string_replace(argv[1], ".lex", ".enum.generated.cpp"), prepare_enum(rules));
  file_dump(string_replace(argv[1], ".lex", ".match.generated.cpp"), prepare_match(rules));
  return 0;
}
