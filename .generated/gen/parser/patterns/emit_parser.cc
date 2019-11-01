namespace production_spec {

void emitSanitizedPatternExpr(EmitContext* emit_ctx, PatternExpr* expr);
void emitResultIfPresent(EmitContext* emit_ctx);
void emitConditionalArg(EmitContext* emit_ctx, PatternExpr* expr);
void emitSanitizedPatternStmt(EmitContext* emit_ctx, PatternStmt* s);
void emitBasics(ModuleContext* globals, Module* m, bool is_header);

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
void emitBasics(ModuleContext* globals, Module* m, bool is_header) {
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
if (is_header) {
} else {
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
}
std::cout << ("}  // namespace parser\n");
std::cout << ("}  // namespace ");
std::cout << (m->mod_name.str);
std::cout << ("\n");
}

}  // namespace production_spec
