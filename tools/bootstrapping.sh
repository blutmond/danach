#!/bin/bash
/bin/mkdir -p .generated/gen/rules
/bin/mkdir -p .build/objects/.generated/gen/rules
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/rules/rule-spec.d -MD -c .generated/gen/rules/rule-spec.cc -o .build/objects/.generated/gen/rules/rule-spec.o
/bin/mkdir -p .build/objects/src/rules
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/process.d -MD -c src/rules/process.cc -o .build/objects/src/rules/process.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/emit-passes.d -MD -c src/rules/emit-passes.cc -o .build/objects/src/rules/emit-passes.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/rules/interface.d -MD -c .generated/gen/rules/interface.cc -o .build/objects/.generated/gen/rules/interface.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/emit-passes-template.d -MD -c src/rules/emit-passes-template.cc -o .build/objects/src/rules/emit-passes-template.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/string-utils.d -MD -c src/rules/string-utils.cc -o .build/objects/src/rules/string-utils.o
/bin/mkdir -p .generated/gen/parser
/bin/mkdir -p .build/objects/.generated/gen/parser
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/parser-spec.d -MD -c .generated/gen/parser/parser-spec.cc -o .build/objects/.generated/gen/parser/parser-spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/tokenizer-spec.d -MD -c .generated/gen/parser/tokenizer-spec.cc -o .build/objects/.generated/gen/parser/tokenizer-spec.o
/bin/mkdir -p .generated/gen/parser/types
/bin/mkdir -p .build/objects/src/parser/types
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/types/emit_type_decls.d -MD -c src/parser/types/emit_type_decls.cc -o .build/objects/src/parser/types/emit_type_decls.o
/bin/mkdir -p .generated/gen/parser/patterns
/bin/mkdir -p .build/objects/src/parser/patterns
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/patterns/emit_parser.d -MD -c src/parser/patterns/emit_parser.cc -o .build/objects/src/parser/patterns/emit_parser.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/patterns/type_check.d -MD -c src/parser/patterns/type_check.cc -o .build/objects/src/parser/patterns/type_check.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/patterns/lower_and_merge.d -MD -c src/parser/patterns/lower_and_merge.cc -o .build/objects/src/parser/patterns/lower_and_merge.o
/bin/mkdir -p .build/objects/src/parser
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/tokens/regex_nfa_to_dfa.d -MD -c src/parser/tokens/regex_nfa_to_dfa.cc -o .build/objects/src/parser/tokens/regex_nfa_to_dfa.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/tokens/goto_dfa_emitter.d -MD -c src/parser/tokens/goto_dfa_emitter.cc -o .build/objects/src/parser/tokens/goto_dfa_emitter.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/tokenizer_helper.d -MD -c src/parser/tokenizer_helper.cc -o .build/objects/src/parser/tokenizer_helper.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/parser_lowering.d -MD -c src/parser/parser_lowering.cc -o .build/objects/src/parser/parser_lowering.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/.generated/gen/parser/lowering_spec.d -MD -c .generated/gen/parser/lowering_spec.cc -o .build/objects/.generated/gen/parser/lowering_spec.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/parser/lowering_spec_lowering.d -MD -c src/parser/lowering_spec_lowering.cc -o .build/objects/src/parser/lowering_spec_lowering.o
/usr/bin/ccache /usr/bin/clang-6.0 -Wall -std=c++17 -fpic -I .generated/ -I src -I .build/ -MF .build/objects/src/rules/tool.d -MD -c src/rules/tool.cc -o .build/objects/src/rules/tool.o
/usr/bin/clang-6.0 -Wall .build/objects/src/rules/tool.o .build/objects/src/parser/lowering_spec_lowering.o .build/objects/.generated/gen/parser/lowering_spec.o .build/objects/src/parser/parser_lowering.o .build/objects/src/parser/tokenizer_helper.o .build/objects/src/parser/tokens/regex_nfa_to_dfa.o .build/objects/src/parser/tokens/goto_dfa_emitter.o .build/objects/src/parser/patterns/lower_and_merge.o .build/objects/src/parser/patterns/type_check.o .build/objects/src/parser/patterns/emit_parser.o .build/objects/src/parser/types/emit_type_decls.o .build/objects/.generated/gen/parser/tokenizer-spec.o .build/objects/.generated/gen/parser/parser-spec.o .build/objects/src/rules/string-utils.o .build/objects/src/rules/emit-passes-template.o .build/objects/.generated/gen/rules/interface.o .build/objects/src/rules/emit-passes.o .build/objects/src/rules/process.o .build/objects/.generated/gen/rules/rule-spec.o -lstdc++ -o .build/rules-dynamic
/bin/mv .build/rules-dynamic .build/rules
