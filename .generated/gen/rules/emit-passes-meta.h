#pragma once
namespace passes {

struct TopLevelDecl;
struct ContextDef;
struct IndexComponent;
struct TopLevelDecl {
  virtual void EmitStructFwdDeclare(std::ostream& stream);
  virtual void EmitFwdDeclare(std::ostream& stream);
  virtual void EmitDefinitions(std::ostream& stream);
};
struct ContextDef : public TopLevelDecl {
  string_view name;
  ContextDef* context = nullptr;
  std::vector<IndexComponent*> decls;
  void add_decls(IndexComponent* value);
  void EmitStructFwdDeclare(std::ostream& stream) override;
  void EmitFwdDeclare(std::ostream& stream) override;
  void EmitDefinitions(std::ostream& stream) override;
};
struct IndexComponent {
  ContextDef* parent;
  virtual void EmitPublicDecls(std::ostream& stream);
  virtual void EmitPrivateDecls(std::ostream& stream);
  virtual void EmitImpls(std::ostream& stream);
};
}  // namespace passes
