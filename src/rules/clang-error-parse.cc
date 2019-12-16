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

 private:
  string_view original_;
  string_view item_;
};

struct Item {
};

void ParseErrorMessages(string_view errs) {
  ErrorTokenizer tokens(errs);
  std::cout << tokens.GetFilename() << std::endl;
  // std::cout << errs << std::endl;


  
}

}  // namespace clang_util
