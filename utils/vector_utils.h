#pragma once
#include <vector>

template<typename T>
void vector_append(std::vector<T>& to, const std::vector<T> from) {
  to.insert(to.end(), from.begin(), from.end());
}
