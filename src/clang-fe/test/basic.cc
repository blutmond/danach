// TODO: wrap into basic.fe as a c++-subset sort of deal.

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

class A {
 public:
};

template <typename T>
using ptr = T*;

void doMain(int argc, char** argv) {
  printf("It works\n");
}

namespace Blah {
void doMain2(int argc, char** argv) {
  printf("Blah::It works\n");
}
}  // namespace

using G = ptr<char>;

class Pete {
 public:
  void G() {}
};

int main2() {
  printf("hmm\n");
  return 0;
}
