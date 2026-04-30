#pragma once
#include <string>

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

void skip_ws(Input &input);

// identifier = [a-zA-Z_][a-zA-Z0-9_]*
bool is_identifier_start(char c);
bool is_identifier(char c);
std::string parse_identifier(Input &input);

// action = { (action|[^}])* }
std::string parse_action(Input &input, const std::string &name);
