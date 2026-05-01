#include "grammar_common.h"

#include <iostream>
#include <sstream>

extern bool trace;

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
    throw std::runtime_error("Expected '{' to start action in " + name);
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
