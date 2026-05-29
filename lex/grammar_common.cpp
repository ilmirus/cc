#include "grammar_common.h"

#include <format>
#include <iostream>
#include <sstream>

extern bool trace;

bool Input::starts_with(const std::string &str) {
  auto safepoint = offset;
  for (char c: str) {
    if (peek() != c) {
      offset = safepoint;
      return false;
    }
    skip();
  }
  offset = safepoint;
  return true;
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

  if (!is_identifier_start(input.peek())) {
    throw std::runtime_error("Expected identifier, got: " + input.rest());
  }
  std::string result;
  while (is_identifier(input.peek())) {
    result += input.peek();
    input.skip();
  }
  return result;
}

std::string parse_action(Input &input, const std::string &name) {
  if (trace)
    std::cout << "\n====\nparse_action: " << input.rest() << std::endl;

  if (input.peek() != '{') {
    throw std::runtime_error("Expected '{' to start action in " + name + " " + input.rest());
  }
  std::stringstream result;
  result << '{';
  input.skip();
  char c = input.peek();
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

std::string parse_grouping(Input &input, const std::string &rule_name, char begin, char end) {
  if (trace)
    std::cout << "\n====\nparse_grouping: " << input.rest() << std::endl;

  if (input.peek() != begin) {
    throw std::runtime_error(
      std::format("Expected '{}' while parsing grouping in {}: {}", begin, rule_name, input.rest())
    );
  }
  std::stringstream result;
  result << begin;
  input.skip();
  do {
    switch (input.peek()) {
      case 0:
      case '\n':
        throw std::runtime_error(
          std::format(
            "Unclosed grouping {}..{} in {}: {}",
            begin, end, rule_name, input.rest()
          )
        );
      case '(':
        result << parse_grouping(input, rule_name, '(', ')');
        break;
      case '[':
        result << parse_grouping(input, rule_name, '[', ']');
        break;
      default:
        result << input.peek();
        input.skip();
    }
  } while (input.peek() != end);
  result << end;
  input.skip();
  return result.str();
}