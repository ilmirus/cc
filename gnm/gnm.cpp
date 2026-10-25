#include <format>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <cstddef>

#include "ast.h"
#include "lex/grammar_common.h"
#include "parse.h"
#include "pretty_print.h"
#include "utils/file_utils.h"

using namespace std::string_literals;

static void check_for_duplicates(Grammar &grammar) {
  std::set<std::string> seen;
  for (auto &rule: grammar.rules) {
    if (seen.contains(rule.name.value))
      throw std::runtime_error("Duplicate rule "s + rule.name.value);
    seen.insert(rule.name.value);
    grammar.symbol_table.emplace(rule.name, &rule);
  }
}

static void color(Rule &, const std::string &, Grammar &);

static void color(Expression *expr, const std::string &new_color, Grammar &grammar) {
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

static void color(Rule &rule, const std::string &initial_color, Grammar &grammar) {
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
    color(alternative, initial_color, grammar);
  } else if (auto *unpack = rule.as_unpack()) {
    auto &[mapping_name, unpacking, _] = *unpack;
    if (auto *resolved = mapping_name.as_rule(grammar)) {
      if (auto *mapping = resolved->as_mapping()) {
        color(*resolved, initial_color, grammar);
        if (unpacking) {
          for (auto &[_, primary]: unpacking.value().bindings) {
            if (auto *subrule = primary.as_rule(grammar)) {
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

static void color(Grammar &grammar) {
  if (grammar.rules.empty())
    return;
  color(grammar.rules[0], grammar.initial_color, grammar);
}

static void prepend_indent(std::stringstream &ss, size_t indent) {
  for (size_t i = 0; i < indent; i++) {
    ss << ' ';
  }
}

static std::string leftpad(size_t indent) {
  std::stringstream ss;
  prepend_indent(ss, indent);
  return ss.str();
}

static const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited, const Grammar &grammar);
static const Rule *find_actionable_rule(const Expression &expr, std::set<const Rule *> &visited, const Grammar &grammar);

static const Rule *find_actionable_rule(const Primary &primary, std::set<const Rule *> &visited, const Grammar &grammar) {
  if (auto *rule = primary.as_rule(grammar)) {
    return find_actionable_rule(*rule, visited, grammar);
  }
  if (auto grouping = primary.as_grouping()) {
    return find_actionable_rule(*grouping, visited, grammar);
  }
  return nullptr;
}

static const Rule *find_actionable_rule(const Expression &expr, std::set<const Rule *> &visited, const Grammar &grammar) {
  for (const auto &seq: expr) {
    for (const auto &primary: seq) {
      if (auto res = find_actionable_rule(primary, visited, grammar))
        return res;
    }
  }
  return nullptr;
}

static const Rule *find_actionable_rule(
  const std::vector<Binding> &bindings, std::set<const Rule *> &visited, const Grammar &grammar
) {
  for (auto &[_, primary]: bindings) {
    if (auto res = find_actionable_rule(primary, visited, grammar))
      return res;
  }
  return nullptr;
}

static const Rule *find_actionable_rule(const Rule &rule, std::set<const Rule *> &visited, const Grammar &grammar) {
  if (visited.contains(&rule))
    return nullptr;
  visited.insert(&rule);
  if (auto *bound = rule.as_bound_expression()) {
    if (!bound->action.empty())
      return &rule;
    return find_actionable_rule(bound->bindings, visited, grammar);
  }
  if (auto *expr = rule.as_alternative()) {
    return find_actionable_rule(*expr, visited, grammar);
  }
  if (auto *unpack = rule.as_unpack()) {
    if (!unpack->action.empty())
      return &rule;
    if (unpack->expr) {
      for (const auto &[_, primary]: unpack->expr->bindings) {
        if (auto *resolved = primary.as_rule(grammar)) {
          if (auto found = find_actionable_rule(*resolved, visited, grammar))
            return found;
        }
      }
    }
  }
  return nullptr;
}

static void generate_alternative_branch(std::ostream &ss, const std::vector<Primary> &seq, size_t indent, const Grammar &grammar) {
  if (seq.size() != 1)
    throw std::logic_error("TODO: implement");

  if (auto *name = seq[0].as_name()) {
    ss << leftpad(indent) << "if (auto result = parse_" << name->value << "(input)";
  } else {
    throw std::logic_error("TODO: implement primary");
  }
  
  ss << ")\n";
  ss << leftpad(indent) << "  return result;\n\n";
}

static void generate_alternative(std::ostream &ss, const Expression &expr, size_t indent, const Grammar &grammar) {
  ss << leftpad(indent) << "auto safepoint = input;\n";
  for (auto &seq : expr) {
    generate_alternative_branch(ss, seq, indent, grammar);
  }
  ss << leftpad(indent) << "input = safepoint;\n";
  ss << leftpad(indent) << "return {};\n";
}

// Every actionable rule - bound_expression and payload_unpack generate the lambda,
// however, bound_expression does not have unpack condition, and payload_unpack may skip bound_expression
// and generate the action right away.
static void generate_bound_expression_common(
  std::ostream &ss, 
  const BoundExpression *bound_expression, const Mapping *mapping,
  size_t indent,
  const std::string& color, const std::string &action
) {
  ss << leftpad(indent) << "auto TYPE_HINTER = [](" << color << " &input) -> decltype(auto) {" << "\n";
  indent += 2;
  if (bound_expression) {
    for (const auto &binding: bound_expression->bindings) {
      if (binding.binding.empty()) {
        // Nothing to do - unbound expressions do not contribute to resulting action type
        continue;
      }
      if (auto *name = binding.primary.as_name()) {
        switch (binding.primary.suffix) {
          case Primary::kZeroOrOne:
            ss << leftpad(indent) << "auto " << binding.binding << " = " << "parse_" << name->value << "(input);\n";
            break;
          case Primary::kNone:
            ss << leftpad(indent) << "auto " << binding.binding << " = *" << "parse_" << name->value << "(input);\n";
            break;
          default:
            throw std::logic_error("TODO: support other primary suffixes");
        }
      } else {
        throw std::logic_error("TODO: support other primary kinds");
      }
    }
  }

  ss << leftpad(indent) << "std::string it;\n";
  ss << leftpad(indent) << "return " << unwrap_action(action) << ";\n";
  indent -= 2;
  ss << leftpad(indent) << "}\n";
  ss << leftpad(indent) << "using RETURN_TYPE = std::invoke_result_t<decltype(TYPE_HINTER), " << color << ")>;\n\n";

  ss << leftpad(indent) << "auto safepoint = input;\n";
  if (mapping) {
    ss << leftpad(indent) << "const auto &token = input.peek();\n";
    ss << leftpad(indent) << "if " << mapping->condition << " {\n";
    indent += 2;
    ss << leftpad(indent) << "auto input = " << mapping->unpack_action << ";\n";
  }

  if (bound_expression) {
    // All the ifs and bindings
    for (const auto &binding: bound_expression->bindings) {
      if (const auto *name = binding.primary.as_name()) {
        switch (binding.primary.suffix) {
          case Primary::kZeroOrOne:
            ss << leftpad(indent);
            if (!binding.binding.empty()) {
              ss << "auto " << binding.binding << " = ";
            }
            ss << "parse_" << name->value << "(input);\n";
            break;
          case Primary::kNone:
            if (!binding.binding.empty()) {
              ss << leftpad(indent) << "if (auto " << binding.binding << "_opt = parse_" << name->value << "(input)) {\n";
              indent += 2;
              ss << leftpad(indent) << "auto " << binding.binding << " = *" << binding.binding << "_opt;\n";
            } else {
              ss << leftpad(indent) << "if (parse_" << name->value << "(input)) {\n";
              indent += 2;
            }
            break;
          default:
            throw std::logic_error("TODO: support other primary suffixes");
        }
      } else if (const auto *number = binding.primary.as_number()) {
        if (!binding.binding.empty()) {
          ss << leftpad(indent) << "std::string " << binding.binding << ";\n";
        }
        ss << leftpad(indent) << "if (input.starts_with(\"" << *number << "\")) {\n";
        indent += 2;
        ss << leftpad(indent) << "input.skip(" << number->size() << ");\n";
        if (!binding.binding.empty()) {
          ss << leftpad(indent) << binding.binding << " = \"" << *number << "\";\n";
        }
        indent -= 2;
        ss << leftpad(indent) << "} else {\n";
        indent += 2;
        ss << leftpad(indent) << "return std::optional<TYPE_HINTER>();\n";
        indent -= 2;
        ss << leftpad(indent) << "}\n";
      } else if (const auto *subexpr = binding.primary.as_grouping()) {
        throw std::logic_error("TODO: Support groupings");
      } else {
        throw std::logic_error("TODO: support other primary kinds");
      }
    }
  }

  if (!action.empty()) {
    ss << leftpad(indent) << "std::string it = safepoint - input;\n";
    ss << leftpad(indent) << "return " << unwrap_action(action) << ";\n";
  } else {
    throw std::logic_error("TODO: support empty action");
  }

  if (bound_expression) {
    // All the closing brackets
    for (auto i = static_cast<std::ptrdiff_t>(bound_expression->bindings.size()) - 1; i >= 0; i--) {
      const auto &binding = bound_expression->bindings[i];
      if (const auto *name = binding.primary.as_name()) {
        switch (binding.primary.suffix) {
          case Primary::kZeroOrOne:
            // nothing to do - pass std::optional to action
            break;
          case Primary::kNone:
            indent -= 2;
            ss << leftpad(indent) << "}\n";
            break;
          default:
            throw std::logic_error("TODO: support other primary suffixes");
        }
      } else if (binding.primary.as_number()) {
        // Nothing to do.
      } else {
        throw std::logic_error("TODO: support other primary kinds");
      }
    }
  }

  if (mapping) {
    indent -= 2;
    ss << leftpad(indent) << "}\n\n";
  }

  ss << leftpad(indent) << "input = safepoint;\n";
  ss << leftpad(indent) << "return std::optional<TYPE_HINTER>();\n";
  indent -= 2;
}

static void generate_bound_expression(std::ostream &ss, const BoundExpression &bound_expression, size_t indent, const std::string &color) {
  generate_bound_expression_common(ss, &bound_expression, nullptr, indent, color, bound_expression.action);
}

static void generate_unpack(
  std::ostream &ss, const PayloadUnpack &unpack, size_t indent, const Grammar &grammar, const Name &rule_name
) {
  if (unpack.action.empty())
    throw std::runtime_error(std::format("payload unpack {} should have action", rule_name.value));

  // Print the mapping as well
  ss << leftpad(indent) << "// " << unpack.mapping_name.value << " = ";
  auto *mapping = grammar.symbol_table.at(unpack.mapping_name)->as_mapping();
  pretty_print(ss, *mapping);
  ss << "\n";

  generate_bound_expression_common(ss, &*unpack.expr, mapping, indent, mapping->payload_type, unpack.action);
}

static void generate_rule(std::ostream &ss, const Rule &rule, const Grammar &grammar) {
  // Function header
  ss << "// ";
  pretty_print(ss, rule, grammar);
  ss << "\n";
  ss << "auto parse_" << rule.name.value << "(" << rule.color << " &input) ";

  std::set<const Rule *> visited;
  if (auto *rule_for_type_deduction = find_actionable_rule(rule, visited, grammar)) {
    if (&rule != rule_for_type_deduction) {
      ss << "-> std::optional<std::invoke_result_t<decltype(parse_" << rule_for_type_deduction->name.value << "), ";
      ss << rule_for_type_deduction->color << ">> ";
    }
  }
  ss << "{\n";

  if (auto *alternative = rule.as_alternative()) {
    generate_alternative(ss, *alternative, 2, grammar);
  } else if (auto *unpack = rule.as_unpack()) {
    generate_unpack(ss, *unpack, 2, grammar, rule.name);
  } else if (auto *bound_expression = rule.as_bound_expression()) {
    generate_bound_expression(ss, *bound_expression, 2, rule.color);
  } else {
    throw std::logic_error("TODO: implement");
  }

  ss << "}\n\n";
}

static void generate(std::stringstream &ss, const Grammar &grammar) {
  for (auto &rule: grammar.rules) {
    generate_rule(ss, rule, grammar);
  }
}

extern bool generate_colors;

int main(int argc, char **argv) {
  if (argc < 2)
    throw std::runtime_error("Expected grammar name as an argument");
  auto file_name = std::string(argv[1]);
  auto raw_grammar = slurp(file_name);
  auto input = Input(raw_grammar);
  auto grammar = parse_grammar(input);

  check_for_duplicates(grammar);
  color(grammar);

  generate_rule(std::cout, grammar.rules[0], grammar);
  generate_rule(std::cout, grammar.rules[1], grammar);
  generate_rule(std::cout, grammar.rules[2], grammar);
  generate_rule(std::cout, grammar.rules[3], grammar);
  return 0;
}
