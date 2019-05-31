PARSER_TOOL=.build/parser-gen
TOKENIZER_TOOL=.build/tokenizer-gen
CPP_SUBSET_TOOL=.build/cpp_subset-gen
CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I src"
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
