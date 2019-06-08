#ifndef _CLANG_FE_CODEGENAUGMENT_H_
#define _CLANG_FE_CODEGENAUGMENT_H_

#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>

namespace clang {

// Hacks to make up for the fact that you can't get a CodeGenerator from an EmitLLVMOnlyAction.
class CodeGenWrappingAstConsumer : public clang::ASTConsumer {
 public:
  std::unique_ptr<clang::ASTConsumer> consumer;

 public:
  CodeGenWrappingAstConsumer(std::unique_ptr<clang::ASTConsumer> consumer) : consumer(std::move(consumer)) {}
  ~CodeGenWrappingAstConsumer() override;

  void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override { consumer->HandleCXXStaticMemberVarInstantiation(VD); }
  void Initialize(ASTContext &Ctx) override { consumer->Initialize(Ctx); }
  bool HandleTopLevelDecl(DeclGroupRef D) override;
  void HandleInlineFunctionDefinition(FunctionDecl *D) override { consumer->HandleInlineFunctionDefinition(D); }
  void HandleInterestingDecl(DeclGroupRef D) override;
  void HandleTranslationUnit(ASTContext &C) override { consumer->HandleTranslationUnit(C); }
  void HandleTagDeclDefinition(TagDecl *D) override { consumer->HandleTagDeclDefinition(D); }
  void HandleTagDeclRequiredDefinition(const TagDecl *D) override { consumer->HandleTagDeclRequiredDefinition(D); }
  void CompleteTentativeDefinition(VarDecl *D) override { consumer->CompleteTentativeDefinition(D); }
  void AssignInheritanceModel(CXXRecordDecl *RD) override { consumer->AssignInheritanceModel(RD); }
  void HandleVTable(CXXRecordDecl *RD) override { consumer->HandleVTable(RD); }

  CodeGenerator* getCodeGenerator();
};

}  // namespace clang

#endif  // _CLANG_FE_CODEGENAUGMENT_H_
