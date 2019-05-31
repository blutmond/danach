CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I src"

curl -s file:///tmp/bootstrapping_release.tar.gz | tar -xvz
$CLANG src/tokens/tool.cc -o .build/tokenizer-gen || exit -1
$CLANG src/parser/tool.cc -o .build/parser-gen || exit -1
$CLANG src/cpp_subset/tool.cc -o .build/cpp_subset-gen || exit -1
$CLANG .build/gen/rules/tool.cc -o .build/rule-apply || exit -1
