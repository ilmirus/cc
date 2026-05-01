#pragma once
#include <cctype>
#include <string>

struct Input {
  std::string raw;
  int offset;
  bool is_line_start;

  Input(const std::string& raw): raw(raw), offset(0), is_line_start(true) {}

  [[nodiscard]] char peek(int i = 0) const {
    if (offset + i >= raw.size())
      return 0;
    return raw[offset + i];
  }

  void skip(int i = 1) {
    for (int ii = 0; ii < i; ii++) {
      if (ii == raw.size()) break;
      if (raw[ii] == '\n') is_line_start = true;
      if (!isspace(raw[ii])) is_line_start = false;
      offset++;
    }
  }

  void skip_ws() {
    while (std::isblank(peek())) {
      skip();
    }
  }

  [[nodiscard]] std::string rest() const {
    return raw.substr(offset);
  }
};

// identifier = [a-zA-Z_][a-zA-Z0-9_]*
bool is_identifier_start(char c);
bool is_identifier(char c);
std::string parse_identifier(Input &input);

// action = { (action|[^}])* }
std::string parse_action(Input &input, const std::string &name);
