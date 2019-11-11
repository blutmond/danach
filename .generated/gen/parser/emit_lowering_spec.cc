namespace lowering_spec {

void EmitType(std::ostream& stream, TypeRef* t);
void EmitTypeSignature(std::ostream& stream, TypeRef* t);
void EmitExpr(std::ostream& stream, ContextFinderContext* ctx, Expr* expr);
void EmitStmt(std::ostream& stream, ContextFinderContext* ctx, Stmt* stmt);
void EmitFuncDeclHeader(std::ostream& stream, FuncDecl* decl);
void EmitFuncDecl(std::ostream& stream, ContextFinderContext* ctx, FuncDecl* decl);
void Emit(std::ostream& stream, Module* m);

void EmitType(std::ostream& stream, TypeRef* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeRef::Kind::Void: {
auto* t = reinterpret_cast<VoidTypeRef*>(__tmp_switch_name);
(void)t;
GetStream(stream) << ("void");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(stream, t->base);
GetStream(stream) << ("::");
GetStream(stream) << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
(void)t;
GetStream(stream) << (t->name.str);
break;
} default: {
GetStream(stream) << ("unknown");
}
}
}
}
void EmitTypeSignature(std::ostream& stream, TypeRef* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeRef::Kind::Void: {
auto* t = reinterpret_cast<VoidTypeRef*>(__tmp_switch_name);
(void)t;
GetStream(stream) << ("void");
break;
} case TypeRef::Kind::Template: {
auto* t = reinterpret_cast<TemplateTypeRef*>(__tmp_switch_name);
(void)t;
EmitTypeSignature(stream, t->base);
GetStream(stream) << ("<");
int i;i = 0;
for (auto arg : t->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitTypeSignature(stream, arg);
}
GetStream(stream) << (">");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(stream, t->base);
GetStream(stream) << ("::");
GetStream(stream) << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
(void)t;
if (t->name.str == "Token") {
GetStream(stream) << ("tok::Token");
return;
}
if (t->name.str == "Array") {
GetStream(stream) << ("std::vector");
return;
}
if (t->name.str == "Map") {
GetStream(stream) << ("std::map");
return;
}
if (t->name.str == "String") {
GetStream(stream) << ("string_view");
return;
}
if (t->name.str == "Stream") {
GetStream(stream) << ("std::ostream&");
return;
}
if (t->name.str == "char") {
GetStream(stream) << ("char");
return;
}
if (t->name.str == "int") {
GetStream(stream) << ("int");
return;
}
if (t->name.str == "bool") {
GetStream(stream) << ("bool");
return;
}
GetStream(stream) << (t->name.str);
GetStream(stream) << ("*");
}
}
}
}
void EmitExpr(std::ostream& stream, ContextFinderContext* ctx, Expr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case Expr::Kind::New: {
auto* expr = reinterpret_cast<NewExpr*>(__tmp_switch_name);
(void)expr;
GetStream(stream) << ("({\nauto* self = new ");
EmitType(stream, expr->type);
GetStream(stream) << (";\n");
EmitStmt(stream, ctx, expr->body);
GetStream(stream) << ("self;\n");
GetStream(stream) << ("})");
break;
} case Expr::Kind::Number: {
auto* expr = reinterpret_cast<NumberExpr*>(__tmp_switch_name);
(void)expr;
GetStream(stream) << (expr->value.str);
break;
} case Expr::Kind::Str: {
auto* expr = reinterpret_cast<StrExpr*>(__tmp_switch_name);
(void)expr;
GetStream(stream) << (expr->value.str);
break;
} case Expr::Kind::Dot: {
auto* expr = reinterpret_cast<DotExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
GetStream(stream) << (".");
GetStream(stream) << (expr->name.str);
break;
} case Expr::Kind::Arrow: {
auto* expr = reinterpret_cast<ArrowExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
GetStream(stream) << ("->");
GetStream(stream) << (expr->name.str);
break;
} case Expr::Kind::Named: {
auto* expr = reinterpret_cast<NamedExpr*>(__tmp_switch_name);
(void)expr;
GetStream(stream) << (expr->name.str);
break;
} case Expr::Kind::Index: {
auto* expr = reinterpret_cast<IndexExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
int i;i = 0;
GetStream(stream) << ("[");
for (auto arg : expr->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
GetStream(stream) << ("]");
break;
} case Expr::Kind::ColonColon: {
auto* expr = reinterpret_cast<ColonColonExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
GetStream(stream) << ("::");
GetStream(stream) << (expr->name.str);
break;
} case Expr::Kind::Call: {
auto* expr = reinterpret_cast<CallExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__base = expr->base;
auto base = std::move(__tmp__base);
{
auto __tmp_switch_name = base;
switch (base->getKind()) {
case Expr::Kind::Named: {
auto* base = reinterpret_cast<NamedExpr*>(__tmp_switch_name);
(void)base;
if (base->name.str == "assert") {
GetStream(stream) << ("({\n");
GetStream(stream) << ("if (!(");
EmitExpr(stream, ctx, expr->args[0]);
GetStream(stream) << (")) {\n");
GetStream(stream) << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
GetStream(stream) << ("\n)ASSERT\";\n");
GetStream(stream) << ("exit(-1);\n");
GetStream(stream) << ("}\n})");
return;
}
if (base->name.str == "error") {
GetStream(stream) << ("({\n");
GetStream(stream) << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
GetStream(stream) << ("\n)ASSERT\";\n");
GetStream(stream) << ("exit(-1);\n");
GetStream(stream) << ("})");
return;
}
break;
} default: {
}
}
}
EmitExpr(stream, ctx, expr->base);
GetStream(stream) << ("(");
int i;i = 0;
{
auto __tmp_switch_name = base;
switch (base->getKind()) {
case Expr::Kind::Named: {
auto* base = reinterpret_cast<NamedExpr*>(__tmp_switch_name);
(void)base;
for (auto param : ctx->GetHiddenParams(base->name.str)) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
GetStream(stream) << (param->name.str);
}
break;
} default: {
}
}
}
for (auto arg : expr->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
GetStream(stream) << (")");
break;
} case Expr::Kind::CompEqEq: {
auto* expr = reinterpret_cast<CompEqEqExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->lhs);
GetStream(stream) << (" == ");
EmitExpr(stream, ctx, expr->rhs);
break;
} case Expr::Kind::Assign: {
auto* expr = reinterpret_cast<AssignExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->lhs);
GetStream(stream) << (" = ");
EmitExpr(stream, ctx, expr->rhs);
}
}
}
}
void EmitStmt(std::ostream& stream, ContextFinderContext* ctx, Stmt* stmt) {
{
auto __tmp_switch_name = stmt;
switch (stmt->getKind()) {
case Stmt::Kind::Compound: {
auto* stmt = reinterpret_cast<CompoundStmt*>(__tmp_switch_name);
(void)stmt;
for (auto cstmt : stmt->stmts) {
EmitStmt(stream, ctx, cstmt);
}
break;
} case Stmt::Kind::Return: {
auto* stmt = reinterpret_cast<ReturnStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("return ");
EmitExpr(stream, ctx, stmt->expr);
GetStream(stream) << (";\n");
break;
} case Stmt::Kind::Let: {
auto* stmt = reinterpret_cast<LetStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("auto __tmp__");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = ");
EmitExpr(stream, ctx, stmt->expr);
GetStream(stream) << (";\n");
GetStream(stream) << ("auto ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = std::move(__tmp__");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (");\n");
break;
} case Stmt::Kind::Var: {
auto* stmt = reinterpret_cast<VarStmt*>(__tmp_switch_name);
(void)stmt;
EmitTypeSignature(stream, stmt->type);
GetStream(stream) << (" ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (";");
break;
} case Stmt::Kind::OpenWithType: {
auto* stmt = reinterpret_cast<OpenWithTypeStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("{\n");
GetStream(stream) << ("auto __tmp_switch_name = ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (";\n");
GetStream(stream) << ("switch (");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
GetStream(stream) << ("break;\n} ");
}
case_open = true;
GetStream(stream) << ("case ");
EmitType(stream, stmt->type);
GetStream(stream) << ("::Kind::");
GetStream(stream) << (cstmt->name.str);
GetStream(stream) << (": {\n");
GetStream(stream) << ("auto* ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = reinterpret_cast<");
GetStream(stream) << (cstmt->name.str);
EmitType(stream, stmt->type);
GetStream(stream) << ("*>(__tmp_switch_name);\n");
GetStream(stream) << ("(void)");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (";\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
GetStream(stream) << ("break;\n} ");
}
case_open = true;
GetStream(stream) << ("default: {\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
if (case_open) {
GetStream(stream) << ("}\n");
}
GetStream(stream) << ("}\n");
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::Open: {
auto* stmt = reinterpret_cast<OpenStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("{\n");
GetStream(stream) << ("auto __tmp_switch_name = ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (";\n");
GetStream(stream) << ("switch (");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
GetStream(stream) << ("break;\n} ");
}
case_open = true;
GetStream(stream) << ("case Kind::");
GetStream(stream) << (cstmt->name.str);
GetStream(stream) << (": {\n");
GetStream(stream) << ("auto* ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = reinterpret_cast<");
GetStream(stream) << (cstmt->name.str);
GetStream(stream) << (">(__tmp_switch_name);\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
GetStream(stream) << ("break;\n} ");
}
case_open = true;
GetStream(stream) << ("default: {\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
if (case_open) {
GetStream(stream) << ("}\n");
}
GetStream(stream) << ("}\n");
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::Case: {
auto* stmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)stmt;
({
std::cerr << R"ASSERT(Assert failed: "Case can only be used as part of a switch...\n"
)ASSERT";
exit(-1);
});
break;
} case Stmt::Kind::Emitter: {
auto* stmt = reinterpret_cast<EmitterStmt*>(__tmp_switch_name);
(void)stmt;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Discard: {
auto* cstmt = reinterpret_cast<DiscardStmt*>(__tmp_switch_name);
(void)cstmt;
GetStream(stream) << (ctx->GetStdoutContext());
GetStream(stream) << (" << (");
EmitExpr(stream, ctx, cstmt->expr);
GetStream(stream) << (");\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
break;
} case Stmt::Kind::DbgEmitter: {
auto* stmt = reinterpret_cast<DbgEmitterStmt*>(__tmp_switch_name);
(void)stmt;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Discard: {
auto* cstmt = reinterpret_cast<DiscardStmt*>(__tmp_switch_name);
(void)cstmt;
GetStream(stream) << ("std::cerr << (");
EmitExpr(stream, ctx, cstmt->expr);
GetStream(stream) << (");\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
break;
} case Stmt::Kind::Break: {
auto* stmt = reinterpret_cast<BreakStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("break;\n");
break;
} case Stmt::Kind::ReturnVoid: {
auto* stmt = reinterpret_cast<ReturnVoidStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("return;\n");
break;
} case Stmt::Kind::If: {
auto* stmt = reinterpret_cast<IfStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("if (");
EmitExpr(stream, ctx, stmt->cond);
GetStream(stream) << (") {\n");
EmitStmt(stream, ctx, stmt->body);
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::IfElse: {
auto* stmt = reinterpret_cast<IfElseStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("if (");
EmitExpr(stream, ctx, stmt->cond);
GetStream(stream) << (") {\n");
EmitStmt(stream, ctx, stmt->body);
GetStream(stream) << ("} else {\n");
EmitStmt(stream, ctx, stmt->else_body);
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::Loop: {
auto* stmt = reinterpret_cast<LoopStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("while (true) {\n");
EmitStmt(stream, ctx, stmt->body);
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::For: {
auto* stmt = reinterpret_cast<ForStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("for (auto ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" : ");
EmitExpr(stream, ctx, stmt->sequence);
GetStream(stream) << (") {\n");
EmitStmt(stream, ctx, stmt->body);
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::Scope: {
auto* stmt = reinterpret_cast<ScopeStmt*>(__tmp_switch_name);
(void)stmt;
GetStream(stream) << ("{\n");
GetStream(stream) << ("auto __tmp__");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = ");
EmitExpr(stream, ctx, stmt->expr);
GetStream(stream) << (";\n");
GetStream(stream) << ("auto ");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (" = std::move(__tmp__");
GetStream(stream) << (stmt->name.str);
GetStream(stream) << (");\n");
EmitStmt(stream, ctx, stmt->body);
GetStream(stream) << ("}\n");
break;
} case Stmt::Kind::Default: {
auto* stmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)stmt;
({
std::cerr << R"ASSERT(Assert failed: "Default can only be used as part of a switch...\n"
)ASSERT";
exit(-1);
});
break;
} case Stmt::Kind::Discard: {
auto* stmt = reinterpret_cast<DiscardStmt*>(__tmp_switch_name);
(void)stmt;
EmitExpr(stream, ctx, stmt->expr);
GetStream(stream) << (";\n");
}
}
}
}
void EmitFuncDeclHeader(std::ostream& stream, FuncDecl* decl) {
EmitTypeSignature(stream, decl->ret_t);
GetStream(stream) << (" ");
GetStream(stream) << (decl->name.str);
GetStream(stream) << ("(");
int i;i = 0;
for (auto arg : decl->args) {
if (i == 0) {
} else {
GetStream(stream) << (", ");
}
increment(i);
EmitTypeSignature(stream, arg->type);
GetStream(stream) << (" ");
GetStream(stream) << (arg->name.str);
}
GetStream(stream) << (")");
}
void EmitFuncDecl(std::ostream& stream, ContextFinderContext* ctx, FuncDecl* decl) {
EmitFuncDeclHeader(stream, decl);
GetStream(stream) << (" {\n");
EmitStmt(stream, ctx, decl->body);
GetStream(stream) << ("}\n");
}
void Emit(std::ostream& stream, Module* m) {
GetStream(stream) << ("namespace ");
GetStream(stream) << (m->mod_name.str);
GetStream(stream) << (" {\n\n");
auto __tmp__ctx = ({
auto* self = new ContextFinderContext;
self;
});
auto ctx = std::move(__tmp__ctx);
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
default: {
break;
} case Decl::Kind::Func: {
auto* decl = reinterpret_cast<FuncDecl*>(__tmp_switch_name);
(void)decl;
EmitFuncDeclHeader(stream, decl);
GetStream(stream) << (";\n");
ctx->RegisterFunc(decl);
break;
} case Decl::Kind::Context: {
auto* decl = reinterpret_cast<ContextDecl*>(__tmp_switch_name);
(void)decl;
ctx->RegisterContext(decl);
}
}
}
}
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
default: {
break;
} case Decl::Kind::Func: {
auto* decl = reinterpret_cast<FuncDecl*>(__tmp_switch_name);
(void)decl;
for (auto arg : decl->args) {
auto __tmp__sub_ctx = ctx->isContextUsage(arg->name.str);
auto sub_ctx = std::move(__tmp__sub_ctx);
if (sub_ctx) {
ctx->HardSetContext(decl, sub_ctx);
} else {
break;
}
}
}
}
}
}
GetStream(stream) << ("\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
default: {
break;
} case Decl::Kind::Func: {
auto* decl = reinterpret_cast<FuncDecl*>(__tmp_switch_name);
(void)decl;
EmitFuncDecl(stream, ctx, decl);
}
}
}
}
GetStream(stream) << ("\n}  // namespace ");
GetStream(stream) << (m->mod_name.str);
GetStream(stream) << ("\n");
}

}  // namespace lowering_spec
