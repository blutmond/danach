namespace production_spec {

void emitTypeExpr(TypeDeclExpr* t);
void emitNewType(TypeDeclExpr* t);
void emitStructBody(ProductTypeDeclExpr* t);
void ImplicitDumpTypes(Module* m);
void productionName(DefineWithTypeDecl* d);
void emitSanitizedPatternExpr(EmitContext* emit_ctx, PatternExpr* expr);
void emitResultIfPresent(EmitContext* emit_ctx);
PatternExpr* getValue(PatternStmt* s);
PatternStmt* findSuccessorForExpr(PatternExpr* expr, PatternStmt* stmt);
PatternStmt* findSuccessor(PatternStmt* s);
void emitConditionalArg(EmitContext* emit_ctx, PatternExpr* expr);
void DebugPrintType(TypeDeclExpr* t);
void DebugPrintExpr(PatternExpr* e);
void DebugPrintStmt(PatternStmt* s);
void emitSanitizedPatternStmt(EmitContext* emit_ctx, PatternStmt* s);
void collectTypeFields(FieldTypeCheckContext* field_tc_ctx, TypeCheckContext* tc_ctx, PatternStmt* stmt);
TypeDeclExpr* doTypeCheckExpr(TypeCheckContext* tc_ctx, PatternExpr* expr);
TypeDeclExpr* doTypeCheck(TypeCheckContext* tc_ctx, PatternStmt* s);
void doModuleTypeCheck(ModuleContext* globals, Module* m);
void emitBasics(ModuleContext* globals, Module* m);
PatternStmt* makeTryStmtFromPattern(PatternDecl* subdecl, TypeDeclExpr* base_type);
Decl* lowerProductionToMergeDecl(Module* m, ModuleContext* globals, Decl* d);
Module* lowerProductionToMerge(ModuleContext* globals, Module* m);
string_view getTokenizerName(Module* m);

void emitTypeExpr(TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Parametric: {
auto* t = reinterpret_cast<ParametricTypeDeclExpr*>(__tmp_switch_name);
(void)t;
emitTypeExpr(t->base);
std::cout << ("<");
auto __tmp__notfirst = false;
auto notfirst = std::move(__tmp__notfirst);
for (auto param : t->params) {
if (notfirst) {
std::cout << (", ");
}
notfirst = true;
emitTypeExpr(param);
}
std::cout << (">");
break;
} case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)t;
if (t->name.str == "Token") {
std::cout << ("tok::Token");
return;
}
if (t->name.str == "Array") {
std::cout << ("std::vector");
return;
}
if (t->name.str == "Map") {
std::cout << ("std::map");
return;
}
if (t->name.str == "String") {
std::cout << ("string_view");
return;
}
if (t->name.str == "char") {
std::cout << ("char");
return;
}
if (t->name.str == "int") {
std::cout << ("int");
return;
}
if (t->name.str == "bool") {
std::cout << ("bool");
return;
}
std::cout << (t->name.str);
std::cout << ("*");
break;
} case TypeDeclExpr::Kind::Colon: {
auto* t = reinterpret_cast<ColonTypeDeclExpr*>(__tmp_switch_name);
(void)t;
auto __tmp__sub_t = t->base;
auto sub_t = std::move(__tmp__sub_t);
{
auto __tmp_switch_name = sub_t;
switch (sub_t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* sub_t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)sub_t;
std::cout << (t->name.str);
std::cout << (sub_t->name.str);
std::cout << ("*");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
void emitNewType(TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)t;
if (t->name.str == "Token") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "Array") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "Map") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "String") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "char") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "int") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "bool") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
std::cout << ("new ");
std::cout << (t->name.str);
break;
} case TypeDeclExpr::Kind::Colon: {
auto* t = reinterpret_cast<ColonTypeDeclExpr*>(__tmp_switch_name);
(void)t;
auto __tmp__sub_t = t->base;
auto sub_t = std::move(__tmp__sub_t);
{
auto __tmp_switch_name = sub_t;
switch (sub_t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* sub_t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)sub_t;
std::cout << ("new ");
std::cout << (t->name.str);
std::cout << (sub_t->name.str);
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
void emitStructBody(ProductTypeDeclExpr* t) {
for (auto subdecl : t->decls) {
std::cout << ("  ");
emitTypeExpr(subdecl->type);
std::cout << (" ");
std::cout << (subdecl->name.str);
std::cout << (";\n");
}
}
void ImplicitDumpTypes(Module* m) {
std::cout << ("namespace ");
std::cout << (m->mod_name.str);
std::cout << ("{\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
std::cout << ("struct ");
std::cout << (decl->name.str);
std::cout << (";\n");
auto __tmp__type = decl->type;
auto type = std::move(__tmp__type);
{
auto __tmp_switch_name = type;
switch (type->getKind()) {
case TypeDeclExpr::Kind::Sum: {
auto* type = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)type;
for (auto subdecl : type->decls) {
std::cout << ("struct ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << (";\n");
}
break;
} default: {
}
}
}
break;
} default: {
}
}
}
}
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__type = decl->type;
auto type = std::move(__tmp__type);
{
auto __tmp_switch_name = type;
switch (type->getKind()) {
case TypeDeclExpr::Kind::Sum: {
auto* type = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)type;
std::cout << ("\nstruct ");
std::cout << (decl->name.str);
std::cout << (" {\n");
std::cout << ("  enum class Kind {\n   ");
for (auto subdecl : type->decls) {
std::cout << (" ");
std::cout << (subdecl->name.str);
std::cout << (",");
}
std::cout << ("\n  };\n");
std::cout << ("  ");
std::cout << (decl->name.str);
std::cout << ("(Kind kind) : kind_(kind) {}\n");
std::cout << (" Kind getKind() { return kind_; }\n");
std::cout << (" private:\n");
std::cout << ("  Kind kind_;\n");
std::cout << ("};\n");
for (auto subdecl : type->decls) {
auto __tmp__subt = subdecl->type;
auto subt = std::move(__tmp__subt);
{
auto __tmp_switch_name = subt;
switch (subt->getKind()) {
case TypeDeclExpr::Kind::Product: {
auto* subt = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)subt;
std::cout << ("\nstruct ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << (": public ");
std::cout << (decl->name.str);
std::cout << (" {\n");
std::cout << ("  ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << ("()");
std::cout << (" : ");
std::cout << (decl->name.str);
std::cout << ("(Kind::");
std::cout << (subdecl->name.str);
std::cout << (") {}\n");
emitStructBody(subt);
std::cout << ("};\n");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
break;
} case TypeDeclExpr::Kind::Product: {
auto* type = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)type;
std::cout << ("\nstruct ");
std::cout << (decl->name.str);
std::cout << (" {\n");
emitStructBody(type);
std::cout << ("};\n");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
}
}
}
}
std::cout << ("}  // namespace ");
std::cout << (m->mod_name.str);
std::cout << ("\n");
}
void productionName(DefineWithTypeDecl* d) {
std::cout << ("_production_");
std::cout << (d->name.str);
}
void emitSanitizedPatternExpr(EmitContext* emit_ctx, PatternExpr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::New: {
auto* expr = reinterpret_cast<NewPatternExpr*>(__tmp_switch_name);
(void)expr;
std::cout << ("({\n");
std::cout << ("auto __current_self = ");
emitNewType(expr->type);
std::cout << (";");
emitSanitizedPatternStmt(emit_ctx, expr->value);
std::cout << ("__current_self;\n");
std::cout << ("})");
break;
} case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
if (emit_ctx->globals->isToken(expr->name.str)) {
std::cout << ("tokens.expect(tok::");
std::cout << (expr->name.str);
std::cout << (")");
} else {
std::cout << ("_production_");
std::cout << (expr->name.str);
std::cout << ("(tokens)");
}
break;
} case PatternExpr::Kind::Pop: {
auto* expr = reinterpret_cast<PopPatternExpr*>(__tmp_switch_name);
(void)expr;
std::cout << ("_tmp_");
std::cout << (emit_ctx->Pop());
break;
} case PatternExpr::Kind::CommaConcat: {
auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__emit_ctx = emit_ctx->ConcatContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
auto __tmp__type = emit_ctx->TypeCheck(expr->element);
auto type = std::move(__tmp__type);
std::cout << ("([&]{\n");
std::cout << ("std::vector<");
emitTypeExpr(type);
std::cout << ("> __current_vector__;\n");
auto __tmp__succ = emit_ctx->GetSuccessor(expr);
auto succ = std::move(__tmp__succ);
if (succ) {
{
auto __tmp_switch_name = succ;
switch (succ->getKind()) {
case PatternStmt::Kind::String: {
auto* succ = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)succ;
std::cout << ("   if (!tokens.peak_check_str(");
std::cout << (succ->value.str);
std::cout << ("))");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: succ, " must be constant string"
)ASSERT";
exit(-1);
});
}
}
}
} else {
std::cout << ("   if (!tokens.peak_check(tok::eof))");
}
std::cout << (" {\n");
std::cout << ("    while (true) {\n");
std::cout << (" __current_vector__.push_back([&]{");
emitSanitizedPatternStmt(emit_ctx, expr->element);
emitResultIfPresent(emit_ctx);
std::cout << (" }());");
std::cout << (" if (tokens.peak_check(tok::");
std::cout << (expr->comma.str);
std::cout << (")) {\n");
std::cout << ("   tokens.expect(tok::");
std::cout << (expr->comma.str);
std::cout << (");\n");
std::cout << (" } else { break; }\n");
std::cout << ("  }}return __current_vector__;\n}())\n");
break;
} case PatternExpr::Kind::Concat: {
auto* expr = reinterpret_cast<ConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__emit_ctx = emit_ctx->ConcatContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
auto __tmp__type = emit_ctx->TypeCheck(expr->element);
auto type = std::move(__tmp__type);
std::cout << ("([&]{\n");
std::cout << ("std::vector<");
emitTypeExpr(type);
std::cout << ("> __current_vector__;\n");
std::cout << ("    while (true) {\n");
auto __tmp__succ = emit_ctx->GetSuccessor(expr);
auto succ = std::move(__tmp__succ);
if (succ) {
{
auto __tmp_switch_name = succ;
switch (succ->getKind()) {
case PatternStmt::Kind::String: {
auto* succ = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)succ;
std::cout << ("   if (tokens.peak_check_str(");
std::cout << (succ->value.str);
std::cout << ("))");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: succ, " must be constant string"
)ASSERT";
exit(-1);
});
}
}
}
} else {
std::cout << ("   if (tokens.peak_check(tok::eof))");
}
std::cout << (" { break; }\n");
std::cout << (" __current_vector__.push_back([&]{");
emitSanitizedPatternStmt(emit_ctx, expr->element);
emitResultIfPresent(emit_ctx);
std::cout << (" }());");
std::cout << ("  }\nreturn __current_vector__;\n}())\n");
break;
} case PatternExpr::Kind::Self: {
auto* expr = reinterpret_cast<SelfPatternExpr*>(__tmp_switch_name);
(void)expr;
std::cout << ("(0 /* unknown expr*/)");
}
}
}
}
void emitResultIfPresent(EmitContext* emit_ctx) {
if (emit_ctx->has_result) {
if (emit_ctx->is_inside_expr) {
std::cout << ("expr_result = result;\ncontinue;\n");
} else {
std::cout << ("return result;\n");
}
} else {
std::cout << ("tokens.unexpected();\n");
}
}
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
void emitConditionalArg(EmitContext* emit_ctx, PatternExpr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
if (emit_ctx->globals->isToken(expr->name.str)) {
std::cout << ("tokens.peak_check(tok::");
std::cout << (expr->name.str);
std::cout << (")");
} else {
({
std::cerr << R"ASSERT(Assert failed: "Cannot handle as peak_expr: ", expr, " // not token"
)ASSERT";
exit(-1);
});
}
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot handle as peak_expr: ", expr
)ASSERT";
exit(-1);
});
}
}
}
}
void DebugPrintType(TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Product: {
auto* t = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)t;
std::cerr << ("Invalid");
break;
} case TypeDeclExpr::Kind::Sum: {
auto* t = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)t;
std::cerr << ("Invalid");
break;
} case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)t;
std::cerr << (t->name.str);
break;
} case TypeDeclExpr::Kind::Colon: {
auto* t = reinterpret_cast<ColonTypeDeclExpr*>(__tmp_switch_name);
(void)t;
DebugPrintType(t->base);
std::cerr << ("::");
std::cerr << (t->name.str);
break;
} case TypeDeclExpr::Kind::Parametric: {
auto* t = reinterpret_cast<ParametricTypeDeclExpr*>(__tmp_switch_name);
(void)t;
DebugPrintType(t->base);
std::cerr << ("<");
auto __tmp__notfirst = false;
auto notfirst = std::move(__tmp__notfirst);
for (auto param : t->params) {
if (notfirst) {
std::cerr << (", ");
}
notfirst = true;
DebugPrintType(param);
}
std::cerr << (">");
}
}
}
}
void DebugPrintExpr(PatternExpr* e) {
{
auto __tmp_switch_name = e;
switch (e->getKind()) {
case PatternExpr::Kind::Named: {
auto* e = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)e;
std::cerr << (e->name.str);
break;
} case PatternExpr::Kind::New: {
auto* e = reinterpret_cast<NewPatternExpr*>(__tmp_switch_name);
(void)e;
std::cerr << ("new ");
DebugPrintType(e->type);
std::cerr << (" ");
DebugPrintStmt(e->value);
break;
} case PatternExpr::Kind::Pop: {
auto* e = reinterpret_cast<PopPatternExpr*>(__tmp_switch_name);
(void)e;
std::cerr << ("pop");
break;
} default: {
std::cerr << ("Unknown");
}
}
}
}
void DebugPrintStmt(PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::String: {
auto* s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << (s->value.str);
break;
} case PatternStmt::Kind::Assign: {
auto* s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("%");
std::cerr << (s->name.str);
std::cerr << (" = ");
DebugPrintExpr(s->value);
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("push ");
DebugPrintExpr(s->value);
break;
} case PatternStmt::Kind::ExprTailLoop: {
auto* s = reinterpret_cast<ExprTailLoopPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("expr_tail_loop(");
DebugPrintExpr(s->base);
std::cerr << (")");
DebugPrintStmt(s->value);
break;
} case PatternStmt::Kind::Conditional: {
auto* s = reinterpret_cast<ConditionalPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("try ");
DebugPrintStmt(s->value);
std::cerr << ("\n");
break;
} case PatternStmt::Kind::Merge: {
auto* s = reinterpret_cast<MergePatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("merge?");
break;
} case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
DebugPrintExpr(s->value);
break;
} case PatternStmt::Kind::Compound: {
auto* s = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)s;
std::cerr << ("{");
std::cerr << ("\n");
for (auto item : s->items) {
DebugPrintStmt(item);
}
std::cerr << ("\n");
std::cerr << ("}");
}
}
}
}
void emitSanitizedPatternStmt(EmitContext* emit_ctx, PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::String: {
auto* s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)s;
std::cout << ("tokens.expect(");
std::cout << (s->value.str);
std::cout << (");\n");
break;
} case PatternStmt::Kind::Assign: {
auto* s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)s;
std::cout << ("__current_self->");
std::cout << (s->name.str);
std::cout << (" = ");
emitSanitizedPatternExpr(emit_ctx, s->value);
std::cout << (";\n");
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__next_id = emit_ctx->Push();
auto next_id = std::move(__tmp__next_id);
std::cout << ("auto _tmp_");
std::cout << (next_id);
std::cout << (" = ");
emitSanitizedPatternExpr(emit_ctx, s->value);
std::cout << (";\n");
break;
} case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
emit_ctx->has_result = true;
std::cout << ("auto result = ");
emitSanitizedPatternExpr(emit_ctx, s->value);
std::cout << (";\n");
break;
} case PatternStmt::Kind::Compound: {
auto* s = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)s;
emit_ctx->RegisterConcatSuccessors(s->items);
for (auto stmt : s->items) {
emitSanitizedPatternStmt(emit_ctx, stmt);
}
break;
} case PatternStmt::Kind::Conditional: {
auto* s = reinterpret_cast<ConditionalPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__emit_ctx = emit_ctx->NewConditionalContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
std::cout << ("if (");
auto __tmp__peak_s = getFirstItem(s->value);
auto peak_s = std::move(__tmp__peak_s);
{
auto __tmp_switch_name = peak_s;
switch (peak_s->getKind()) {
case PatternStmt::Kind::String: {
auto* peak_s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)peak_s;
std::cout << ("tokens.peak_check_str(");
std::cout << (peak_s->value.str);
std::cout << (")");
break;
} case PatternStmt::Kind::Push: {
auto* peak_s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)peak_s;
emitConditionalArg(emit_ctx, peak_s->value);
break;
} case PatternStmt::Kind::Assign: {
auto* peak_s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)peak_s;
emitConditionalArg(emit_ctx, peak_s->value);
break;
} case PatternStmt::Kind::ExprTailLoop: {
auto* peak_s = reinterpret_cast<ExprTailLoopPatternStmt*>(__tmp_switch_name);
(void)peak_s;
({
std::cerr << R"ASSERT(Assert failed: "Cannot handle ExprTailLoop as peak_expr: ", peak_s
)ASSERT";
exit(-1);
});
break;
} case PatternStmt::Kind::Wrap: {
auto* peak_s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)peak_s;
DebugPrintExpr(peak_s->value);
std::cerr << ("\n");
({
std::cerr << R"ASSERT(Assert failed: "Cannot handle Wrap as peak_expr: ", peak_s
)ASSERT";
exit(-1);
});
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot handle as peak_expr: ", peak_s
)ASSERT";
exit(-1);
});
}
}
}
std::cout << (") {\n");
emitSanitizedPatternStmt(emit_ctx, s->value);
emitResultIfPresent(emit_ctx);
std::cout << ("}\n");
break;
} case PatternStmt::Kind::Merge: {
auto* s = reinterpret_cast<MergePatternStmt*>(__tmp_switch_name);
(void)s;
({
std::cerr << R"ASSERT(Assert failed: s, " merge is not supported"
)ASSERT";
exit(-1);
});
break;
} case PatternStmt::Kind::ExprTailLoop: {
auto* s = reinterpret_cast<ExprTailLoopPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__emit_ctx = emit_ctx->NewExprTailLoopContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
emitTypeExpr(s->type);
std::cout << (" expr_result = ");
emitSanitizedPatternExpr(emit_ctx, s->base);
std::cout << (";\n");
std::cout << ("while (true) {\n");
auto __tmp__next_id = emit_ctx->Push();
auto next_id = std::move(__tmp__next_id);
std::cout << ("auto _tmp_");
std::cout << (next_id);
std::cout << (" = expr_result;");
emitSanitizedPatternStmt(emit_ctx, s->value);
std::cout << ("return expr_result;\n");
std::cout << ("}\n");
}
}
}
}
void collectTypeFields(FieldTypeCheckContext* field_tc_ctx, TypeCheckContext* tc_ctx, PatternStmt* stmt) {
{
auto __tmp_switch_name = stmt;
switch (stmt->getKind()) {
case PatternStmt::Kind::Compound: {
auto* stmt = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)stmt;
for (auto cstmt : stmt->items) {
collectTypeFields(field_tc_ctx, tc_ctx, cstmt);
}
break;
} case PatternStmt::Kind::Push: {
auto* stmt = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)stmt;
tc_ctx->Push(doTypeCheckExpr(tc_ctx, stmt->value));
break;
} case PatternStmt::Kind::String: {
auto* stmt = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)stmt;
break;
} case PatternStmt::Kind::Assign: {
auto* stmt = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)stmt;
field_tc_ctx->setField(stmt->name.str, doTypeCheckExpr(tc_ctx, stmt->value));
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "stmt not supported: ", stmt
)ASSERT";
exit(-1);
});
}
}
}
}
TypeDeclExpr* doTypeCheckExpr(TypeCheckContext* tc_ctx, PatternExpr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::CommaConcat: {
auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__globals = tc_ctx->globals;
auto globals = std::move(__tmp__globals);
auto __tmp__tc_ctx = tc_ctx->ConcatContext();
auto tc_ctx = std::move(__tmp__tc_ctx);
auto __tmp__element_ty = globals->CacheType(expr->element, doTypeCheck(tc_ctx, expr->element));
auto element_ty = std::move(__tmp__element_ty);
return ({
auto* self = new ParametricTypeDeclExpr;
self->base = theArrayType;
self->params.push_back(element_ty);
self;
});
break;
} case PatternExpr::Kind::Concat: {
auto* expr = reinterpret_cast<ConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__globals = tc_ctx->globals;
auto globals = std::move(__tmp__globals);
auto __tmp__tc_ctx = tc_ctx->ConcatContext();
auto tc_ctx = std::move(__tmp__tc_ctx);
auto __tmp__element_ty = globals->CacheType(expr->element, doTypeCheck(tc_ctx, expr->element));
auto element_ty = std::move(__tmp__element_ty);
return ({
auto* self = new ParametricTypeDeclExpr;
self->base = theArrayType;
self->params.push_back(element_ty);
self;
});
break;
} case PatternExpr::Kind::Self: {
auto* expr = reinterpret_cast<SelfPatternExpr*>(__tmp_switch_name);
(void)expr;
return tc_ctx->getSelfType();
break;
} case PatternExpr::Kind::New: {
auto* expr = reinterpret_cast<NewPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__field_tc_ctx = tc_ctx->newTypeCtx();
auto field_tc_ctx = std::move(__tmp__field_tc_ctx);
collectTypeFields(field_tc_ctx, tc_ctx, expr->value);
field_tc_ctx->DoVerify(expr->type);
return expr->type;
break;
} case PatternExpr::Kind::Pop: {
auto* expr = reinterpret_cast<PopPatternExpr*>(__tmp_switch_name);
(void)expr;
return tc_ctx->Pop();
break;
} case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
if (tc_ctx->globals->isToken(expr->name.str)) {
return theTokenType;
}
return tc_ctx->globals->getType(expr->name.str);
}
}
}
({
std::cerr << R"ASSERT(Assert failed: "trouble type-checking"
)ASSERT";
exit(-1);
});
}
TypeDeclExpr* doTypeCheck(TypeCheckContext* tc_ctx, PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
return doTypeCheckExpr(tc_ctx, s->value);
break;
} case PatternStmt::Kind::Compound: {
auto* s = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)s;
TypeDeclExpr* type;type = nullptr;
for (auto stmt : s->items) {
auto __tmp__tmp = doTypeCheck(tc_ctx, stmt);
auto tmp = std::move(__tmp__tmp);
if (tmp) {
if (type) {
DebugPrintStmt(s);
({
std::cerr << R"ASSERT(Assert failed: "Compound can only have one type", tmp
)ASSERT";
exit(-1);
});
} else {
type = tmp;
}
}
}
return type;
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
tc_ctx->Push(doTypeCheckExpr(tc_ctx, s->value));
return nullptr;
break;
} case PatternStmt::Kind::Conditional: {
auto* s = reinterpret_cast<ConditionalPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__tc_ctx = tc_ctx->NewConditionalContext();
auto tc_ctx = std::move(__tmp__tc_ctx);
doTypeCheck(tc_ctx, s->value);
return nullptr;
break;
} case PatternStmt::Kind::ExprTailLoop: {
auto* s = reinterpret_cast<ExprTailLoopPatternStmt*>(__tmp_switch_name);
(void)s;
tc_ctx->Push(s->type);
doTypeCheck(tc_ctx, s->value);
return nullptr;
break;
} default: {
return nullptr;
}
}
}
}
void doModuleTypeCheck(ModuleContext* globals, Module* m) {
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::DefineWithType: {
auto* decl = reinterpret_cast<DefineWithTypeDecl*>(__tmp_switch_name);
(void)decl;
globals->RegisterForTypeChecking(decl);
break;
} case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
globals->RegisterType(decl);
break;
} default: {
}
}
}
}
globals->typeCheckAll();
}
void emitBasics(ModuleContext* globals, Module* m) {
std::cout << ("namespace ");
std::cout << (m->mod_name.str);
std::cout << ("{\n");
std::cout << ("namespace parser {\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::DefineWithType: {
auto* decl = reinterpret_cast<DefineWithTypeDecl*>(__tmp_switch_name);
(void)decl;
emitTypeExpr(decl->type);
std::cout << (" ");
productionName(decl);
std::cout << ("(Tokenizer& tokens);\n");
break;
} case Decl::Kind::Entry: {
auto* decl = reinterpret_cast<EntryDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__entry_type = ({
auto* self = new NamedTypeDeclExpr;
self->name = decl->name;
self;
});
auto entry_type = std::move(__tmp__entry_type);
emitTypeExpr(entry_type);
std::cout << (" DoParse(Tokenizer& tokens);\n");
break;
} default: {
}
}
}
}
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::DefineWithType: {
auto* decl = reinterpret_cast<DefineWithTypeDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__emit_ctx = EmitContext::makeRoot(globals);
auto emit_ctx = std::move(__tmp__emit_ctx);
emitTypeExpr(decl->type);
std::cout << (" ");
productionName(decl);
std::cout << ("(Tokenizer& tokens) {\n");
emitSanitizedPatternStmt(emit_ctx, decl->value);
emitResultIfPresent(emit_ctx);
std::cout << ("}\n");
break;
} case Decl::Kind::Entry: {
auto* decl = reinterpret_cast<EntryDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__entry_type = ({
auto* self = new NamedTypeDeclExpr;
self->name = decl->name;
self;
});
auto entry_type = std::move(__tmp__entry_type);
emitTypeExpr(entry_type);
std::cout << (" DoParse(Tokenizer& tokens) {\n");
std::cout << ("  return _production_");
std::cout << (decl->name.str);
std::cout << ("(tokens);\n");
std::cout << ("}\n");
break;
} default: {
}
}
}
}
std::cout << ("}  // namespace parser\n");
std::cout << ("}  // namespace ");
std::cout << (m->mod_name.str);
std::cout << ("\n");
}
PatternStmt* makeTryStmtFromPattern(PatternDecl* subdecl, TypeDeclExpr* base_type) {
auto __tmp__subt = ({
auto* self = new ColonTypeDeclExpr;
self->base = base_type;
self->name = subdecl->name;
self;
});
auto subt = std::move(__tmp__subt);
return ({
auto* self = new ConditionalPatternStmt;
self->value = ({
auto* self = new CompoundPatternStmt;
self->items.push_back(({
auto* self = new WrapPatternStmt;
self->value = ({
auto* self = new NewPatternExpr;
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
auto* self = new DefineWithTypeDecl;
self->name = d->name;
auto __tmp__type = ({
auto* self = new NamedTypeDeclExpr;
self->name = d->name;
self;
});
auto type = std::move(__tmp__type);
self->type = type;
self->value = ({
auto* self = new WrapPatternStmt;
self->value = ({
auto* self = new NewPatternExpr;
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
auto* self = new NamedTypeDeclExpr;
self->name = d->name;
self;
});
auto base_type = std::move(__tmp__base_type);
auto __tmp__last_group_name = DoExprAnalysis(m, globals, d, base_type);
auto last_group_name = std::move(__tmp__last_group_name);
return ({
auto* self = new DefineWithTypeDecl;
self->name = d->name;
self->type = base_type;
self->value = ({
auto* self = new CompoundPatternStmt;
self->items.push_back(({
auto* self = new WrapPatternStmt;
self->value = ({
auto* self = new NamedPatternExpr;
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
auto* self = new DefineWithTypeDecl;
self->name = d->name;
auto __tmp__base_type = ({
auto* self = new NamedTypeDeclExpr;
self->name = d->name;
self;
});
auto base_type = std::move(__tmp__base_type);
self->type = base_type;
self->value = RotateAndVerifyTrys(globals, ({
auto* self = new CompoundPatternStmt;
for (auto subdecl : d->stmts) {
{
auto __tmp_switch_name = subdecl;
switch (subdecl->getKind()) {
case Decl::Kind::Pattern: {
auto* subdecl = reinterpret_cast<PatternDecl*>(__tmp_switch_name);
(void)subdecl;
self->items.push_back(makeTryStmtFromPattern(subdecl, base_type));
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
auto* self = new DefineWithTypeDecl;
self->name = d->name;
auto __tmp__base_type = d->type;
auto base_type = std::move(__tmp__base_type);
self->type = base_type;
self->value = RotateAndVerifyTrys(globals, ({
auto* self = new CompoundPatternStmt;
for (auto subdecl : d->stmts) {
{
auto __tmp_switch_name = subdecl;
switch (subdecl->getKind()) {
case Decl::Kind::Pattern: {
auto* subdecl = reinterpret_cast<PatternDecl*>(__tmp_switch_name);
(void)subdecl;
self->items.push_back(makeTryStmtFromPattern(subdecl, base_type));
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
auto* self = new DefineWithTypeDecl;
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
auto* self = new Module;
self->mod_name = m->mod_name;
for (auto decl : m->decls) {
self->decls.push_back(lowerProductionToMergeDecl(self, globals, decl));
}
self;
});
}
string_view getTokenizerName(Module* m) {
for (auto d : m->decls) {
{
auto __tmp_switch_name = d;
switch (d->getKind()) {
case Decl::Kind::Tokenizer: {
auto* d = reinterpret_cast<TokenizerDecl*>(__tmp_switch_name);
(void)d;
return d->name.str;
break;
} default: {
}
}
}
}
return "";
}

}  // namespace production_spec
