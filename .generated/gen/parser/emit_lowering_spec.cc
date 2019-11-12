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
stream << ("void");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(stream, t->base);
stream << ("::");
stream << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
(void)t;
stream << (t->name.str);
break;
} default: {
stream << ("unknown");
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
stream << ("void");
break;
} case TypeRef::Kind::Template: {
auto* t = reinterpret_cast<TemplateTypeRef*>(__tmp_switch_name);
(void)t;
EmitTypeSignature(stream, t->base);
stream << ("<");
int i;i = 0;
for (auto arg : t->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitTypeSignature(stream, arg);
}
stream << (">");
break;
} case TypeRef::Kind::Member: {
auto* t = reinterpret_cast<MemberTypeRef*>(__tmp_switch_name);
(void)t;
EmitType(stream, t->base);
stream << ("::");
stream << (t->name.str);
break;
} case TypeRef::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeRef*>(__tmp_switch_name);
(void)t;
if (t->name.str == "Token") {
stream << ("tok::Token");
return;
}
if (t->name.str == "Array") {
stream << ("std::vector");
return;
}
if (t->name.str == "Map") {
stream << ("std::map");
return;
}
if (t->name.str == "String") {
stream << ("string_view");
return;
}
if (t->name.str == "Stream") {
stream << ("std::ostream&");
return;
}
if (t->name.str == "char") {
stream << ("char");
return;
}
if (t->name.str == "int") {
stream << ("int");
return;
}
if (t->name.str == "bool") {
stream << ("bool");
return;
}
stream << (t->name.str);
stream << ("*");
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
stream << ("({\nauto* self = new ");
EmitType(stream, expr->type);
stream << (";\n");
EmitStmt(stream, ctx, expr->body);
stream << ("self;\n");
stream << ("})");
break;
} case Expr::Kind::Number: {
auto* expr = reinterpret_cast<NumberExpr*>(__tmp_switch_name);
(void)expr;
stream << (expr->value.str);
break;
} case Expr::Kind::Str: {
auto* expr = reinterpret_cast<StrExpr*>(__tmp_switch_name);
(void)expr;
stream << (expr->value.str);
break;
} case Expr::Kind::Dot: {
auto* expr = reinterpret_cast<DotExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
stream << (".");
stream << (expr->name.str);
break;
} case Expr::Kind::Arrow: {
auto* expr = reinterpret_cast<ArrowExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
stream << ("->");
stream << (expr->name.str);
break;
} case Expr::Kind::Named: {
auto* expr = reinterpret_cast<NamedExpr*>(__tmp_switch_name);
(void)expr;
stream << (expr->name.str);
break;
} case Expr::Kind::Index: {
auto* expr = reinterpret_cast<IndexExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
int i;i = 0;
stream << ("[");
for (auto arg : expr->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
stream << ("]");
break;
} case Expr::Kind::ColonColon: {
auto* expr = reinterpret_cast<ColonColonExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->base);
stream << ("::");
stream << (expr->name.str);
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
stream << ("({\n");
stream << ("if (!(");
EmitExpr(stream, ctx, expr->args[0]);
stream << (")) {\n");
stream << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
stream << ("\n)ASSERT\";\n");
stream << ("exit(-1);\n");
stream << ("}\n})");
return;
}
if (base->name.str == "error") {
stream << ("({\n");
stream << ("std::cerr << R\"ASSERT(Assert failed: ");
int i;i = 0;
for (auto arg : expr->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
stream << ("\n)ASSERT\";\n");
stream << ("exit(-1);\n");
stream << ("})");
return;
}
break;
} default: {
}
}
}
EmitExpr(stream, ctx, expr->base);
stream << ("(");
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
stream << (", ");
}
increment(i);
stream << (param->name.str);
}
break;
} default: {
}
}
}
for (auto arg : expr->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitExpr(stream, ctx, arg);
}
stream << (")");
break;
} case Expr::Kind::CompEqEq: {
auto* expr = reinterpret_cast<CompEqEqExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->lhs);
stream << (" == ");
EmitExpr(stream, ctx, expr->rhs);
break;
} case Expr::Kind::Assign: {
auto* expr = reinterpret_cast<AssignExpr*>(__tmp_switch_name);
(void)expr;
EmitExpr(stream, ctx, expr->lhs);
stream << (" = ");
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
stream << ("return ");
EmitExpr(stream, ctx, stmt->expr);
stream << (";\n");
break;
} case Stmt::Kind::Let: {
auto* stmt = reinterpret_cast<LetStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("auto __tmp__");
stream << (stmt->name.str);
stream << (" = ");
EmitExpr(stream, ctx, stmt->expr);
stream << (";\n");
stream << ("auto ");
stream << (stmt->name.str);
stream << (" = std::move(__tmp__");
stream << (stmt->name.str);
stream << (");\n");
break;
} case Stmt::Kind::Var: {
auto* stmt = reinterpret_cast<VarStmt*>(__tmp_switch_name);
(void)stmt;
EmitTypeSignature(stream, stmt->type);
stream << (" ");
stream << (stmt->name.str);
stream << (";");
break;
} case Stmt::Kind::OpenWithType: {
auto* stmt = reinterpret_cast<OpenWithTypeStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("{\n");
stream << ("auto __tmp_switch_name = ");
stream << (stmt->name.str);
stream << (";\n");
stream << ("switch (");
stream << (stmt->name.str);
stream << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
stream << ("break;\n} ");
}
case_open = true;
stream << ("case ");
EmitType(stream, stmt->type);
stream << ("::Kind::");
stream << (cstmt->name.str);
stream << (": {\n");
stream << ("auto* ");
stream << (stmt->name.str);
stream << (" = reinterpret_cast<");
stream << (cstmt->name.str);
EmitType(stream, stmt->type);
stream << ("*>(__tmp_switch_name);\n");
stream << ("(void)");
stream << (stmt->name.str);
stream << (";\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
stream << ("break;\n} ");
}
case_open = true;
stream << ("default: {\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
if (case_open) {
stream << ("}\n");
}
stream << ("}\n");
stream << ("}\n");
break;
} case Stmt::Kind::Open: {
auto* stmt = reinterpret_cast<OpenStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("{\n");
stream << ("auto __tmp_switch_name = ");
stream << (stmt->name.str);
stream << (";\n");
stream << ("switch (");
stream << (stmt->name.str);
stream << ("->getKind()) {\n");
bool case_open;case_open = false;
for (auto cstmt : AsCompound(stmt->body)->stmts) {
{
auto __tmp_switch_name = cstmt;
switch (cstmt->getKind()) {
case Stmt::Kind::Case: {
auto* cstmt = reinterpret_cast<CaseStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
stream << ("break;\n} ");
}
case_open = true;
stream << ("case Kind::");
stream << (cstmt->name.str);
stream << (": {\n");
stream << ("auto* ");
stream << (stmt->name.str);
stream << (" = reinterpret_cast<");
stream << (cstmt->name.str);
stream << (">(__tmp_switch_name);\n");
break;
} case Stmt::Kind::Default: {
auto* cstmt = reinterpret_cast<DefaultStmt*>(__tmp_switch_name);
(void)cstmt;
if (case_open) {
stream << ("break;\n} ");
}
case_open = true;
stream << ("default: {\n");
break;
} default: {
EmitStmt(stream, ctx, cstmt);
}
}
}
}
if (case_open) {
stream << ("}\n");
}
stream << ("}\n");
stream << ("}\n");
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
stream << (ctx->GetStdoutContext());
stream << (" << (");
EmitExpr(stream, ctx, cstmt->expr);
stream << (");\n");
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
stream << ("std::cerr << (");
EmitExpr(stream, ctx, cstmt->expr);
stream << (");\n");
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
stream << ("break;\n");
break;
} case Stmt::Kind::ReturnVoid: {
auto* stmt = reinterpret_cast<ReturnVoidStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("return;\n");
break;
} case Stmt::Kind::If: {
auto* stmt = reinterpret_cast<IfStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("if (");
EmitExpr(stream, ctx, stmt->cond);
stream << (") {\n");
EmitStmt(stream, ctx, stmt->body);
stream << ("}\n");
break;
} case Stmt::Kind::IfElse: {
auto* stmt = reinterpret_cast<IfElseStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("if (");
EmitExpr(stream, ctx, stmt->cond);
stream << (") {\n");
EmitStmt(stream, ctx, stmt->body);
stream << ("} else {\n");
EmitStmt(stream, ctx, stmt->else_body);
stream << ("}\n");
break;
} case Stmt::Kind::Loop: {
auto* stmt = reinterpret_cast<LoopStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("while (true) {\n");
EmitStmt(stream, ctx, stmt->body);
stream << ("}\n");
break;
} case Stmt::Kind::For: {
auto* stmt = reinterpret_cast<ForStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("for (auto ");
stream << (stmt->name.str);
stream << (" : ");
EmitExpr(stream, ctx, stmt->sequence);
stream << (") {\n");
EmitStmt(stream, ctx, stmt->body);
stream << ("}\n");
break;
} case Stmt::Kind::Scope: {
auto* stmt = reinterpret_cast<ScopeStmt*>(__tmp_switch_name);
(void)stmt;
stream << ("{\n");
stream << ("auto __tmp__");
stream << (stmt->name.str);
stream << (" = ");
EmitExpr(stream, ctx, stmt->expr);
stream << (";\n");
stream << ("auto ");
stream << (stmt->name.str);
stream << (" = std::move(__tmp__");
stream << (stmt->name.str);
stream << (");\n");
EmitStmt(stream, ctx, stmt->body);
stream << ("}\n");
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
stream << (";\n");
}
}
}
}
void EmitFuncDeclHeader(std::ostream& stream, FuncDecl* decl) {
EmitTypeSignature(stream, decl->ret_t);
stream << (" ");
stream << (decl->name.str);
stream << ("(");
int i;i = 0;
for (auto arg : decl->args) {
if (i == 0) {
} else {
stream << (", ");
}
increment(i);
EmitTypeSignature(stream, arg->type);
stream << (" ");
stream << (arg->name.str);
}
stream << (")");
}
void EmitFuncDecl(std::ostream& stream, ContextFinderContext* ctx, FuncDecl* decl) {
EmitFuncDeclHeader(stream, decl);
stream << (" {\n");
EmitStmt(stream, ctx, decl->body);
stream << ("}\n");
}
void Emit(std::ostream& stream, Module* m) {
stream << ("namespace ");
stream << (m->mod_name.str);
stream << (" {\n\n");
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
stream << (";\n");
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
stream << ("\n");
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
stream << ("\n}  // namespace ");
stream << (m->mod_name.str);
stream << ("\n");
}

}  // namespace lowering_spec
