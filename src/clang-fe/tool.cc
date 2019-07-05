#include <string>
#include <experimental/string_view>
#include <iostream>
#include <vector>
#include <fstream>

#include "llvm/ADT/StringRef.h"
using std::experimental::string_view;

#include "parser/line_number_helper.cc"
#include "gen/clang-fe/clang-fe-parser.cc"

#include "clang-fe/CodeGenAugment.h"
#include "clang-fe/RunClang.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "clang/AST/GlobalDecl.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Lookup.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CodeGenAction.h"

struct CompilationPlan {
  std::string filename;
};

llvm::StringRef MakeStringRef(string_view view) {
  return llvm::StringRef(view.data(), view.size());
}

class Type {
 public:
  virtual ~Type() {}

  virtual void dump() = 0;
};

class ErrorType : public Type {
 public:
  void dump() {
    llvm::dbgs() << "Its an error type!\n";
  }
};

class ClangDeclType : public Type {
 public:
  ClangDeclType(clang::Decl* decl) : decl(decl) {}
  clang::Decl* decl;
  void dump() {
    decl->dump();
    llvm::dbgs() << "\n";
  }
};

class QualClangType : public Type {
 public:
  QualClangType(clang::QualType type) : type(type) {}
  clang::QualType type;
  void dump() {
    type.dump();
    llvm::dbgs() << "\n";
  }
};

class LLVMValueType : public Type {
 public:
  LLVMValueType(llvm::Value* value) : value(value) {}
  llvm::Value* value;
  void dump() {
    value->print(llvm::dbgs());
    llvm::dbgs() << "\n";
  }
};

struct LoweringContext {
  clang::CompilerInstance &CI;
  clang::CodeGenerator* Gen;
  clang::ASTContext& C;

  LoweringContext(clang::CompilerInstance &CI, clang::CodeGenerator* Gen, clang::ASTContext &C)
      : CI(CI), Gen(Gen), C(C) {}

  clang_fe::Module* doParse(llvm::StringRef filename) {
    auto& SM = CI.getSourceManager();
    auto& FM = SM.getFileManager();

    auto* file = FM.getFile(filename, true);
    auto FID = SM.createFileID(file, clang::SourceLocation(), clang::SrcMgr::C_User);

    auto* buffer = SM.getMemoryBufferForFile(file);

    start_sl = SM.getLocForStartOfFile(FID);

    using namespace clang_fe;
    const char* stBuffer = buffer->getBufferStart();
    Tokenizer tokens(stBuffer);
    return parser::DoParse(tokens);
  }

  llvm::Module* module = Gen->GetModule();
  llvm::LLVMContext& Ctx = module->getContext();

  clang::Sema& sema = CI.getSema();

  struct PreResolvedDecl {
    clang_fe::Decl* decl = nullptr;
    Type* type = nullptr;
  };

  ErrorType* errorTy = new ErrorType;
  std::map<string_view, PreResolvedDecl> decl_resolver;
  class BaseClangType : public Type {
   public:
    BaseClangType(LoweringContext* ctx) : ctx(ctx) {}

    void dump() {
      llvm::dbgs() << "It is the \"Clang\" Type\n";
    }
    LoweringContext* ctx;
  };
  BaseClangType* clangTy = new BaseClangType(this);

  const char* stBuffer = nullptr;
  clang::SourceLocation start_sl;

  uint64_t DIAG_DuplicateTypeId = CI.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Error, "Dupliciate type: '%0'");
  uint64_t DIAG_UnknownTypeId = CI.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Error, "Unknown type: '%0'");
  uint64_t DIAG_NotTypeId = CI.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Error, "Not a type: '%0'");
  uint64_t DIAG_CouldNotFindId = CI.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Error, "Could not find: '%0'");

  Type* DoResolveTypeRef(clang_fe::Decl* decl) {
    using namespace clang_fe;
    switch (decl->getKind()) {
    case Decl::Kind::Func: {
      auto tok = reinterpret_cast<FuncDecl*>(decl)->name;
      tokDiagnostic(DIAG_NotTypeId, tok);
      return errorTy;
    } case Decl::Kind::Alias: {
      auto* alias = reinterpret_cast<AliasDecl*>(decl);
      return visit(alias->type);
    } case Decl::Kind::DebugPrint: {
      llvm_unreachable("not reachable");
    }
    }
  }
  Type* ResolveTypeRef(PreResolvedDecl& decl) {
    if (decl.type) return decl.type;
    return decl.type = DoResolveTypeRef(decl.decl);
  }

  Type* GetMember(Type* base, clang_fe::tok::Token tok) {
    if (base == clangTy) {
      clang::DeclarationName DName = &sema.Context.Idents.get(MakeStringRef(tok.str));
      clang::LookupResult R(sema, DName, GetSourceLocation(tok), clang::Sema::LookupAnyName);
      sema.LookupQualifiedName(R, C.getTranslationUnitDecl());
      if (R.asUnresolvedSet().size() != 1) {
        tokDiagnostic(DIAG_CouldNotFindId, tok);
        return errorTy;
      }
      R.resolveKind();
      auto* decl = R.getFoundDecl();
      return new ClangDeclType(decl);
    } else if (auto* declTy = dynamic_cast<ClangDeclType*>(base)) {
      if (llvm::isa<clang::NamespaceDecl>(declTy->decl)) {
        clang::DeclarationName DName = &sema.Context.Idents.get(MakeStringRef(tok.str));
        clang::LookupResult R(sema, DName, GetSourceLocation(tok), clang::Sema::LookupAnyName);
        sema.LookupQualifiedName(R, llvm::cast<clang::NamespaceDecl>(declTy->decl));
        if (R.asUnresolvedSet().size() != 1) {
          tokDiagnostic(DIAG_CouldNotFindId, tok);
          return errorTy;
        }
        R.resolveKind();
        auto* decl = R.getFoundDecl();
        return new ClangDeclType(decl);
      }
      llvm::dbgs() << "Implement all of this!! {\n";
      declTy->decl->dump();
      llvm::dbgs() << "}  // Implement all of this!!\n";
    }
    return errorTy;
  }

  Type* visit(clang_fe::TypeRef* decl) {
    using namespace clang_fe;
    switch (decl->getKind()) {
    case TypeRef::Kind::Named: {
      auto tok = reinterpret_cast<NamedTypeRef*>(decl)->name;
      if (tok.str == "clang") {
        return clangTy;
      }
      auto it = decl_resolver.find(tok.str);
      if (it == decl_resolver.end()) {
        tokDiagnostic(DIAG_UnknownTypeId, tok);
        return errorTy;
      }
      return ResolveTypeRef(it->second);
    } case TypeRef::Kind::Template: {
      auto tdecl = reinterpret_cast<TemplateTypeRef*>(decl);
      clang::TemplateArgumentListInfo args;
      for (auto* arg : tdecl->args) {
        {
         if (auto* ty = dynamic_cast<QualClangType*>(visit(arg))) {
           auto clangTy = ty->type;
           args.addArgument(clang::TemplateArgumentLoc(clang::TemplateArgument(clangTy), C.getTrivialTypeSourceInfo(clangTy, clang::SourceLocation())));
           continue;
         }
        }
        auto* ty = dynamic_cast<ClangDeclType*>(visit(arg));
        if (ty && llvm::isa<clang::TypeDecl>(ty->decl)) {
          clang::TypeDecl* mdecl = llvm::cast<clang::TypeDecl>(ty->decl);
          auto clangTy = C.getTypeDeclType(mdecl);
          args.addArgument(clang::TemplateArgumentLoc(clang::TemplateArgument(clangTy), C.getTrivialTypeSourceInfo(clangTy, clang::SourceLocation())));
        } else {
          llvm::dbgs() << "error!\n";
          return errorTy;
        }
      }
      auto* base = visit(tdecl->base);
      auto* ty = dynamic_cast<ClangDeclType*>(base);
      if (ty) {
        if (llvm::isa<clang::TemplateDecl>(ty->decl)) {
          auto declty = llvm::cast<clang::TemplateDecl>(ty->decl);
          auto templ = sema.CheckTemplateIdType(clang::TemplateName(declty), clang::SourceLocation(), args);
          return new QualClangType(templ);
        }
        llvm::dbgs() << "FAIL: \n";
        ty->decl->dump(llvm::dbgs());
        llvm::dbgs() << "} =====\n";
      }
      std::cout << "Problem unknown Template\n";
      return errorTy;
    } case TypeRef::Kind::Member: {
      auto* mdecl = reinterpret_cast<MemberTypeRef*>(decl);
      auto* base = visit(mdecl->base);
      return GetMember(base, mdecl->name);
    } case TypeRef::Kind::Void: {
      std::cout << "Problem unknown Void\n";
      return errorTy;
    }
    }
  }

  llvm::Type* GetLLVMType(Type* ty_) {
    if (auto* ty = dynamic_cast<QualClangType*>(ty_)) {
      return clang::CodeGen::convertTypeForMemory(Gen->CGM(), ty->type);
    }
    if (auto* ty = dynamic_cast<ClangDeclType*>(ty_)) {
      if (llvm::isa<clang::TypeDecl>(ty->decl)) {
        return clang::CodeGen::convertTypeForMemory(Gen->CGM(), C.getTypeDeclType(llvm::cast<clang::TypeDecl>(ty->decl)));
      }
    }
    return nullptr;
  }

  struct FnLoweringContext {
    FnLoweringContext(llvm::LLVMContext& Ctx, llvm::Function *func) : builder(Ctx), func(func) {

    }
    Type* lookup(clang_fe::tok::Token tok) {
      auto it = local_vars.find(tok.str);
      if (it == local_vars.end()) return nullptr;
      return it->second;
    }
    
    std::map<string_view, Type*> local_vars;
    llvm::IRBuilder<> builder;
    llvm::Function *func;
  };

  Type* visitExpr(FnLoweringContext& lowering, clang_fe::Expr* expr) {
    using namespace clang_fe;
    auto& builder = lowering.builder;
    switch (expr->getKind()) {
    case Expr::Kind::Named: {
      auto tok = reinterpret_cast<NamedExpr*>(expr)->name;
      if (auto* ptr = lowering.lookup(tok)) {
        return ptr;
      }
      if (tok.str == "clang") {
        return clangTy;
      }
      auto it = decl_resolver.find(tok.str);
      if (it == decl_resolver.end()) {
        tokDiagnostic(DIAG_UnknownTypeId, tok);
        return errorTy;
      }
      return ResolveTypeRef(it->second);
    } case Expr::Kind::ColonColon: {
      auto* mdecl = reinterpret_cast<ColonColonExpr*>(expr);
      auto* base = visitExpr(lowering, mdecl->base);
      return GetMember(base, mdecl->name);
    } case Expr::Kind::Call: {
      auto* mdecl = reinterpret_cast<CallExpr*>(expr);
      auto* base = visitExprAsLLVM(lowering, mdecl->base);
      std::vector<llvm::Value*> args;
      for (auto* arg : mdecl->args) {
        args.push_back(visitExprAsLLVM(lowering, arg));
      }
      return new LLVMValueType(builder.CreateCall(base, args));
    }
    default:
      llvm::dbgs() << "Unknown expr!\n";
    }
    return errorTy;
  }

  llvm::Value* visitExprAsLLVM(FnLoweringContext& lowering, clang_fe::Expr* expr) {
    auto* tmpv = visitExpr(lowering, expr);
    if (auto* v = dynamic_cast<LLVMValueType*>(tmpv)) {
      return v->value;
    }
    if (auto* v = dynamic_cast<ClangDeclType*>(tmpv)) {
      if (llvm::isa<clang::FunctionDecl>(v->decl)) {
        clang::GlobalDecl gdecl(llvm::cast<clang::FunctionDecl>(v->decl));
        return Gen->GetAddrOfGlobal(gdecl, false);
      }
    }
    fprintf(stderr, "Problem!\n");
    exit(-1);
  }

  void LowerStmt(FnLoweringContext& lowering, clang_fe::Stmt* stmt) {
    using namespace clang_fe;
    auto& builder = lowering.builder;
    switch (stmt->getKind()) {
    case Stmt::Kind::Compound:
      for (auto* substmt : reinterpret_cast<CompoundStmt*>(stmt)->stmts) {
        LowerStmt(lowering, substmt);
      }
      break;
    case Stmt::Kind::Discard: {
      auto* expr = visitExprAsLLVM(lowering, reinterpret_cast<DiscardStmt*>(stmt)->expr);
      break;
    } case Stmt::Kind::Return:
      if (auto* value = visitExprAsLLVM(lowering, reinterpret_cast<ReturnStmt*>(stmt)->expr)) {
        builder.CreateRet(value);
      }
      break;
    }
  }

  void visit(clang_fe::FuncDecl* decl) {
    std::vector<llvm::Type*> args;
    for (auto* sdecl : decl->args) {
      if (auto* ty = GetLLVMType(visit(sdecl->type))) {
        args.push_back(ty);
      }
    }
    llvm::FunctionType *funcType = 
        llvm::FunctionType::get(llvm::Type::getInt32Ty(Ctx), args, false);
    llvm::Function *mainFunc = 
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, MakeStringRef(decl->name.str), module);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Ctx, "entry", mainFunc);

    FnLoweringContext lowering(Ctx, mainFunc);
    lowering.builder.SetInsertPoint(BB);

    int i = 0;
    auto arg_it = mainFunc->arg_begin();
    for (auto* sdecl : decl->args) {
      lowering.local_vars[sdecl->name.str] = new LLVMValueType(arg_it);
      ++arg_it;
      ++i;
    }
  
    LowerStmt(lowering, decl->body);
  }

  void visit(clang_fe::Module* m) {
    using namespace clang_fe;
    for (auto* decl : m->decls) {
      switch (decl->getKind()) {
      case Decl::Kind::Func: {
        auto tok = reinterpret_cast<FuncDecl*>(decl)->name;
        setDecl(tok, decl_resolver[tok.str].decl, decl);
        break;
      } case Decl::Kind::Alias: {
        auto tok = reinterpret_cast<FuncDecl*>(decl)->name;
        setDecl(tok, decl_resolver[tok.str].decl, decl);
        break;
      } case Decl::Kind::DebugPrint:
        break;
      }
    }

    for (auto* decl : m->decls) {
      switch (decl->getKind()) {
      case Decl::Kind::Func: {
        auto* func = reinterpret_cast<FuncDecl*>(decl);
        visit(func);
        break;
      } case Decl::Kind::Alias: {
        auto tok = reinterpret_cast<AliasDecl*>(decl)->name;
        ResolveTypeRef(decl_resolver[tok.str]);
        break;
      } case Decl::Kind::DebugPrint:
        llvm::dbgs() << "Compiler debug print statement!!\n";
        auto* ty = visit(reinterpret_cast<DebugPrintDecl*>(decl)->type);
        ty->dump();
        break;
      }
    }

//    module->print(llvm::dbgs(), nullptr);
    if (llvm::verifyModule(*module, &llvm::dbgs())) {
      fprintf(stderr, "problem!\n");
      exit(-1);
    }
  }

  clang::SourceLocation GetSourceLocation(clang_fe::tok::Token tok) {
    return start_sl.getLocWithOffset(tok.str.data() - stBuffer);
  }

  void tokDiagnostic(unsigned DiagID, clang_fe::tok::Token tok) {
    auto sl = GetSourceLocation(tok);
    CI.getDiagnostics().Report(sl, DiagID).AddString(MakeStringRef(tok.str));
  }

  void setDecl(clang_fe::tok::Token tok, clang_fe::Decl*& stdecl, clang_fe::Decl* decl) {
    if (stdecl) {
      tokDiagnostic(DIAG_DuplicateTypeId, tok);
    } else {
      stdecl = decl;
    }
  }
};

class ClangFeAstConsumer : public clang::CodeGenWrappingAstConsumer {
 public:
  CompilationPlan* plan;
  clang::CompilerInstance &CI;
 public:
  ClangFeAstConsumer(clang::CompilerInstance &CI, CompilationPlan* plan, std::unique_ptr<clang::ASTConsumer> consumer)
      : CodeGenWrappingAstConsumer(std::move(consumer)), plan(plan), CI(CI) {}
  ~ClangFeAstConsumer() override {}

  void HandleTranslationUnit(clang::ASTContext &C) override {
    LoweringContext ctx(CI, getCodeGenerator(), C);
    auto* m = ctx.doParse(plan->filename);
    ctx.visit(m);
    consumer->HandleTranslationUnit(C);
  }
};

using Action = clang::EmitObjAction;
class ClangFeCodeGenAction : public Action {
 public:
  ClangFeCodeGenAction(llvm::LLVMContext* ctx, CompilationPlan* plan) : Action(ctx), plan_(plan) {}

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                 llvm::StringRef InFile) override {
    return llvm::make_unique<ClangFeAstConsumer>(CI, plan_, Action::CreateASTConsumer(CI, InFile));
  }
 private:
  CompilationPlan* plan_;
};

// Run with:
// .build/clang-fe src/clang-fe/test/basic.fe src/clang-fe/test/basic.cc
// clang-6.0 .build/gen/clang-fe/test/basic.o && ./a.out
int main(int argc, char **argv){
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  CompilationPlan plan;
  plan.filename = argv[1];

  llvm::LLVMContext context;
  ClangFeCodeGenAction action(&context, &plan);
  return RunClang(&action, argc, argv, ".build/gen/clang-fe/test/basic.o");

  /*
  while (true) {
    auto token = tokens.next();
    if (token.type == tok::eof) break;
    PrintToken(token);
  }
  */
}
