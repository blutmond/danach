namespace lower_regex_to_nfa {

void skip(ASTContext* ast, Node* a, Node* b);
char lowerToChar(RegexExpr* e);
NodePair* lower(ASTContext* ast, LetContext* ctx, RegexExpr* e);
NFAGraphDecl* rewriteRegexDecl(ASTContext* ast, RegexDecl* decl);

void skip(ASTContext* ast, Node* a, Node* b) {
a->edges.push_back(({
auto* self = ast->New<SkipToEdge>();
self->next = b;
self;
}));
}
char lowerToChar(RegexExpr* e) {
{
auto __tmp_switch_name = e;
switch (e->getKind()) {
case RegexExpr::Kind::String: {
auto* e = reinterpret_cast<StringRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__tmp = Unescaped(e->value.str);
auto tmp = std::move(__tmp__tmp);
({
if (!(tmp.size() == 1)) {
std::cerr << R"ASSERT(Assert failed: tmp.size() == 1, e->value, " is not a single char"
)ASSERT";
exit(-1);
}
});
return tmp[0];
break;
} case RegexExpr::Kind::Integer: {
auto* e = reinterpret_cast<IntegerRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__tmp = fromString(e->value.str);
auto tmp = std::move(__tmp__tmp);
({
if (!(isChar(tmp))) {
std::cerr << R"ASSERT(Assert failed: isChar(tmp), e->value, " out of range [0, 255]"
)ASSERT";
exit(-1);
}
});
return tmp;
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: e, "is not a single char"
)ASSERT";
exit(-1);
});
}
}
}
}
NodePair* lower(ASTContext* ast, LetContext* ctx, RegexExpr* e) {
{
auto __tmp_switch_name = e;
switch (e->getKind()) {
case RegexExpr::Kind::String: {
auto* e = reinterpret_cast<StringRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__n = ({
auto* self = ast->New<Node>();
self;
});
auto n = std::move(__tmp__n);
auto __tmp__pair = ({
auto* self = ast->New<NodePair>();
self->st = n;
self->ed = n;
self;
});
auto pair = std::move(__tmp__pair);
for (auto c : Unescaped(e->value.str)) {
auto __tmp__edge = ({
auto* self = ast->New<UnaryEdge>();
self;
});
auto edge = std::move(__tmp__edge);
edge->match = c;
edge->next = ({
auto* self = ast->New<Node>();
self;
});
pair->ed->edges.push_back(edge);
pair->ed = edge->next;
}
return pair;
break;
} case RegexExpr::Kind::Integer: {
auto* e = reinterpret_cast<IntegerRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = ast->New<NodePair>();
self->st = ({
auto* self = ast->New<Node>();
self;
});
self->ed = ({
auto* self = ast->New<Node>();
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__edge = ({
auto* self = ast->New<UnaryEdge>();
self;
});
auto edge = std::move(__tmp__edge);
pair->st->edges.push_back(edge);
edge->match = lowerToChar(e);
edge->next = pair->ed;
return pair;
break;
} case RegexExpr::Kind::Range: {
auto* e = reinterpret_cast<RangeRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = ast->New<NodePair>();
self->st = ({
auto* self = ast->New<Node>();
self;
});
self->ed = ({
auto* self = ast->New<Node>();
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__edge = ({
auto* self = ast->New<RangeEdge>();
self;
});
auto edge = std::move(__tmp__edge);
pair->st->edges.push_back(edge);
edge->start = lowerToChar(e->st);
edge->end = lowerToChar(e->ed);
edge->next = pair->ed;
return pair;
break;
} case RegexExpr::Kind::Juxta: {
auto* e = reinterpret_cast<JuxtaRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__a = lower(ast, ctx, e->lhs);
auto a = std::move(__tmp__a);
auto __tmp__b = lower(ast, ctx, e->rhs);
auto b = std::move(__tmp__b);
skip(ast, a->ed, b->st);
return ({
auto* self = ast->New<NodePair>();
self->st = a->st;
self->ed = b->ed;
self;
});
break;
} case RegexExpr::Kind::Wrapped: {
auto* e = reinterpret_cast<WrappedRegexExpr*>(__tmp_switch_name);
(void)e;
return lower(ast, ctx, e->value);
break;
} case RegexExpr::Kind::Named: {
auto* e = reinterpret_cast<NamedRegexExpr*>(__tmp_switch_name);
(void)e;
return lower(ast, ctx, ctx->find(e->name.str));
break;
} case RegexExpr::Kind::Alt: {
auto* e = reinterpret_cast<AltRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = ast->New<NodePair>();
self->st = ({
auto* self = ast->New<Node>();
self;
});
self->ed = ({
auto* self = ast->New<Node>();
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__a = lower(ast, ctx, e->lhs);
auto a = std::move(__tmp__a);
auto __tmp__b = lower(ast, ctx, e->rhs);
auto b = std::move(__tmp__b);
skip(ast, pair->st, a->st);
skip(ast, pair->st, b->st);
skip(ast, a->ed, pair->ed);
skip(ast, b->ed, pair->ed);
return pair;
break;
} case RegexExpr::Kind::Star: {
auto* e = reinterpret_cast<StarRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = ast->New<NodePair>();
self->st = ({
auto* self = ast->New<Node>();
self;
});
self->ed = ({
auto* self = ast->New<Node>();
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__a = lower(ast, ctx, e->base);
auto a = std::move(__tmp__a);
skip(ast, a->ed, a->st);
skip(ast, pair->st, a->st);
skip(ast, a->ed, pair->ed);
skip(ast, pair->st, pair->ed);
return pair;
break;
} case RegexExpr::Kind::Plus: {
auto* e = reinterpret_cast<PlusRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__a = lower(ast, ctx, e->base);
auto a = std::move(__tmp__a);
skip(ast, a->ed, a->st);
return a;
}
}
}
}
NFAGraphDecl* rewriteRegexDecl(ASTContext* ast, RegexDecl* decl) {
{
auto __tmp__ctx = ({
auto* self = ast->New<LetContext>();
self;
});
auto ctx = std::move(__tmp__ctx);
return ({
auto* self = ast->New<NFAGraphDecl>();
self->name = decl->name;
self->root = ({
auto* self = ast->New<Node>();
self;
});
for (auto i : decl->items) {
{
auto __tmp_switch_name = i;
switch (i->getKind()) {
case TokenDecl::Kind::Let: {
auto* i = reinterpret_cast<LetTokenDecl*>(__tmp_switch_name);
(void)i;
ctx->declare(i->name.str, i->value);
break;
} case TokenDecl::Kind::Import: {
auto* i = reinterpret_cast<ImportTokenDecl*>(__tmp_switch_name);
(void)i;
({
std::cerr << R"ASSERT(Assert failed: "import not supported right now"
)ASSERT";
exit(-1);
});
break;
} case TokenDecl::Kind::Emit: {
auto* i = reinterpret_cast<EmitTokenDecl*>(__tmp_switch_name);
(void)i;
auto __tmp__nfa = lower(ast, ctx, i->value);
auto nfa = std::move(__tmp__nfa);
skip(ast, self->root, nfa->st);
nfa->ed->edges.push_back(({
auto* self = ast->New<EmitEdge>();
self->name = i->name;
self;
}));
break;
} case TokenDecl::Kind::Ignore: {
auto* i = reinterpret_cast<IgnoreTokenDecl*>(__tmp_switch_name);
(void)i;
auto __tmp__nfa = lower(ast, ctx, i->value);
auto nfa = std::move(__tmp__nfa);
skip(ast, self->root, nfa->st);
nfa->ed->edges.push_back(({
auto* self = ast->New<IgnoreEdge>();
self;
}));
}
}
}
}
self;
});
}
}

}  // namespace lower_regex_to_nfa
