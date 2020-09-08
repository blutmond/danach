#pragma once

#include <typeinfo>
#include <string>

struct TypeMismatchException : public std::exception {
  std::string message;
  TypeMismatchException(const std::type_info& expected_id,
                        const std::type_info& actual_id) {
    message = std::string("expected: ") + std::string(expected_id.name())
        + " got: " + std::string(actual_id.name());
  }
  TypeMismatchException(std::string message) : message(std::move(message)) {}
  const char* what() const throw () {
    return message.c_str();
  }
};
