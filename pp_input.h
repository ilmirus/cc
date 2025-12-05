#pragma once

#include <string>

struct Locus {
  std::string file;
  size_t row, column;
};

// same as std::string, but ignores line splices
struct PPInput {
  std::string raw;

  struct Iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using pointer = char*;
    using reference = char&;

    char *ptr;

    char& operator*() const {
      if (is_line_splice()) return *(ptr + 2);
      return *ptr;
    }

    [[nodiscard]] bool is_line_splice() const {
      return *ptr == '\\' && *(ptr + 1) == '\n';
    }

    Iterator& operator++() {
      if (is_line_splice()) ptr += 3;
      else ++ptr;
      return *this;
    }

    Iterator& operator--() {
      if (*(ptr - 1) == '\n' && *(ptr - 2) == '\\') ptr -= 3;
      else --ptr;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    Iterator operator--(int) {
      Iterator temp = *this;
      --(*this);
      return temp;
    }

    Iterator& operator+=(difference_type diff) {
      if (diff > 0) {
        for (difference_type i = 0; i < diff; ++i) {
          ++(*this);
        }
      } else {
        for (difference_type i = 0; i > diff; --i) {
          --(*this);
        }
      }
      return *this;
    }

    bool operator==(const Iterator &other) const { return ptr == other.ptr; }
    bool operator!=(const Iterator &other) const { return ptr != other.ptr; }
  };

  Iterator begin() { return Iterator(raw.data()); }
  Iterator end() { return Iterator(raw.data() + raw.size()); }
};
