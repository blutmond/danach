#ifndef _CLANG_FE_RUNCLANG_H_
#define _CLANG_FE_RUNCLANG_H_

#include "clang/Frontend/FrontendAction.h"
#include <string>

int RunClang(clang::ASTFrontendAction* action, int argc, char **argv,
             const std::string& o_file);

#endif  // _CLANG_FE_RUNCLANG_H_
