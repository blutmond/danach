#include "parser/lowering_spec_lowering.h"

int main(int argc, char **argv){
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  auto contents = LoadFile(argv[1]);
  lowering_spec::Tokenizer tokens(contents.c_str());
  auto* m = lowering_spec::parser::DoParse(tokens);

  lowering_spec::Emit(std::cout, m);
}
