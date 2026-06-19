#include <algorithm>
#include <map>
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

void resolve(Name *name, const Grammar &grammar) {
  for (auto &cursor: grammar.names) {
    if (cursor == *name) {
      name->resolved_to = cursor.resolved_to;
    }
  }
}

void resolve(Expression *expression, const Grammar &grammar) {
  for (auto &sequence: *expression) {
    for (auto &[_, primary]: sequence) {
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

void resolve(const Grammar &grammar) {
  for (const auto &[_, rule]: grammar.names) {
    switch (rule->value.index()) {
      case Rule::kOrExpression:
        resolve(&std::get<OrExpression>(rule->value).expr, grammar);
        break;
      case Rule::kPayloadUnpack: {
        resolve(&std::get<PayloadUnpack>(rule->value).mapping_name, grammar);
        if (auto &expr = std::get<PayloadUnpack>(rule->value).expr; expr.has_value()) {
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

void color(Rule *, const std::string &);

void color(Expression *expr, const std::string &new_color) {
  std::vector<Name *> result;
  for (auto &seq: *expr) {
    for (auto &[_, primary]: seq) {
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

void color(Rule *rule, const std::string &initial_color) {
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
      auto &[mapping_name, unpacking, _] = std::get<PayloadUnpack>(rule->value);
      if (mapping_name.resolved_to != nullptr) {
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

void color(const Grammar &grammar) {
  if (grammar.names.empty())
    return;
  color(grammar.names[0].resolved_to, grammar.initial_color);
}

void prepend_indent(std::stringstream &ss, size_t indent) {
  for (size_t i = 0; i < indent; i++) {
    ss << ' ';
  }
}

const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited);

const Rule *find_actionable_rule(const Expression &expr, std::set<const Rule *> &visited) {
  for (const auto &seq: expr) {
    for (const auto &match: seq) {
      switch (match.primary.value.index()) {
        case Primary::kName: {
          const auto &[_, rule] = std::get<Name>(match.primary.value);
          return find_actionable_rule(*rule, visited);
        }
        case Primary::kGrouping:
          return find_actionable_rule(std::get<Expression>(match.primary.value), visited);
        default:
          // nothing to do
      }
    }
  }
  return nullptr;
}

const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited) {
  if (visited.contains(&rule))
    return nullptr;
  visited.insert(&rule);
  switch (rule.value.index()) {
    case Rule::kOrExpression: {
      auto expr = std::get<OrExpression>(rule.value);
      if (!expr.action.empty())
        return &rule;
      return find_actionable_rule(expr.expr, visited);
    }
    case Rule::kPayloadUnpack: {
      auto unpack = std::get<PayloadUnpack>(rule.value);
      if (!unpack.action.empty())
        return &rule;
      if (unpack.expr.has_value()) {
        return find_actionable_rule(unpack.expr.value(), visited);
      }
    }
    default:
      return nullptr;
  }
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

void generate(std::stringstream &ss, const Rule &rule) {
  ss << "// " << rule << "\n";
  ss << "auto " << rule.name << "(" << rule.color << " &input) {\n";
  switch (rule.value.index()) {
    case Rule::kOrExpression:
      generate(ss, std::get<OrExpression>(rule.value), 2);
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
  if (argc < 2)
    throw std::runtime_error("Expected grammar name as an argument");
  auto name = std::string(argv[1]);
  auto raw_grammar = slurp(name);
  auto input = Input(raw_grammar);
  auto grammar = parse_grammar(input);
  extract_names(grammar);
  std::cout << "Unresolved:\n" << grammar << "\n";
  resolve(grammar);
  color(grammar);
  std::cout << "Resolved:\n" << grammar << "\n";
  return 0;
}
