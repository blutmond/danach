CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I src"
LLVM_CXX=`llvm-config-6.0 --cxxflags`
LLVM_CXX="$LLVM_CXX -std=c++17 -Wno-gnu-statement-expression"
BASIC_LLVM_LINK=`llvm-config-6.0 --libs --ldflags `
LLVM_LINK="$BASIC_LLVM_LINK /usr/lib/llvm-6.0/lib/libclang.so -Wl,--start-group -lclangFrontend -lclangSerialization -lclangDriver -lclangCodeGen -lclangSema  -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangEdit -lclangLex -lclangBasic -lclangTooling -Wl,--end-group"
mkdir -p .build/ || exit -1

mkdir -p .build/gen/parser || exit -1

.build/parser src/parser/tokenizer_spec-parser src/parser/tokenizer_spec-tokenizer > .build/gen/parser/tokenizer-spec.cc || exit -1
.build/parser src/parser/parser_spec-parser src/parser/parser_spec-tokenizer > .build/gen/parser/parser-spec.cc || exit -1
.build/parser src/parser/lowering_spec-parser src/parser/lowering_spec-tokenizer > .build/gen/parser/lowering_spec.cc || exit -1

.build/lowering-spec-tool src/parser/emit_lowering_spec > .build/gen/parser/emit_lowering_spec.cc || exit -1

$CLANG src/parser/lowering-spec-tool.cc -o .build/lowering-spec-tool || exit -1

.build/lowering-spec-tool src/parser/lower_to_nfa > .build/gen/parser/lower_to_nfa.cc || exit -1
.build/lowering-spec-tool src/parser/lower_nfa_to_dfa > .build/gen/parser/lower_nfa_to_dfa.cc || exit -1
.build/lowering-spec-tool src/parser/lower_parser_to_functions > .build/gen/parser/lower_parser_to_functions.cc || exit -1

$CLANG src/parser/tool.cc -o .build/parser || exit -1

mkdir -p .build/gen/clang-fe || exit -1
mkdir -p .build/gen/clang-fe/test || exit -1

$CLANG $LLVM_CXX src/clang-fe/CodeGenAugment.cc -c -o .build/gen/clang-fe/CodeGenAugment.o
$CLANG $LLVM_CXX src/clang-fe/RunClang.cc -c -o .build/gen/clang-fe/RunClang.o
.build/parser src/clang-fe/parser src/clang-fe/tokenizer > .build/gen/clang-fe/clang-fe-parser.cc || exit -1
$CLANG $LLVM_CXX src/clang-fe/tool.cc .build/gen/clang-fe/RunClang.o .build/gen/clang-fe/CodeGenAugment.o $LLVM_LINK -o .build/clang-fe || exit -1

mkdir -p .build/gen/llvm_graph/ || exit -1
.build/parser src/llvm_graph/graph_parser src/llvm_graph/tokenizer > .build/gen/llvm_graph/graph_parser.cc || exit -1
.build/parser src/llvm_graph/spec_parser src/llvm_graph/tokenizer > .build/gen/llvm_graph/spec_parser.cc || exit -1
$CLANG src/llvm_graph/tool.cc -o .build/llvm_graph || exit -1

echo "Success."
