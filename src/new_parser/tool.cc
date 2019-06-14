#include <string>
#include <experimental/string_view>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

using std::experimental::string_view;

#include "tokens/line_number_helper.cc"
#include "gen/new_parser/tokenizer.cc"
#include "gen/new_parser/parser.cc"
#include "new_parser/regex_nfa_to_dfa.cc"

int main(int argc, char **argv){
  using namespace parser_spec;
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  std::ifstream in(argv[1], std::ios::in | std::ios::binary);
  if (!in) {
    fprintf(stderr, "Could not read file: %s\n", argv[1]);
    exit(-1);
  }
  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();

  Tokenizer tokens(contents.c_str());
  auto* m = parser::DoParse(tokens);
  m = LoweringToNFA().visit(m);
  LoweringToJumpText().visit(m);

  /*
  while (true) {
    auto token = tokens.next();
    if (token.type == tok::eof) break;
    PrintToken(token);
  }
  */
}
