#include "rules/process.h"
#include "rules/rule-invoker.h"
#include "rules/compiler.h"

extern bool use_asan;
int main(int argc, char **argv) {
  using namespace rule_spec;
  
  if (argc <= 2) {
    fprintf(stderr, "Not enough arguments: %d.\n", argc);
    exit(-1);
  }

  if (argc == 4 && argv[3] == string_view("--asan")) {
    use_asan = true;
  }

  rules::GlobalContext ctx;
  ctx.GetFile(argv[1])->LinkOrTrigger(argv[2]);
  // TODO: Remove this at some point (These are for linker errors).
  if (argv[2] == string_view("rules-dynamic")) {
    RunTrace({"/bin/mv", ".build/rules-dynamic", ".build/rules"});
    EmitCompilerTrace("tools/bootstrapping.sh");
  }

  fprintf(stderr, "\e[32mSuccess!\e[m\n");
}
