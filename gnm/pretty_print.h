#pragma once

#include <ostream>

#include "ast.h"

std::ostream &operator<<(std::ostream &os, const Grammar &grammar);
std::ostream &operator<<(std::ostream &os, const Rule &rule);
std::ostream &operator<<(std::ostream &os, const Name &name);