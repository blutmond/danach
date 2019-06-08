#include "clang-fe/CodeGenAugment.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/Timer.h"
#include "clang/CodeGen/BackendUtil.h"

using namespace clang;
using namespace llvm;

// Hack to get access to the module information...
// Must be kept in sync with the clang sources...
// Luckily it is super easy to keep up to date, and also
// Relatively easy to know if it is totally out of whack (nothing will work).
namespace clang {

  class BackendConsumer : public ASTConsumer {
    using LinkModule = CodeGenAction::LinkModule;

    virtual void anchor();
    DiagnosticsEngine &Diags;
    BackendAction Action;
    const HeaderSearchOptions &HeaderSearchOpts;
    const CodeGenOptions &CodeGenOpts;
    const TargetOptions &TargetOpts;
    const LangOptions &LangOpts;
    std::unique_ptr<raw_pwrite_stream> AsmOutStream;
    ASTContext *Context;

    Timer LLVMIRGeneration;
    unsigned LLVMIRGenerationRefCount;

    /// True if we've finished generating IR. This prevents us from generating
    /// additional LLVM IR after emitting output in HandleTranslationUnit. This
    /// can happen when Clang plugins trigger additional AST deserialization.
    bool IRGenFinished = false;

    std::unique_ptr<CodeGenerator> Gen;

    SmallVector<LinkModule, 4> LinkModules;

    // This is here so that the diagnostic printer knows the module a diagnostic
    // refers to.
    llvm::Module *CurLinkModule = nullptr;

  public:
    BackendConsumer(BackendAction Action, DiagnosticsEngine &Diags,
                    const HeaderSearchOptions &HeaderSearchOpts,
                    const PreprocessorOptions &PPOpts,
                    const CodeGenOptions &CodeGenOpts,
                    const TargetOptions &TargetOpts,
                    const LangOptions &LangOpts, bool TimePasses,
                    const std::string &InFile,
                    SmallVector<LinkModule, 4> LinkModules,
                    std::unique_ptr<raw_pwrite_stream> OS, LLVMContext &C,
                    CoverageSourceInfo *CoverageInfo = nullptr);
    llvm::Module *getModule() const;
    std::unique_ptr<llvm::Module> takeModule();

    CodeGenerator *getCodeGenerator() { return Gen.get(); }

    void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override;

    void Initialize(ASTContext &Ctx) override;

    bool HandleTopLevelDecl(DeclGroupRef D) override;

    void HandleInlineFunctionDefinition(FunctionDecl *D) override;

    void HandleInterestingDecl(DeclGroupRef D) override;

    // Links each entry in LinkModules into our module.  Returns true on error.
    bool LinkInModules();

    void HandleTranslationUnit(ASTContext &C) override;

    void HandleTagDeclDefinition(TagDecl *D) override;

    void HandleTagDeclRequiredDefinition(const TagDecl *D) override;

    void CompleteTentativeDefinition(VarDecl *D) override;

    void AssignInheritanceModel(CXXRecordDecl *RD) override;

    void HandleVTable(CXXRecordDecl *RD) override;

    static void InlineAsmDiagHandler(const llvm::SMDiagnostic &SM,void *Context,
                                     unsigned LocCookie);

    /// Get the best possible source location to represent a diagnostic that
    /// may have associated debug info.
    const FullSourceLoc
    getBestLocationFromDebugLoc(const llvm::DiagnosticInfoWithLocationBase &D,
                                bool &BadDebugInfo, StringRef &Filename,
                                unsigned &Line, unsigned &Column) const;

    void InlineAsmDiagHandler2(const llvm::SMDiagnostic &,
                               SourceLocation LocCookie);

    void DiagnosticHandlerImpl(const llvm::DiagnosticInfo &DI);
    /// \brief Specialized handler for InlineAsm diagnostic.
    /// \return True if the diagnostic has been successfully reported, false
    /// otherwise.
    bool InlineAsmDiagHandler(const llvm::DiagnosticInfoInlineAsm &D);
    /// \brief Specialized handler for StackSize diagnostic.
    /// \return True if the diagnostic has been successfully reported, false
    /// otherwise.
    bool StackSizeDiagHandler(const llvm::DiagnosticInfoStackSize &D);
    /// \brief Specialized handler for unsupported backend feature diagnostic.
    void UnsupportedDiagHandler(const llvm::DiagnosticInfoUnsupported &D);
    /// \brief Specialized handlers for optimization remarks.
    /// Note that these handlers only accept remarks and they always handle
    /// them.
    void EmitOptimizationMessage(const llvm::DiagnosticInfoOptimizationBase &D,
                                 unsigned DiagID);
    void
    OptimizationRemarkHandler(const llvm::DiagnosticInfoOptimizationBase &D);
    void OptimizationRemarkHandler(
        const llvm::OptimizationRemarkAnalysisFPCommute &D);
    void OptimizationRemarkHandler(
        const llvm::OptimizationRemarkAnalysisAliasing &D);
    void OptimizationFailureHandler(
        const llvm::DiagnosticInfoOptimizationFailure &D);
  };

}  // namespace clang

CodeGenWrappingAstConsumer::~CodeGenWrappingAstConsumer() {}

CodeGenerator* CodeGenWrappingAstConsumer::getCodeGenerator() {
 return static_cast<BackendConsumer*>(consumer.get())->getCodeGenerator();
}

bool CodeGenWrappingAstConsumer::HandleTopLevelDecl(DeclGroupRef D) { return consumer->HandleTopLevelDecl(D); }
void  CodeGenWrappingAstConsumer::HandleInterestingDecl(DeclGroupRef D) { consumer->HandleInterestingDecl(D); }
