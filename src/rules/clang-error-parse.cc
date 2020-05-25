#include "rules/clang-error-parse.h"

#include <iostream>

namespace clang_util {

enum TokenType {

};

struct Token {
  TokenType type;
  string_view data;
};

struct ErrorTokenizer {
 public:
  ErrorTokenizer(string_view item) : original_(item), item_(item) {}

  string_view GetFilename() {
    auto it = item_.find(':');
    if (it == string_view::npos) {
      std::cerr << original_ << std::endl;
      fprintf(stderr, "Cannot find filename\n");
      exit(EXIT_FAILURE);
    }
    auto res = item_.substr(0, it);
    item_.remove_prefix(it);
    return res;
  }

  bool IsTrailing() {
    if (item_.empty() || item_[0] != ' ') return false;
    return true;
  }
  string_view ConsumeTrailingLine() {
    item_.remove_prefix(1);
    return ConsumeLine();
  }

  bool Done() { return item_.empty(); }

  string_view ConsumeLine() {
    auto it = item_.find('\n');
    auto result = item_.substr(0, it);
    item_.remove_prefix(it);
    if (it != string_view::npos) item_.remove_prefix(1);
    return result;
  }

 private:
  string_view original_;
  string_view item_;
};

std::vector<std::unique_ptr<ErrorMessage>> ParseErrorMessages(string_view errs) {
  ErrorTokenizer tokens(errs);

  std::vector<std::unique_ptr<ErrorMessage>> out;
  while (!tokens.Done()) {
    auto* tmp = new UnknownError;
    out.emplace_back(tmp);
    tmp->message = std::string(tokens.ConsumeLine());
    while (!tokens.Done() && tokens.IsTrailing()) {
      tmp->trailing.push_back(std::string(tokens.ConsumeTrailingLine()));
    }
  }
  return out;
}

}  // namespace clang_util
