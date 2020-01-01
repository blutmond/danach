#include "ide-gen/gen-0.h"
#include "rules/rule-invoker.h"

#include <stdio.h>
#include <stdlib.h>

int main()
 {
EmitFromMultiBuffer();

rules::GlobalContext ctx;

auto* dep1 = ctx.GetRule("src/gui", "buffer");
auto* dep2 = ctx.GetRule("src/rules", "rule_invoker");
auto* dep3 = ctx.GetRule("src/gui", "emit_manifest");

auto* obj = SimpleCompileCXXFile({ctx.default_flags, dep1, dep2, dep3},
                                 ".generated/ide-gen",
                                 ".build/objects/ide-gen", {"gen-1.cc", "gen-0.cc"});

LinkCommand cmd;
cmd.deps.push_back(obj);
cmd.output_name = ".build/run-buffer-tests-dynamic";
BuildLinkCommand(&cmd);

RunTrace({"/bin/mv", ".build/run-buffer-tests-dynamic", ".build/run-buffer-tests"});
EmitCompilerTrace("tools/gui-test-bootstrapping.sh");

fprintf(stderr, "\e[32mSuccess!\e[m\n");

}
