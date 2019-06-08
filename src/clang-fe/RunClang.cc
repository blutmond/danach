#include "clang-fe/RunClang.h"

#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/GlobalDecl.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Lookup.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/CodeGen/BackendUtil.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/TargetSelect.h"

int RunClang(clang::ASTFrontendAction* action, int argc, char **argv,
             const std::string& o_file) {
  using namespace clang::driver;
  using namespace clang;
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  DiagOpts->ShowColors = 1;
  TextDiagnosticPrinter *DiagClient =
    new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);

  Driver TheDriver("cpp_interop", T.str(), Diags);
  TheDriver.setTitle("cpp_interop");
  TheDriver.setCheckInputsExist(false);

  // FIXME: This is a hack copied from the main clang repo...
  SmallVector<const char *, 16> Args(argv + 2, argv + argc);
  Args.insert(Args.begin(), "clang");
  for (auto arg : Args) {
    fprintf(stderr, "Arg: \"%s\"\n", arg);
  }
  Args.push_back("-fsyntax-only");
  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C)
    return 0;

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const driver::JobList &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin())) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    return 1;
  }

  const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags.Report(diag::err_fe_expected_clang_command);
    return 1;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const driver::ArgStringList &CCArgs = Cmd.getArguments();
  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) +
                                       CCArgs.size(),
                                     Diags);

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "clang invocation:\n";
    Jobs.Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  CI->getFrontendOpts().OutputFile = o_file;

  // Create a compiler instance to handle the actual work.
  CompilerInstance Clang;
  Clang.setInvocation(std::move(CI));

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics();
  if (!Clang.hasDiagnostics())
    return 1;

  // Create and execute the frontend to generate an LLVM bitcode module.
  if (!Clang.ExecuteAction(*action))
    return 1;

  return 0;
}
