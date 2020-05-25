#pragma once

#include "rules/string-utils.h"
#include <vector>
#include <memory>
#include <string>

namespace clang_util {

struct ErrorMessage {
  enum class Kind {
    Error,
    Notes,
    Fixit,
    BadError,
    UnknownError,
  };
  Kind getKind() const { return kind_; }
  explicit ErrorMessage(Kind kind) : kind_(kind) {}
 private:
  Kind kind_;
};

struct BasicError : public ErrorMessage {
  using ErrorMessage::ErrorMessage;
};
struct UnknownError : public ErrorMessage {
  UnknownError() : ErrorMessage(Kind::UnknownError) {}

  std::string message;
  std::vector<std::string> trailing;
};

std::vector<std::unique_ptr<ErrorMessage>> ParseErrorMessages(string_view errs);

}  // namespace clang_util
