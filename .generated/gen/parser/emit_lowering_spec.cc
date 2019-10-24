namespace lowering_spec {

void EmitType(TypeRef* t);
void EmitTypeSignature(TypeRef* t);
void EmitExpr(ContextFinderContext* ctx, Expr* expr);
void EmitStmt(ContextFinderContext* ctx, Stmt* stmt);
void EmitFuncDeclHeader(FuncDecl* decl);
void EmitFuncDecl(ContextFinderContext* ctx, FuncDecl* decl);
void Emit(Module* m);

void EmitType(TypeRef* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeRef::Kind::Void: {
auto* t = reinterpret_cast<VoidTypeRef*>(__tmp_switch_name);
(void)t;
std::cout << ("void");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(t->base);
std::cout << ("::");
std::cout << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
(void)t;
std::cout << (t->name.str);
break;
} default: {
std::cout << ("unknown");
}
}
}
}
void EmitTypeSignature(TypeRef* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeRef::Kind::Void: {
auto* t = reinterpret_cast<VoidTypeRef*>(__tmp_switch_name);
(void)t;
std::cout << ("void");
break;
} case TypeRef::Kind::Template: {
auto* t = reinterpret_cast<TemplateTypeRef*>(__tmp_switch_name);
(void)t;
EmitTypeSignature(t->base);
std::cout << ("<");
int i;i = 0;
for (auto arg : t->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitTypeSignature(arg);
}
std::cout << (">");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(t->base);
std::cout << ("::");
std::cout << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
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
}
}
}
}
void EmitExpr(ContextFinderContext* ctx, Expr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case Expr::Kind::New: {
auto* expr = reinterpret_cast<NewExpr*>(__tmp_switch_name);
(void)expr;
std::cout << ("({\nauto* self = new ");
EmitType(expr->type);
std::cout << (";\n");
EmitStmt(ctx, expr->body);
std::cout << ("self;\n");
std::cout << ("})");
break;
} case Expr::Kind::Number: {
auto* expr = reinterpret_cast<NumberExpr*>(__tmp_switch_name);
(void)expr;
std::cout << (expr->value.str);
break;
} case Expr::Kind::Str: {
auto* expr = reinterpret_cast<StrExpr*>(__tmp_switch_name);
(void)expr;
std::cout << (expr->value.str);
break;
} case Expr::Kind::Dot: {
auto* expr = reinterpret_cast<DotExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->base);
std::cout << (".");
std::cout << (expr->name.str);
break;
} case Expr::Kind::Arrow: {
auto* expr = reinterpret_cast<ArrowExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->base);
std::cout << ("->");
std::cout << (expr->name.str);
break;
} case Expr::Kind::Named: {
auto* expr = reinterpret_cast<NamedExpr*>(__tmp_switch_name);
(void)expr;
std::cout << (expr->name.str);
break;
} case Expr::Kind::Index: {
auto* expr = reinterpret_cast<IndexExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->base);
int i;i = 0;
std::cout << ("[");
for (auto arg : expr->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitExpr(ctx, arg);
}
std::cout << ("]");
break;
} case Expr::Kind::ColonColon: {
auto* expr = reinterpret_cast<ColonColonExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->base);
std::cout << ("::");
std::cout << (expr->name.str);
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
std::cout << ("({\n");
std::cout << ("if (!(");
EmitExpr(ctx, expr->args[0]);
std::cout << (")) {\n");
std::cout << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitExpr(ctx, arg);
}
std::cout << ("\n)ASSERT\";\n");
std::cout << ("exit(-1);\n");
std::cout << ("}\n})");
return;
}
if (base->name.str == "error") {
std::cout << ("({\n");
std::cout << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitExpr(ctx, arg);
}
std::cout << ("\n)ASSERT\";\n");
std::cout << ("exit(-1);\n");
std::cout << ("})");
return;
}
break;
} default: {
}
}
}
EmitExpr(ctx, expr->base);
std::cout << ("(");
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
std::cout << (", ");
}
increment(i);
std::cout << (param->name.str);
}
break;
} default: {
}
}
}
for (auto arg : expr->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitExpr(ctx, arg);
}
std::cout << (")");
break;
} case Expr::Kind::CompEqEq: {
auto* expr = reinterpret_cast<CompEqEqExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->lhs);
std::cout << (" == ");
EmitExpr(ctx, expr->rhs);
break;
} case Expr::Kind::Assign: {
auto* expr = reinterpret_cast<AssignExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(ctx, expr->lhs);
std::cout << (" = ");
EmitExpr(ctx, expr->rhs);
}
}
}
}
void EmitStmt(ContextFinderContext* ctx, Stmt* stmt) {
{
auto __tmp_switch_name = stmt;
switch (stmt->getKind()) {
case Stmt::Kind::Compound: {
auto* stmt = reinterpret_cast<CompoundStmt*>(__tmp_switch_name);
(void)stmt;
for (auto cstmt : stmt->stmts) {
EmitStmt(ctx, cstmt);
}
break;
} case Stmt::Kind::Return: {
auto* stmt = reinterpret_cast<ReturnStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("return ");
EmitExpr(ctx, stmt->expr);
std::cout << (";\n");
break;
} case Stmt::Kind::Let: {
auto* stmt = reinterpret_cast<LetStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("auto __tmp__");
std::cout << (stmt->name.str);
std::cout << (" = ");
EmitExpr(ctx, stmt->expr);
std::cout << (";\n");
std::cout << ("auto ");
std::cout << (stmt->name.str);
std::cout << (" = std::move(__tmp__");
std::cout << (stmt->name.str);
std::cout << (");\n");
break;
} case Stmt::Kind::Var: {
auto* stmt = reinterpret_cast<VarStmt*>(__tmp_switch_name);
(void)stmt;
EmitTypeSignature(stmt->type);
std::cout << (" ");
std::cout << (stmt->name.str);
std::cout << (";");
break;
} case Stmt::Kind::OpenWithType: {
auto* stmt = reinterpret_cast<OpenWithTypeStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("{\n");
std::cout << ("auto __tmp_switch_name = ");
std::cout << (stmt->name.str);
std::cout << (";\n");
std::cout << ("switch (");
std::cout << (stmt->name.str);
std::cout << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
std::cout << ("break;\n} ");
}
case_open = true;
std::cout << ("case ");
EmitType(stmt->type);
std::cout << ("::Kind::");
std::cout << (cstmt->name.str);
std::cout << (": {\n");
std::cout << ("auto* ");
std::cout << (stmt->name.str);
std::cout << (" = reinterpret_cast<");
std::cout << (cstmt->name.str);
EmitType(stmt->type);
std::cout << ("*>(__tmp_switch_name);\n");
std::cout << ("(void)");
std::cout << (stmt->name.str);
std::cout << (";\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
std::cout << ("break;\n} ");
}
case_open = true;
std::cout << ("default: {\n");
break;
} default: {
EmitStmt(ctx, cstmt);
}
}
}
}
if (case_open) {
std::cout << ("}\n");
}
std::cout << ("}\n");
std::cout << ("}\n");
break;
} case Stmt::Kind::Open: {
auto* stmt = reinterpret_cast<OpenStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("{\n");
std::cout << ("auto __tmp_switch_name = ");
std::cout << (stmt->name.str);
std::cout << (";\n");
std::cout << ("switch (");
std::cout << (stmt->name.str);
std::cout << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
std::cout << ("break;\n} ");
}
case_open = true;
std::cout << ("case Kind::");
std::cout << (cstmt->name.str);
std::cout << (": {\n");
std::cout << ("auto* ");
std::cout << (stmt->name.str);
std::cout << (" = reinterpret_cast<");
std::cout << (cstmt->name.str);
std::cout << (">(__tmp_switch_name);\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
std::cout << ("break;\n} ");
}
case_open = true;
std::cout << ("default: {\n");
break;
} default: {
EmitStmt(ctx, cstmt);
}
}
}
}
if (case_open) {
std::cout << ("}\n");
}
std::cout << ("}\n");
std::cout << ("}\n");
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
std::cout << ("std::cout << (");
EmitExpr(ctx, cstmt->expr);
std::cout << (");\n");
break;
} default: {
EmitStmt(ctx, cstmt);
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
std::cout << ("std::cerr << (");
EmitExpr(ctx, cstmt->expr);
std::cout << (");\n");
break;
} default: {
EmitStmt(ctx, cstmt);
}
}
}
}
break;
} case Stmt::Kind::Break: {
auto* stmt = reinterpret_cast<BreakStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("break;\n");
break;
} case Stmt::Kind::ReturnVoid: {
auto* stmt = reinterpret_cast<ReturnVoidStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("return;\n");
break;
} case Stmt::Kind::If: {
auto* stmt = reinterpret_cast<IfStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("if (");
EmitExpr(ctx, stmt->cond);
std::cout << (") {\n");
EmitStmt(ctx, stmt->body);
std::cout << ("}\n");
break;
} case Stmt::Kind::IfElse: {
auto* stmt = reinterpret_cast<IfElseStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("if (");
EmitExpr(ctx, stmt->cond);
std::cout << (") {\n");
EmitStmt(ctx, stmt->body);
std::cout << ("} else {\n");
EmitStmt(ctx, stmt->else_body);
std::cout << ("}\n");
break;
} case Stmt::Kind::Loop: {
auto* stmt = reinterpret_cast<LoopStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("while (true) {\n");
EmitStmt(ctx, stmt->body);
std::cout << ("}\n");
break;
} case Stmt::Kind::For: {
auto* stmt = reinterpret_cast<ForStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("for (auto ");
std::cout << (stmt->name.str);
std::cout << (" : ");
EmitExpr(ctx, stmt->sequence);
std::cout << (") {\n");
EmitStmt(ctx, stmt->body);
std::cout << ("}\n");
break;
} case Stmt::Kind::Scope: {
auto* stmt = reinterpret_cast<ScopeStmt*>(__tmp_switch_name);
(void)stmt;
std::cout << ("{\n");
std::cout << ("auto __tmp__");
std::cout << (stmt->name.str);
std::cout << (" = ");
EmitExpr(ctx, stmt->expr);
std::cout << (";\n");
std::cout << ("auto ");
std::cout << (stmt->name.str);
std::cout << (" = std::move(__tmp__");
std::cout << (stmt->name.str);
std::cout << (");\n");
EmitStmt(ctx, stmt->body);
std::cout << ("}\n");
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
EmitExpr(ctx, stmt->expr);
std::cout << (";\n");
}
}
}
}
void EmitFuncDeclHeader(FuncDecl* decl) {
EmitTypeSignature(decl->ret_t);
std::cout << (" ");
std::cout << (decl->name.str);
std::cout << ("(");
int i;i = 0;
for (auto arg : decl->args) {
if (i == 0) {
} else {
std::cout << (", ");
}
increment(i);
EmitTypeSignature(arg->type);
std::cout << (" ");
std::cout << (arg->name.str);
}
std::cout << (")");
}
void EmitFuncDecl(ContextFinderContext* ctx, FuncDecl* decl) {
EmitFuncDeclHeader(decl);
std::cout << (" {\n");
EmitStmt(ctx, decl->body);
std::cout << ("}\n");
}
void Emit(Module* m) {
std::cout << ("namespace ");
std::cout << (m->mod_name.str);
std::cout << (" {\n\n");
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
EmitFuncDeclHeader(decl);
std::cout << (";\n");
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
std::cout << ("\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
default: {
break;
} case Decl::Kind::Func: {
auto* decl = reinterpret_cast<FuncDecl*>(__tmp_switch_name);
(void)decl;
EmitFuncDecl(ctx, decl);
}
}
}
}
std::cout << ("\n}  // namespace ");
std::cout << (m->mod_name.str);
std::cout << ("\n");
}

}  // namespace lowering_spec
