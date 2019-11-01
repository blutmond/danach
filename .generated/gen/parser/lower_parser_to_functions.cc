namespace production_spec {

void doModuleTypeCheck(ModuleContext* globals, Module* m);
string_view getTokenizerName(Module* m);
void DebugPrintType(TypeDeclExpr* t);
void DebugPrintExpr(PatternExpr* e);
void DebugPrintStmt(PatternStmt* s);

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

}  // namespace production_spec
