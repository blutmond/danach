#!/bin/bash
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/process.d -MD -c src/rules/process.cc -o .build/objects/src/rules/process.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/tool.d -MD -c src/rules/tool.cc -o .build/objects/src/rules/tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/rules/tool.o .build/objects/src/rules/process.o -lstdc++ -o .build/rules-dynamic
/bin/mv .build/rules-dynamic .build/rules
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/tool.d -MD -c src/parser/tool.cc -o .build/objects/src/parser/tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/parser/tool.o -lstdc++ -o .build/parser-dynamic
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/lowering-spec-tool.d -MD -c src/parser/lowering-spec-tool.cc -o .build/objects/src/parser/lowering-spec-tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/parser/lowering-spec-tool.o -lstdc++ -o .build/lowering-spec-tool-dynamic
mv .build/parser-dynamic .build/parser
mv .build/lowering-spec-tool-dynamic .build/lowering-spec-tool
