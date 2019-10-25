#!/bin/bash
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/rules/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/rules/rule-spec.d -MD -c .generated/gen/rules/rule-spec.cc -o .build/objects/.generated/gen/rules/rule-spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/process.d -MD -c src/rules/process.cc -o .build/objects/src/rules/process.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/tool.d -MD -c src/rules/tool.cc -o .build/objects/src/rules/tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/rules/tool.o .build/objects/src/rules/process.o .build/objects/.generated/gen/rules/rule-spec.o -lstdc++ -o .build/rules-dynamic
/bin/mv .build/rules-dynamic .build/rules
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/rules/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/parser-spec.d -MD -c .generated/gen/parser/parser-spec.cc -o .build/objects/.generated/gen/parser/parser-spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/tokenizer-spec.d -MD -c .generated/gen/parser/tokenizer-spec.cc -o .build/objects/.generated/gen/parser/tokenizer-spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/tool.d -MD -c src/parser/tool.cc -o .build/objects/src/parser/tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/parser/tool.o .build/objects/.generated/gen/parser/tokenizer-spec.o .build/objects/.generated/gen/parser/parser-spec.o -lstdc++ -o .build/parser-dynamic
/bin/mv .build/parser-dynamic .build/parser
/bin/mkdir -p .build/objects/src/rules/
/bin/mkdir -p .generated/gen/rules/
/bin/mkdir -p .build/objects/src/parser/
/bin/mkdir -p .generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/parser/
/bin/mkdir -p .build/objects/.generated/gen/rules/
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/lowering_spec.d -MD -c .generated/gen/parser/lowering_spec.cc -o .build/objects/.generated/gen/parser/lowering_spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/lowering-spec-tool.d -MD -c src/parser/lowering-spec-tool.cc -o .build/objects/src/parser/lowering-spec-tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/parser/lowering-spec-tool.o .build/objects/.generated/gen/parser/lowering_spec.o -lstdc++ -o .build/lowering-spec-tool-dynamic
/bin/mv .build/lowering-spec-tool-dynamic .build/lowering-spec-tool
