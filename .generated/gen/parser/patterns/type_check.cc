namespace production_spec {

void collectTypeFields(FieldTypeCheckContext* field_tc_ctx, TypeCheckContext* tc_ctx, PatternStmt* stmt);
TypeDeclExpr* doTypeCheckExpr(TypeCheckContext* tc_ctx, PatternExpr* expr);
TypeDeclExpr* doTypeCheck(TypeCheckContext* tc_ctx, PatternStmt* s);

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

}  // namespace production_spec
