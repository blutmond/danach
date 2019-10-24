namespace lower_regex_to_nfa {

void skip(Node* a, Node* b);
char lowerToChar(RegexExpr* e);
NodePair* lower(LetContext* ctx, RegexExpr* e);
NFAGraphDecl* rewriteRegexDecl(RegexDecl* decl);

void skip(Node* a, Node* b) {
a->edges.push_back(({
auto* self = new SkipToEdge;
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
NodePair* lower(LetContext* ctx, RegexExpr* e) {
{
auto __tmp_switch_name = e;
switch (e->getKind()) {
case RegexExpr::Kind::String: {
auto* e = reinterpret_cast<StringRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__n = ({
auto* self = new Node;
self;
});
auto n = std::move(__tmp__n);
auto __tmp__pair = ({
auto* self = new NodePair;
self->st = n;
self->ed = n;
self;
});
auto pair = std::move(__tmp__pair);
for (auto c : Unescaped(e->value.str)) {
auto __tmp__edge = ({
auto* self = new UnaryEdge;
self;
});
auto edge = std::move(__tmp__edge);
edge->match = c;
edge->next = ({
auto* self = new Node;
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
auto* self = new NodePair;
self->st = ({
auto* self = new Node;
self;
});
self->ed = ({
auto* self = new Node;
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__edge = ({
auto* self = new UnaryEdge;
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
auto* self = new NodePair;
self->st = ({
auto* self = new Node;
self;
});
self->ed = ({
auto* self = new Node;
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__edge = ({
auto* self = new RangeEdge;
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
auto __tmp__a = lower(ctx, e->lhs);
auto a = std::move(__tmp__a);
auto __tmp__b = lower(ctx, e->rhs);
auto b = std::move(__tmp__b);
skip(a->ed, b->st);
return ({
auto* self = new NodePair;
self->st = a->st;
self->ed = b->ed;
self;
});
break;
} case RegexExpr::Kind::Wrapped: {
auto* e = reinterpret_cast<WrappedRegexExpr*>(__tmp_switch_name);
(void)e;
return lower(ctx, e->value);
break;
} case RegexExpr::Kind::Named: {
auto* e = reinterpret_cast<NamedRegexExpr*>(__tmp_switch_name);
(void)e;
return lower(ctx, ctx->find(e->name.str));
break;
} case RegexExpr::Kind::Alt: {
auto* e = reinterpret_cast<AltRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = new NodePair;
self->st = ({
auto* self = new Node;
self;
});
self->ed = ({
auto* self = new Node;
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__a = lower(ctx, e->lhs);
auto a = std::move(__tmp__a);
auto __tmp__b = lower(ctx, e->rhs);
auto b = std::move(__tmp__b);
skip(pair->st, a->st);
skip(pair->st, b->st);
skip(a->ed, pair->ed);
skip(b->ed, pair->ed);
return pair;
break;
} case RegexExpr::Kind::Star: {
auto* e = reinterpret_cast<StarRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__pair = ({
auto* self = new NodePair;
self->st = ({
auto* self = new Node;
self;
});
self->ed = ({
auto* self = new Node;
self;
});
self;
});
auto pair = std::move(__tmp__pair);
auto __tmp__a = lower(ctx, e->base);
auto a = std::move(__tmp__a);
skip(a->ed, a->st);
skip(pair->st, a->st);
skip(a->ed, pair->ed);
skip(pair->st, pair->ed);
return pair;
break;
} case RegexExpr::Kind::Plus: {
auto* e = reinterpret_cast<PlusRegexExpr*>(__tmp_switch_name);
(void)e;
auto __tmp__a = lower(ctx, e->base);
auto a = std::move(__tmp__a);
skip(a->ed, a->st);
return a;
}
}
}
}
NFAGraphDecl* rewriteRegexDecl(RegexDecl* decl) {
{
auto __tmp__ctx = ({
auto* self = new LetContext;
self;
});
auto ctx = std::move(__tmp__ctx);
return ({
auto* self = new NFAGraphDecl;
self->name = decl->name;
self->root = ({
auto* self = new Node;
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
auto __tmp__nfa = lower(ctx, i->value);
auto nfa = std::move(__tmp__nfa);
skip(self->root, nfa->st);
nfa->ed->edges.push_back(({
auto* self = new EmitEdge;
self->name = i->name;
self;
}));
break;
} case TokenDecl::Kind::Ignore: {
auto* i = reinterpret_cast<IgnoreTokenDecl*>(__tmp_switch_name);
(void)i;
auto __tmp__nfa = lower(ctx, i->value);
auto nfa = std::move(__tmp__nfa);
skip(self->root, nfa->st);
nfa->ed->edges.push_back(({
auto* self = new IgnoreEdge;
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
