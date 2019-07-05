CLANG="/usr/bin/clang-6.0 -Wall -std=c++17 -lstdc++ -I .build -I .build/src-copy"

curl -s -L https://github.com/blutmond/danach/releases/download/v0.0.1/bootstrapping_release.tar.gz | tar -xvz
$CLANG .build/src-copy/parser/tool.cc -o .build/parser || exit -1
rm -rf .build/src-copy || exit -1
