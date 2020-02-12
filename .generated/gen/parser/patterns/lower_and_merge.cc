namespace production_spec {

PatternExpr* getValue(PatternStmt* s);
PatternStmt* findSuccessorForExpr(PatternExpr* expr, PatternStmt* stmt);
PatternStmt* findSuccessor(PatternStmt* s);
PatternStmt* makeTryStmtFromPattern(ModuleContext* globals, PatternDecl* subdecl, TypeDeclExpr* base_type);
Decl* lowerProductionToMergeDecl(Module* m, ModuleContext* globals, Decl* d);
Module* lowerProductionToMerge(ModuleContext* globals, Module* m);

PatternExpr* getValue(PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::Assign: {
auto* s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)s;
return s->value;
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
return s->value;
break;
} case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
return s->value;
break;
} default: {
return nullptr;
}
}
}
}
PatternStmt* findSuccessorForExpr(PatternExpr* expr, PatternStmt* stmt) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::New: {
auto* expr = reinterpret_cast<NewPatternExpr*>(__tmp_switch_name);
(void)expr;
return findSuccessor(expr->value);
break;
} case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
return stmt;
break;
} case PatternExpr::Kind::Pop: {
auto* expr = reinterpret_cast<PopPatternExpr*>(__tmp_switch_name);
(void)expr;
return nullptr;
break;
} case PatternExpr::Kind::CommaConcat: {
auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
return stmt;
break;
} case PatternExpr::Kind::Concat: {
auto* expr = reinterpret_cast<ConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
return stmt;
break;
} case PatternExpr::Kind::Self: {
auto* expr = reinterpret_cast<SelfPatternExpr*>(__tmp_switch_name);
(void)expr;
return stmt;
}
}
}
}
PatternStmt* findSuccessor(PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::String: {
auto* s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)s;
return s;
break;
} case PatternStmt::Kind::Merge: {
auto* s = reinterpret_cast<MergePatternStmt*>(__tmp_switch_name);
(void)s;
return s;
break;
} case PatternStmt::Kind::ExprTailLoop: {
auto* s = reinterpret_cast<ExprTailLoopPatternStmt*>(__tmp_switch_name);
(void)s;
return s;
break;
} case PatternStmt::Kind::Conditional: {
auto* s = reinterpret_cast<ConditionalPatternStmt*>(__tmp_switch_name);
(void)s;
return s;
break;
} case PatternStmt::Kind::Compound: {
auto* s = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)s;
for (auto stmt : s->items) {
auto __tmp__succ = findSuccessor(stmt);
auto succ = std::move(__tmp__succ);
if (succ) {
return succ;
}
}
return nullptr;
break;
} case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
return findSuccessorForExpr(s->value, s);
break;
} case PatternStmt::Kind::Assign: {
auto* s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)s;
return findSuccessorForExpr(s->value, s);
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
return findSuccessorForExpr(s->value, s);
}
}
}
}
PatternStmt* makeTryStmtFromPattern(ModuleContext* globals, PatternDecl* subdecl, TypeDeclExpr* base_type) {
auto __tmp__subt = ({
auto* self = globals->New<ColonTypeDeclExpr>();
self->base = base_type;
self->name = subdecl->name;
self;
});
auto subt = std::move(__tmp__subt);
return ({
auto* self = globals->New<ConditionalPatternStmt>();
self->value = ({
auto* self = globals->New<CompoundPatternStmt>();
self->items.push_back(({
auto* self = globals->New<WrapPatternStmt>();
self->value = ({
auto* self = globals->New<NewPatternExpr>();
self->type = subt;
self->value = subdecl->value;
self;
});
self;
}));
self;
});
self;
});
}
Decl* lowerProductionToMergeDecl(Module* m, ModuleContext* globals, Decl* d) {
{
auto __tmp_switch_name = d;
switch (d->getKind()) {
case Decl::Kind::Define: {
auto* d = reinterpret_cast<DefineDecl*>(__tmp_switch_name);
(void)d;
return ({
auto* self = globals->New<DefineWithTypeDecl>();
self->name = d->name;
auto __tmp__type = ({
auto* self = globals->New<NamedTypeDeclExpr>();
self->name = d->name;
self;
});
auto type = std::move(__tmp__type);
self->type = type;
self->value = ({
auto* self = globals->New<WrapPatternStmt>();
self->value = ({
auto* self = globals->New<NewPatternExpr>();
self->type = type;
self->value = d->value;
self;
});
self;
});
self;
});
break;
} case Decl::Kind::Expr: {
auto* d = reinterpret_cast<ExprDecl*>(__tmp_switch_name);
(void)d;
auto __tmp__base_type = ({
auto* self = globals->New<NamedTypeDeclExpr>();
self->name = d->name;
self;
});
auto base_type = std::move(__tmp__base_type);
auto __tmp__last_group_name = DoExprAnalysis(m, globals, d, base_type);
auto last_group_name = std::move(__tmp__last_group_name);
return ({
auto* self = globals->New<DefineWithTypeDecl>();
self->name = d->name;
self->type = base_type;
self->value = ({
auto* self = globals->New<CompoundPatternStmt>();
self->items.push_back(({
auto* self = globals->New<WrapPatternStmt>();
self->value = ({
auto* self = globals->New<NamedPatternExpr>();
self->name = last_group_name;
self;
});
self;
}));
self;
});
self;
});
break;
} case Decl::Kind::Production: {
auto* d = reinterpret_cast<ProductionDecl*>(__tmp_switch_name);
(void)d;
return ({
auto* self = globals->New<DefineWithTypeDecl>();
self->name = d->name;
auto __tmp__base_type = ({
auto* self = globals->New<NamedTypeDeclExpr>();
self->name = d->name;
self;
});
auto base_type = std::move(__tmp__base_type);
self->type = base_type;
self->value = RotateAndVerifyTrys(globals, ({
auto* self = globals->New<CompoundPatternStmt>();
for (auto subdecl : d->stmts) {
{
auto __tmp_switch_name = subdecl;
switch (subdecl->getKind()) {
case Decl::Kind::Pattern: {
auto* subdecl = reinterpret_cast<PatternDecl*>(__tmp_switch_name);
(void)subdecl;
self->items.push_back(makeTryStmtFromPattern(globals, subdecl, base_type));
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Only patterns allowed in exprs..."
)ASSERT";
exit(-1);
});
}
}
}
}
self;
}));
self;
});
break;
} case Decl::Kind::ProductionAndType: {
auto* d = reinterpret_cast<ProductionAndTypeDecl*>(__tmp_switch_name);
(void)d;
return ({
auto* self = globals->New<DefineWithTypeDecl>();
self->name = d->name;
auto __tmp__base_type = d->type;
auto base_type = std::move(__tmp__base_type);
self->type = base_type;
self->value = RotateAndVerifyTrys(globals, ({
auto* self = globals->New<CompoundPatternStmt>();
for (auto subdecl : d->stmts) {
{
auto __tmp_switch_name = subdecl;
switch (subdecl->getKind()) {
case Decl::Kind::Pattern: {
auto* subdecl = reinterpret_cast<PatternDecl*>(__tmp_switch_name);
(void)subdecl;
self->items.push_back(makeTryStmtFromPattern(globals, subdecl, base_type));
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Only patterns allowed in exprs..."
)ASSERT";
exit(-1);
});
}
}
}
}
self;
}));
self;
});
break;
} case Decl::Kind::Pattern: {
auto* d = reinterpret_cast<PatternDecl*>(__tmp_switch_name);
(void)d;
return ({
auto* self = globals->New<DefineWithTypeDecl>();
self->name = d->name;
self->type = nullptr;
self->value = d->value;
self;
});
break;
} case Decl::Kind::Type: {
auto* d = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)d;
return d;
break;
} case Decl::Kind::DefineWithType: {
auto* d = reinterpret_cast<DefineWithTypeDecl*>(__tmp_switch_name);
(void)d;
return d;
break;
} case Decl::Kind::Entry: {
auto* d = reinterpret_cast<EntryDecl*>(__tmp_switch_name);
(void)d;
return d;
break;
} case Decl::Kind::Tokenizer: {
auto* d = reinterpret_cast<TokenizerDecl*>(__tmp_switch_name);
(void)d;
return d;
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Not allowed at top level", d
)ASSERT";
exit(-1);
});
}
}
}
}
Module* lowerProductionToMerge(ModuleContext* globals, Module* m) {
return ({
auto* self = globals->New<Module>();
self->mod_name = m->mod_name;
for (auto decl : m->decls) {
self->decls.push_back(lowerProductionToMergeDecl(self, globals, decl));
}
self;
});
}

}  // namespace production_spec
