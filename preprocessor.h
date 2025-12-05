#pragma once

#include <string>
#include <vector>

#include "pp_input.h"

struct PPToken {
  Locus locus;

  enum {
#include "pptoken.enum.generated.cpp"
  } kind;

  std::string value;
};

std::vector<PPToken> pp_scan(const std::string &file, const std::string &input);
