namespace production_spec {

void productionName(std::ostream& stream, DefineWithTypeDecl* d);
void emitSanitizedPatternExpr(std::ostream& stream, EmitContext* emit_ctx, PatternExpr* expr);
void emitResultIfPresent(std::ostream& stream, EmitContext* emit_ctx);
void emitConditionalArg(std::ostream& stream, EmitContext* emit_ctx, PatternExpr* expr);
void emitSanitizedPatternStmt(std::ostream& stream, EmitContext* emit_ctx, PatternStmt* s);
void emitBasics(std::ostream& stream, ModuleContext* globals, Module* m, bool is_header);

void productionName(std::ostream& stream, DefineWithTypeDecl* d) {
stream << ("_production_");
stream << (d->name.str);
}
void emitSanitizedPatternExpr(std::ostream& stream, EmitContext* emit_ctx, PatternExpr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::New: {
auto* expr = reinterpret_cast<NewPatternExpr*>(__tmp_switch_name);
(void)expr;
stream << ("({\n");
stream << ("auto __current_self = ");
emitNewType(stream, expr->type);
stream << (";");
emitSanitizedPatternStmt(stream, emit_ctx, expr->value);
stream << ("__current_self;\n");
stream << ("})");
break;
} case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
if (emit_ctx->globals->isToken(expr->name.str)) {
stream << ("tokens.expect(tok::");
stream << (expr->name.str);
stream << (")");
} else {
stream << ("_production_");
stream << (expr->name.str);
stream << ("(tokens)");
}
break;
} case PatternExpr::Kind::Pop: {
auto* expr = reinterpret_cast<PopPatternExpr*>(__tmp_switch_name);
(void)expr;
stream << ("_tmp_");
stream << (emit_ctx->Pop());
break;
} case PatternExpr::Kind::CommaConcat: {
auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__emit_ctx = emit_ctx->ConcatContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
auto __tmp__type = emit_ctx->TypeCheck(expr->element);
auto type = std::move(__tmp__type);
stream << ("([&]{\n");
stream << ("std::vector<");
emitTypeExpr(stream, type);
stream << ("> __current_vector__;\n");
auto __tmp__succ = emit_ctx->GetSuccessor(expr);
auto succ = std::move(__tmp__succ);
if (succ) {
{
auto __tmp_switch_name = succ;
switch (succ->getKind()) {
case PatternStmt::Kind::String: {
auto* succ = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)succ;
stream << ("   if (!tokens.peak_check_str(");
stream << (succ->value.str);
stream << ("))");
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
stream << ("   if (!tokens.peak_check(tok::eof))");
}
stream << (" {\n");
stream << ("    while (true) {\n");
stream << (" __current_vector__.push_back([&]{");
emitSanitizedPatternStmt(stream, emit_ctx, expr->element);
emitResultIfPresent(stream, emit_ctx);
stream << (" }());");
stream << (" if (tokens.peak_check(tok::");
stream << (expr->comma.str);
stream << (")) {\n");
stream << ("   tokens.expect(tok::");
stream << (expr->comma.str);
stream << (");\n");
stream << (" } else { break; }\n");
stream << ("  }}return __current_vector__;\n}())\n");
break;
} case PatternExpr::Kind::Concat: {
auto* expr = reinterpret_cast<ConcatPatternExpr*>(__tmp_switch_name);
(void)expr;
auto __tmp__emit_ctx = emit_ctx->ConcatContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
auto __tmp__type = emit_ctx->TypeCheck(expr->element);
auto type = std::move(__tmp__type);
stream << ("([&]{\n");
stream << ("std::vector<");
emitTypeExpr(stream, type);
stream << ("> __current_vector__;\n");
stream << ("    while (true) {\n");
auto __tmp__succ = emit_ctx->GetSuccessor(expr);
auto succ = std::move(__tmp__succ);
if (succ) {
{
auto __tmp_switch_name = succ;
switch (succ->getKind()) {
case PatternStmt::Kind::String: {
auto* succ = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)succ;
stream << ("   if (tokens.peak_check_str(");
stream << (succ->value.str);
stream << ("))");
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
stream << ("   if (tokens.peak_check(tok::eof))");
}
stream << (" { break; }\n");
stream << (" __current_vector__.push_back([&]{");
emitSanitizedPatternStmt(stream, emit_ctx, expr->element);
emitResultIfPresent(stream, emit_ctx);
stream << (" }());");
stream << ("  }\nreturn __current_vector__;\n}())\n");
break;
} case PatternExpr::Kind::Self: {
auto* expr = reinterpret_cast<SelfPatternExpr*>(__tmp_switch_name);
(void)expr;
stream << ("(0 /* unknown expr*/)");
}
}
}
}
void emitResultIfPresent(std::ostream& stream, EmitContext* emit_ctx) {
if (emit_ctx->has_result) {
if (emit_ctx->is_inside_expr) {
stream << ("expr_result = result;\ncontinue;\n");
} else {
stream << ("return result;\n");
}
} else {
stream << ("tokens.unexpected();\n");
}
}
void emitConditionalArg(std::ostream& stream, EmitContext* emit_ctx, PatternExpr* expr) {
{
auto __tmp_switch_name = expr;
switch (expr->getKind()) {
case PatternExpr::Kind::Named: {
auto* expr = reinterpret_cast<NamedPatternExpr*>(__tmp_switch_name);
(void)expr;
if (emit_ctx->globals->isToken(expr->name.str)) {
stream << ("tokens.peak_check(tok::");
stream << (expr->name.str);
stream << (")");
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
void emitSanitizedPatternStmt(std::ostream& stream, EmitContext* emit_ctx, PatternStmt* s) {
{
auto __tmp_switch_name = s;
switch (s->getKind()) {
case PatternStmt::Kind::String: {
auto* s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)s;
stream << ("tokens.expect(");
stream << (s->value.str);
stream << (");\n");
break;
} case PatternStmt::Kind::Assign: {
auto* s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)s;
stream << ("__current_self->");
stream << (s->name.str);
stream << (" = ");
emitSanitizedPatternExpr(stream, emit_ctx, s->value);
stream << (";\n");
break;
} case PatternStmt::Kind::Push: {
auto* s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__next_id = emit_ctx->Push();
auto next_id = std::move(__tmp__next_id);
stream << ("auto _tmp_");
stream << (next_id);
stream << (" = ");
emitSanitizedPatternExpr(stream, emit_ctx, s->value);
stream << (";\n");
break;
} case PatternStmt::Kind::Wrap: {
auto* s = reinterpret_cast<WrapPatternStmt*>(__tmp_switch_name);
(void)s;
emit_ctx->has_result = true;
stream << ("auto result = ");
emitSanitizedPatternExpr(stream, emit_ctx, s->value);
stream << (";\n");
break;
} case PatternStmt::Kind::Compound: {
auto* s = reinterpret_cast<CompoundPatternStmt*>(__tmp_switch_name);
(void)s;
emit_ctx->RegisterConcatSuccessors(s->items);
for (auto stmt : s->items) {
emitSanitizedPatternStmt(stream, emit_ctx, stmt);
}
break;
} case PatternStmt::Kind::Conditional: {
auto* s = reinterpret_cast<ConditionalPatternStmt*>(__tmp_switch_name);
(void)s;
auto __tmp__emit_ctx = emit_ctx->NewConditionalContext();
auto emit_ctx = std::move(__tmp__emit_ctx);
stream << ("if (");
auto __tmp__peak_s = getFirstItem(s->value);
auto peak_s = std::move(__tmp__peak_s);
{
auto __tmp_switch_name = peak_s;
switch (peak_s->getKind()) {
case PatternStmt::Kind::String: {
auto* peak_s = reinterpret_cast<StringPatternStmt*>(__tmp_switch_name);
(void)peak_s;
stream << ("tokens.peak_check_str(");
stream << (peak_s->value.str);
stream << (")");
break;
} case PatternStmt::Kind::Push: {
auto* peak_s = reinterpret_cast<PushPatternStmt*>(__tmp_switch_name);
(void)peak_s;
emitConditionalArg(stream, emit_ctx, peak_s->value);
break;
} case PatternStmt::Kind::Assign: {
auto* peak_s = reinterpret_cast<AssignPatternStmt*>(__tmp_switch_name);
(void)peak_s;
emitConditionalArg(stream, emit_ctx, peak_s->value);
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
stream << (") {\n");
emitSanitizedPatternStmt(stream, emit_ctx, s->value);
emitResultIfPresent(stream, emit_ctx);
stream << ("}\n");
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
emitTypeExpr(stream, s->type);
stream << (" expr_result = ");
emitSanitizedPatternExpr(stream, emit_ctx, s->base);
stream << (";\n");
stream << ("while (true) {\n");
auto __tmp__next_id = emit_ctx->Push();
auto next_id = std::move(__tmp__next_id);
stream << ("auto _tmp_");
stream << (next_id);
stream << (" = expr_result;");
emitSanitizedPatternStmt(stream, emit_ctx, s->value);
stream << ("return expr_result;\n");
stream << ("}\n");
}
}
}
}
void emitBasics(std::ostream& stream, ModuleContext* globals, Module* m, bool is_header) {
stream << ("namespace ");
stream << (m->mod_name.str);
stream << ("{\n");
stream << ("namespace parser {\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::DefineWithType: {
auto* decl = reinterpret_cast<DefineWithTypeDecl*>(__tmp_switch_name);
(void)decl;
emitTypeExpr(stream, decl->type);
stream << (" ");
productionName(stream, decl);
stream << ("(Tokenizer& tokens);\n");
break;
} case Decl::Kind::Entry: {
auto* decl = reinterpret_cast<EntryDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__entry_type = ({
auto* self = globals->New<NamedTypeDeclExpr>();
self->name = decl->name;
self;
});
auto entry_type = std::move(__tmp__entry_type);
emitTypeExpr(stream, entry_type);
stream << (" DoParse(Tokenizer& tokens);\n");
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
emitTypeExpr(stream, decl->type);
stream << (" ");
productionName(stream, decl);
stream << ("(Tokenizer& tokens) {\n");
emitSanitizedPatternStmt(stream, emit_ctx, decl->value);
emitResultIfPresent(stream, emit_ctx);
stream << ("}\n");
break;
} case Decl::Kind::Entry: {
auto* decl = reinterpret_cast<EntryDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__entry_type = ({
auto* self = globals->New<NamedTypeDeclExpr>();
self->name = decl->name;
self;
});
auto entry_type = std::move(__tmp__entry_type);
emitTypeExpr(stream, entry_type);
stream << (" DoParse(Tokenizer& tokens) {\n");
stream << ("  return _production_");
stream << (decl->name.str);
stream << ("(tokens);\n");
stream << ("}\n");
break;
} default: {
}
}
}
}
}
stream << ("}  // namespace parser\n");
stream << ("}  // namespace ");
stream << (m->mod_name.str);
stream << ("\n");
}

}  // namespace production_spec
