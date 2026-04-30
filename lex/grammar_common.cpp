#include "grammar_common.h"

#include <iostream>

extern bool trace;

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

  if (!is_identifier_start(input.peek())) {
    throw std::runtime_error("Expected identifier, got: " + input.rest());
  }
  std::string result;
  char c = input.peek();
  while (is_identifier(c)) {
    result += c;
    c = input.next();
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
