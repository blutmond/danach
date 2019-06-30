PARSER_TOOL=.build/parser-gen
TOKENIZER_TOOL=.build/tokenizer-gen
CPP_SUBSET_TOOL=.build/cpp_subset-gen
UNIFIED_PARSER_TOOL=.build/tmp-parser-gen
CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I src"
LLVM_CXX=`llvm-config-6.0 --cxxflags`
LLVM_CXX="$LLVM_CXX -std=c++17"
BASIC_LLVM_LINK=`llvm-config-6.0 --libs --ldflags `
LLVM_LINK="$BASIC_LLVM_LINK /usr/lib/llvm-6.0/lib/libclang.so -Wl,--start-group -lclangFrontend -lclangSerialization -lclangDriver -lclangCodeGen -lclangSema  -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangEdit -lclangLex -lclangBasic -lclangTooling -Wl,--end-group"
mkdir -p .build/ || exit -1

mkdir -p .build/gen/tokens/ || exit -1
$TOKENIZER_TOOL src/tokens/tokenizer tokenizer_gen > .build/gen/tokens/tokenizer.cc || exit -1
$PARSER_TOOL src/tokens/types > .build/gen/tokens/types.cc || exit -1
$CLANG src/tokens/tool.cc -o .build/tokenizer-gen || exit -1

mkdir -p .build/gen/parser/ || exit -1
$TOKENIZER_TOOL src/parser/tokenizer parser_spec > .build/gen/parser/tokenizer.cc || exit -1
$PARSER_TOOL src/parser/parser > .build/gen/parser/parser.cc || exit -1
$CLANG src/parser/tool.cc -o .build/parser-gen || exit -1

mkdir -p .build/gen/cpp_subset || exit -1
$TOKENIZER_TOOL src/cpp_subset/tokenizer cpp_subset > .build/gen/cpp_subset/tokenizer.cc || exit -1
$PARSER_TOOL src/cpp_subset/parser > .build/gen/cpp_subset/parser.cc || exit -1
$CLANG src/cpp_subset/tool.cc -o .build/cpp_subset-gen || exit -1

mkdir -p .build/gen/rules || exit -1
$TOKENIZER_TOOL src/rules/tokenizer rule_spec > .build/gen/rules/tokenizer.cc || exit -1
$PARSER_TOOL src/rules/parser > .build/gen/rules/parser.cc || exit -1
$CPP_SUBSET_TOOL src/rules/tool > .build/gen/rules/tool.cc || exit -1
$CLANG .build/gen/rules/tool.cc -o .build/rule-apply || exit -1

mkdir -p .build/gen/clang-fe || exit -1
mkdir -p .build/gen/clang-fe/test || exit -1

$CLANG $LLVM_CXX src/clang-fe/CodeGenAugment.cc -c -o .build/gen/clang-fe/CodeGenAugment.o
$CLANG $LLVM_CXX src/clang-fe/RunClang.cc -c -o .build/gen/clang-fe/RunClang.o
$TOKENIZER_TOOL src/clang-fe/tokenizer clang_fe > .build/gen/clang-fe/tokenizer.cc || exit -1
$PARSER_TOOL src/clang-fe/parser > .build/gen/clang-fe/parser.cc || exit -1
$CLANG $LLVM_CXX src/clang-fe/tool.cc .build/gen/clang-fe/RunClang.o .build/gen/clang-fe/CodeGenAugment.o $LLVM_LINK -o .build/clang-fe || exit -1

mkdir -p .build/gen/new_parser/ || exit -1
$TOKENIZER_TOOL src/new_parser/tokenizer parser_spec > .build/gen/new_parser/tokenizer.cc || exit -1
$PARSER_TOOL src/new_parser/parser > .build/gen/new_parser/parser.cc || exit -1
$CLANG src/new_parser/tool.cc -o .build/new-parser-gen || exit -1

mkdir -p .build/gen/tmp_parser/ || exit -1
$TOKENIZER_TOOL src/tmp_parser/tokenizer production_spec > .build/gen/tmp_parser/tokenizer.cc || exit -1
$PARSER_TOOL src/tmp_parser/parser > .build/gen/tmp_parser/parser.cc || exit -1
$CLANG src/tmp_parser/tool.cc -o .build/tmp-parser-gen || exit -1

mkdir -p .build/gen/llvm_graph/ || exit -1
.build/tmp-parser-gen src/llvm_graph/graph_parser src/llvm_graph/tokenizer > .build/gen/llvm_graph/graph_parser.cc || exit -1
.build/tmp-parser-gen src/llvm_graph/spec_parser src/llvm_graph/tokenizer > .build/gen/llvm_graph/spec_parser.cc || exit -1
$CLANG src/llvm_graph/tool.cc -o .build/llvm_graph || exit -1

echo "Success."
