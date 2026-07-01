#pragma once

#include <ostream>

#include "ast.h"

void pretty_print(std::ostream &os, const Grammar &grammar);
void pretty_print(std::ostream &os, const Rule &rule, const Grammar &grammar);
void pretty_print(std::ostream &os, const Name &name, const Grammar &grammar);