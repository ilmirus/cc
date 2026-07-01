#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ast.h"
#include "lex/grammar_common.h"
#include "parse.h"
#include "pretty_print.h"
#include "utils/file_utils.h"

using namespace std::string_literals;

void check_for_duplicates(Grammar &grammar) {
  std::set<std::string> seen;
  for (auto &rule: grammar.rules) {
    if (seen.contains(rule.name.value))
      throw std::runtime_error("Duplicate rule "s + rule.name.value);
    seen.insert(rule.name.value);
    grammar.symbol_table.emplace(rule.name, &rule);
  }
}

void color(Rule &, const std::string &, Grammar &);

void color(Expression *expr, const std::string &new_color, Grammar &grammar) {
  std::vector<Name *> result;
  for (auto &seq: *expr) {
    for (auto &primary: seq) {
      if (auto *rule = primary.as_rule(grammar)) {
        color(*rule, new_color, grammar);
      } else if (auto *grouping = primary.as_grouping()) {
        color(grouping, new_color, grammar);
      }
    }
  }
}

void color(Rule &rule, const std::string &initial_color, Grammar &grammar) {
  if (!rule.color.empty()) {
    if (rule.color != initial_color) {
      std::stringstream ss;
      ss << "Rule: ";
      pretty_print(ss, rule, grammar);
      ss << " already has color different to " << initial_color;
      throw std::runtime_error(ss.str());
    }
    return;
  }
  rule.color = initial_color;
  if (auto bound_expr = rule.as_bound_expression()) {
    for (auto &[_, primary]: bound_expr->bindings) {
      if (auto *resolved = primary.as_rule(grammar)) {
        color(*resolved, initial_color, grammar);
      }
    }
  } else if (auto *alternative = rule.as_alternative()) {
    color(&alternative->expr, initial_color, grammar);
  } else if (auto *unpack = rule.as_unpack()) {
    auto &[mapping_name, unpacking, _] = *unpack;
    if (auto *resolved = mapping_name.as_rule(grammar)) {
      if (auto *mapping = resolved->as_mapping()) {
        color(*resolved, initial_color, grammar);
        if (unpacking.has_value()) {
          for (auto &binding: unpacking.value().bindings) {
            if (auto *subrule = binding.primary.as_rule(grammar)) {
              color(*subrule, mapping->payload_type, grammar);
            }
          }
        }
      } else {
        throw std::logic_error(
          "Mapping name "s + mapping_name.value + " referenced in " + resolved->name.value + " does not point to mapping"
        );
      }
    }
  }
}

void color(Grammar &grammar) {
  if (grammar.rules.empty())
    return;
  color(grammar.rules[0], grammar.initial_color, grammar);
}

void prepend_indent(std::stringstream &ss, size_t indent) {
  for (size_t i = 0; i < indent; i++) {
    ss << ' ';
  }
}

const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited, const Grammar &grammar);
const Rule *find_actionable_rule(const Expression &expr, std::set<const Rule *> &visited, const Grammar &grammar);

const Rule *find_actionable_rule(const Primary &primary, std::set<const Rule *> &visited, const Grammar &grammar) {
  if (auto *rule = primary.as_rule(grammar)) {
    return find_actionable_rule(*rule, visited, grammar);
  }
  if (auto grouping = primary.as_grouping()) {
    return find_actionable_rule(*grouping, visited, grammar);
  }
  return nullptr;
}

const Rule *find_actionable_rule(const Expression &expr, std::set<const Rule *> &visited, const Grammar &grammar) {
  for (const auto &seq: expr) {
    for (const auto &primary: seq) {
      if (auto res = find_actionable_rule(primary, visited, grammar))
        return res;
    }
  }
  return nullptr;
}

const Rule *find_actionable_rule(
  const std::vector<Binding> &bindings, std::set<const Rule *> &visited, const Grammar &grammar
) {
  for (auto &[_, primary]: bindings) {
    if (auto res = find_actionable_rule(primary, visited, grammar))
      return res;
  }
  return nullptr;
}

const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited, const Grammar &grammar) {
  if (visited.contains(&rule))
    return nullptr;
  visited.insert(&rule);
  if (auto *bound = rule.as_bound_expression()) {
    if (!bound->action.empty())
      return &rule;
    return find_actionable_rule(bound->bindings, visited, grammar);
  }
  if (auto *expr = rule.as_alternative()) {
    if (!expr->action.empty())
      return &rule;
    return find_actionable_rule(expr->expr, visited, grammar);
  }
  if (auto *unpack = rule.as_unpack()) {
    if (!unpack->action.empty())
      return &rule;
    if (unpack->expr.has_value()) {
      for (auto &binding: unpack->expr.value().bindings) {
        if (auto *resolved = binding.primary.as_rule(grammar)) {
          find_actionable_rule(*resolved, visited, grammar);
        }
      }
    }
  }
  return nullptr;
}

void generate(std::stringstream &ss, const Expression &expr, const std::string &action, size_t indent) {
  prepend_indent(ss, indent);
  ss << "auto safepoint = input;\n";
  for (auto &seq: expr) {
    // generate(ss, seq, )
  }
}

void generate(std::stringstream &ss, const OrExpression &expr, size_t indent) {
  generate(ss, expr.expr, expr.action, indent);
}

void generate(std::stringstream &ss, const Rule &rule, const Grammar &grammar) {
  ss << "// ";
  pretty_print(ss, rule, grammar);
  ss << "\n";
  ss << "auto ";
  pretty_print(ss, rule.name, grammar);
  ss << "(" << rule.color << " &input) {\n";
  if (auto *alternative = rule.as_alternative()) {
    generate(ss, *alternative, 2);
  } else {
    throw std::logic_error("unreachable");
  }
}

void generate(std::stringstream &ss, const Grammar &grammar) {
  for (auto &rule: grammar.rules) {
    generate(ss, rule, grammar);
  }
}

int main(int argc, char **argv) {
  if (argc < 2)
    throw std::runtime_error("Expected grammar name as an argument");
  auto name = std::string(argv[1]);
  auto raw_grammar = slurp(name);
  auto input = Input(raw_grammar);
  auto grammar = parse_grammar(input);
  check_for_duplicates(grammar);
  std::cout << "Unresolved:\n";
  pretty_print(std::cout, grammar);
  std::cout << "\n";
  check_for_duplicates(grammar);
  color(grammar);
  std::cout << "Resolved:\n";
  pretty_print(std::cout, grammar);
  std::cout << "\n";
  return 0;
}
