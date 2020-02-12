namespace parser_spec {

void visitSuccessors(ASTContext* ast, DFAMappingContext* ctx, Node* base, std::vector<Node*> states);
Node* toDFA(ASTContext* ast, Node* root);

void visitSuccessors(ASTContext* ast, DFAMappingContext* ctx, Node* base, std::vector<Node*> states) {
auto __tmp__c_ctx = ({
auto* self = ast->New<EdgeSetContext>();
self;
});
auto c_ctx = std::move(__tmp__c_ctx);
for (auto state : states) {
c_ctx->AddWork(state);
}
while (true) {
if (c_ctx->work_list.empty()) {
break;
}
auto __tmp__node = c_ctx->work_list.back();
auto node = std::move(__tmp__node);
c_ctx->work_list.pop_back();
for (auto e : node->edges) {
{
auto __tmp_switch_name = e;
switch (e->getKind()) {
case Edge::Kind::Range: {
auto* e = reinterpret_cast<RangeEdge*>(__tmp_switch_name);
(void)e;
c_ctx->add_edge(e->start, e->end, e->next);
break;
} case Edge::Kind::Unary: {
auto* e = reinterpret_cast<UnaryEdge*>(__tmp_switch_name);
(void)e;
c_ctx->add_edge(e->match, e->match, e->next);
break;
} case Edge::Kind::SkipTo: {
auto* e = reinterpret_cast<SkipToEdge*>(__tmp_switch_name);
(void)e;
c_ctx->AddWork(e->next);
break;
} case Edge::Kind::Emit: {
auto* e = reinterpret_cast<EmitEdge*>(__tmp_switch_name);
(void)e;
c_ctx->SetEmitOrIgnore(e);
break;
} case Edge::Kind::Ignore: {
auto* e = reinterpret_cast<IgnoreEdge*>(__tmp_switch_name);
(void)e;
c_ctx->SetEmitOrIgnore(e);
break;
} case Edge::Kind::Unexpected: {
auto* e = reinterpret_cast<UnexpectedEdge*>(__tmp_switch_name);
(void)e;
({
std::cerr << R"ASSERT(Assert failed: e
)ASSERT";
exit(-1);
});
}
}
}
}
}
for (auto edge : c_ctx->edges) {
auto __tmp__next = ctx->getNode(Canonicalize(edge.next));
auto next = std::move(__tmp__next);
if (edge.st == edge.ed) {
base->edges.push_back(({
auto* self = ast->New<UnaryEdge>();
self->next = next;
self->match = edge.ed;
self;
}));
} else {
base->edges.push_back(({
auto* self = ast->New<RangeEdge>();
self->next = next;
self->start = edge.st;
self->end = edge.ed;
self;
}));
}
}
if (c_ctx->emit_or_ignore) {
base->edges.push_back(c_ctx->emit_or_ignore);
} else {
base->edges.push_back(({
auto* self = ast->New<UnexpectedEdge>();
self;
}));
}
}
Node* toDFA(ASTContext* ast, Node* root) {
auto __tmp__ctx = ({
auto* self = ast->New<DFAMappingContext>();
self->ctx = ast;
self;
});
auto ctx = std::move(__tmp__ctx);
std::vector<Node*> states;states.push_back(root);
auto __tmp__result = ctx->getNode(states);
auto result = std::move(__tmp__result);
while (true) {
if (ctx->work_list.empty()) {
break;
}
auto __tmp__item = ctx->work_list.back();
auto item = std::move(__tmp__item);
ctx->work_list.pop_back();
visitSuccessors(ast, ctx, item.first, item.second);
}
return result;
}

}  // namespace parser_spec
