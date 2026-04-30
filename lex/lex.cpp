#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "grammar_common.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <format>
#include <map>
#include <set>
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
  std::string condition;
  std::string metarule;
  bool is_inline;
};

std::string begin_block;
std::string before_block;
std::string after_block;

bool trace = false;

std::string parse_pattern_char(Input &input, const std::string &rule_name) {
  if (trace)
    std::cout << "\n====\nparse_pattern_char: " << input.rest() << std::endl;

  if (input.peek() == 0) {
    throw std::runtime_error("Unexpected EOF while parsing pattern in " + rule_name);
  }
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

  if (input.peek() != begin) {
    throw std::runtime_error(
      std::format("Expected '{}' while parsing grouping in {}", begin, rule_name)
    );
  }
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

Rule parse_rule_after_name(Input &input, const std::string &name, bool is_inline) {
  std::string condition;

  if (input.peek() == 'i' && input.peek(1) == 'f' && !is_identifier(input.peek(2))) {
    input.skip(2);
    skip_ws(input);
    if (input.peek() != '(') throw std::runtime_error("Expected '(' after 'if' in " + name);
    condition = parse_grouping(input, name, '(', ')');
    skip_ws(input);
  }

  if (input.peek() != '=') throw std::runtime_error("Expected '=' in rule " + name);
  input.skip();
  skip_ws(input);

  auto pattern = parse_pattern(input, name);
  skip_ws(input);

  std::string action;
  if (input.peek() == '{') action = parse_action(input, name);

  return Rule{name, pattern, action, condition, "", is_inline};
}

Rule parse_rule(Input &input) {
  if (!is_identifier_start(input.peek())) {
    throw std::runtime_error("Expected rule name, got: " + input.rest());
  }
  auto name = parse_identifier(input);
  bool is_inline = (name == "inline");
  if (is_inline) {
    skip_ws(input);
    name = parse_identifier(input);
  }
  return parse_rule_after_name(input, name, is_inline);
}

std::vector<Rule> parse(Input &input) {
  if (trace)
    std::cout << "\n====\nparse: " << input.rest() << std::endl;

  std::vector<Rule> result;
  while (input.peek() != 0) {
    skip_ws(input);
    if (input.peek() == '\n') {
      input.skip();
      continue;
    }
    if (input.peek() == 0) break;

    auto name = parse_identifier(input);
    skip_ws(input);

    if (name == "BEGIN" || name == "BEFORE" || name == "AFTER") {
      auto action = parse_action(input, name);
      if (name == "BEGIN") begin_block = action;
      else if (name == "BEFORE") before_block = action;
      else after_block = action;
    } else {
      bool is_inline = (name == "inline");
      if (is_inline) {
        name = parse_identifier(input);
        skip_ws(input);
      }
      result.push_back(parse_rule_after_name(input, name, is_inline));
    }

    skip_ws(input);
    while (input.peek() == '\n') {
      input.skip();
      skip_ws(input);
    }
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

  // Detect inline cycles (including mutual recursion) before expansion.
  std::map<std::string, std::vector<std::string>> inline_deps;
  for (const auto &[name, rule]: nameToRule) {
    if (!rule->is_inline) {
      continue;
    }
    for (const auto &[candidateName, candidateRule]: nameToRule) {
      if (!candidateRule->is_inline) {
        continue;
      }
      if (rule->pattern.find(candidateName) != std::string::npos) {
        inline_deps[name].push_back(candidateName);
      }
    }
  }

  std::set<std::string> inlines_queue;
  std::set<std::string> inlines_visited;
  auto dfs = [&](const auto &self, const std::string &name) -> void {
    if (inlines_queue.contains(name)) {
      throw std::runtime_error("Inline recursion cycle detected at rule " + name);
    }
    if (inlines_visited.contains(name)) {
      return;
    }
    inlines_queue.insert(name);
    if (inline_deps.contains(name)) {
      for (const auto &dep: inline_deps[name]) {
        self(self, dep);
      }
    }
    inlines_queue.erase(name);
    inlines_visited.insert(name);
  };

  for (const auto &[name, _]: inline_deps) {
    dfs(dfs, name);
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

  std::set<std::string> metarules;
  std::map<std::string, std::vector<std::string>> metarule_deps;
  for (auto &rule: rules) {
    bool is_metarule = false;
    for (const auto &key: nameToRule | std::views::keys) {
      if (rule.pattern.find(key) != std::string::npos) {
        is_metarule = true;
        break;
      }
    }
    if (!is_metarule) {
      continue;
    }

    auto subrules = string_split_and_trim(rule.pattern, "|");
    for (auto &sub: subrules) {
      if (!nameToRule.contains(sub)) {
        throw std::runtime_error("Cannot find subrule '" + sub + "' in rule " + rule.name);
      }
      metarule_deps[rule.name].push_back(sub);
    }
    metarules.insert(rule.name);
  }

  std::set<std::string> metarules_queue;
  std::set<std::string> metarules_visited;
  auto dfsMetarules = [&](const auto &self, const std::string &name) -> void {
    if (metarules_queue.contains(name)) {
      throw std::runtime_error("Metarule recursion cycle detected at rule " + name);
    }
    if (metarules_visited.contains(name)) {
      return;
    }
    metarules_queue.insert(name);
    if (metarule_deps.contains(name)) {
      for (const auto &dep: metarule_deps[name]) {
        if (metarules.contains(dep)) {
          self(self, dep);
        }
      }
    }
    metarules_queue.erase(name);
    metarules_visited.insert(name);
  };

  for (const auto &[name, _]: metarule_deps) {
    dfsMetarules(dfsMetarules, name);
  }

  // Move action from metarule to subrules and remove its pattern and action.
  for (auto &rule: rules) {
    if (metarule_deps.contains(rule.name)) {
      for (const auto &sub: metarule_deps[rule.name]) {
        auto *subrule = nameToRule[sub];
        if (subrule->is_inline) {
          throw std::runtime_error("Inline rule '" + sub + "' cannot be subrule of " + rule.name);
        }
        if (subrule->action.empty()) {
          subrule->action = rule.action;
        } else {
          subrule->action += "\n" + rule.action;
        }
        subrule->metarule = rule.name;
      }
      rule.pattern = "";
      rule.action = "";
    }
  }
}

std::string unwrap_action(const std::string &action) {
  std::string s = string_trim(action);
  if (s.starts_with('{') && s.ends_with('}')) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

static std::string choose_raw_string_delimiter(const std::string &content) {
  if (content.find(")\"") == std::string::npos) {
    return "";
  }
  for (size_t i = 0;; ++i) {
    const auto delimiter = std::format("LEX{}", i);
    const auto terminator = std::format("){}\"", delimiter);
    if (content.find(terminator) == std::string::npos) {
      return delimiter;
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
    std::stringstream pattern;
    pattern << '^';
    auto input = Input{rule.pattern, 0};
    while (input.peek() != 0) {
      char c = input.peek();
      switch (c) {
        case '\\':
          pattern << c;
          pattern << input.next();
          break;
        case '(':
          pattern << c;
          if (input.peek(1) != '?') {
            pattern << "?:";
          }
          break;
        default:
          pattern << c;
      }
      input.skip();
    }
    const auto patternContent = pattern.str();
    const auto delimiter = choose_raw_string_delimiter(patternContent);
    if (delimiter.empty()) {
      result << "std::regex(R\"(" << patternContent << ")\")";
    } else {
      result << "std::regex(R\"" << delimiter << "(" << patternContent << ")" << delimiter << "\")";
    }
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

  if (!before_block.empty()) {
    result << unwrap_action(before_block) << "\n";
  }

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

    if (!rule.condition.empty()) {
      result << "if(" << rule.condition << " && std::regex_search(iter, end, match, patterns[" << index++ << "])) {\n";
    } else {
      result << "if(std::regex_search(iter, end, match, patterns[" << index++ << "])) {\n";
    }

    if (!string_trim(rule.action).empty()) {
      result << "  const auto &it = match.str(0);\n";
      auto lines = string_split_lines(rule.action);
      for (auto &line: lines) {
        result << "  " << line << "\n";
      }
    }
    result << "  kind = PPToken::k" << (rule.metarule.empty() ? rule.name : rule.metarule) << ";\n";
  }
  result << "}\n";

  if (!after_block.empty()) {
    result << unwrap_action(after_block) << "\n";
  }

  return result.str();
}

std::string prepare_begin() {
  return unwrap_action(begin_block);
}

int main(int argc, char **argv) {
  if (argc < 2)
    throw std::runtime_error("expecting filename as argument");
  auto raw = slurp(argv[1]);
  auto input = Input{raw, 0};
  auto rules = parse(input);
  analyze_and_inline(rules);
  const auto it = std::find_if(rules.begin(), rules.end(), [](const Rule &rule) {
    return !rule.is_inline && !rule.pattern.empty();
  });
  if (it == rules.end()) {
    throw std::runtime_error("No concrete token rules in lex file: " + std::string(argv[1]));
  }
  file_dump(string_replace(argv[1], ".lex", ".regex.generated.cpp"), prepare_regex(rules));
  file_dump(string_replace(argv[1], ".lex", ".enum.generated.cpp"), prepare_enum(rules));
  file_dump(string_replace(argv[1], ".lex", ".match.generated.cpp"), prepare_match(rules));
  file_dump(string_replace(argv[1], ".lex", ".begin.generated.cpp"), prepare_begin());
  return 0;
}
