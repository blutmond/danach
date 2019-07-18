CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I .build/src-copy"

curl -s -L https://github.com/blutmond/danach/releases/download/v0.0.2/bootstrapping_release.tar.gz | tar -xz
$CLANG .build/src-copy/parser/tool.cc -o .build/parser || exit -1
$CLANG .build/src-copy/parser/lowering-spec-tool.cc -o .build/lowering-spec-tool || exit -1
rm -rf .build/src-copy || exit -1
