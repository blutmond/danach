namespace production_spec {

void emitNewType(TypeDeclExpr* t);
void emitTypeExpr(TypeDeclExpr* t);
void emitStructBody(ProductTypeDeclExpr* t);
void ImplicitDumpTypes(Module* m);

void emitNewType(TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)t;
if (t->name.str == "Token") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "Array") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "Map") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "String") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "char") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "int") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
if (t->name.str == "bool") {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
return;
}
std::cout << ("new ");
std::cout << (t->name.str);
break;
} case TypeDeclExpr::Kind::Colon: {
auto* t = reinterpret_cast<ColonTypeDeclExpr*>(__tmp_switch_name);
(void)t;
auto __tmp__sub_t = t->base;
auto sub_t = std::move(__tmp__sub_t);
{
auto __tmp_switch_name = sub_t;
switch (sub_t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* sub_t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)sub_t;
std::cout << ("new ");
std::cout << (t->name.str);
std::cout << (sub_t->name.str);
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Cannot new: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
void emitTypeExpr(TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Parametric: {
auto* t = reinterpret_cast<ParametricTypeDeclExpr*>(__tmp_switch_name);
(void)t;
emitTypeExpr(t->base);
std::cout << ("<");
auto __tmp__notfirst = false;
auto notfirst = std::move(__tmp__notfirst);
for (auto param : t->params) {
if (notfirst) {
std::cout << (", ");
}
notfirst = true;
emitTypeExpr(param);
}
std::cout << (">");
break;
} case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
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
break;
} case TypeDeclExpr::Kind::Colon: {
auto* t = reinterpret_cast<ColonTypeDeclExpr*>(__tmp_switch_name);
(void)t;
auto __tmp__sub_t = t->base;
auto sub_t = std::move(__tmp__sub_t);
{
auto __tmp_switch_name = sub_t;
switch (sub_t->getKind()) {
case TypeDeclExpr::Kind::Named: {
auto* sub_t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
(void)sub_t;
std::cout << (t->name.str);
std::cout << (sub_t->name.str);
std::cout << ("*");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
void emitStructBody(ProductTypeDeclExpr* t) {
for (auto subdecl : t->decls) {
std::cout << ("  ");
emitTypeExpr(subdecl->type);
std::cout << (" ");
std::cout << (subdecl->name.str);
std::cout << (";\n");
}
}
void ImplicitDumpTypes(Module* m) {
std::cout << ("namespace ");
std::cout << (m->mod_name.str);
std::cout << ("{\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
std::cout << ("struct ");
std::cout << (decl->name.str);
std::cout << (";\n");
auto __tmp__type = decl->type;
auto type = std::move(__tmp__type);
{
auto __tmp_switch_name = type;
switch (type->getKind()) {
case TypeDeclExpr::Kind::Sum: {
auto* type = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)type;
for (auto subdecl : type->decls) {
std::cout << ("struct ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << (";\n");
}
break;
} default: {
}
}
}
break;
} default: {
}
}
}
}
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
auto __tmp__type = decl->type;
auto type = std::move(__tmp__type);
{
auto __tmp_switch_name = type;
switch (type->getKind()) {
case TypeDeclExpr::Kind::Sum: {
auto* type = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)type;
std::cout << ("\nstruct ");
std::cout << (decl->name.str);
std::cout << (" {\n");
std::cout << ("  enum class Kind {\n   ");
for (auto subdecl : type->decls) {
std::cout << (" ");
std::cout << (subdecl->name.str);
std::cout << (",");
}
std::cout << ("\n  };\n");
std::cout << ("  ");
std::cout << (decl->name.str);
std::cout << ("(Kind kind) : kind_(kind) {}\n");
std::cout << (" Kind getKind() { return kind_; }\n");
std::cout << (" private:\n");
std::cout << ("  Kind kind_;\n");
std::cout << ("};\n");
for (auto subdecl : type->decls) {
auto __tmp__subt = subdecl->type;
auto subt = std::move(__tmp__subt);
{
auto __tmp_switch_name = subt;
switch (subt->getKind()) {
case TypeDeclExpr::Kind::Product: {
auto* subt = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)subt;
std::cout << ("\nstruct ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << (": public ");
std::cout << (decl->name.str);
std::cout << (" {\n");
std::cout << ("  ");
std::cout << (subdecl->name.str);
std::cout << (decl->name.str);
std::cout << ("()");
std::cout << (" : ");
std::cout << (decl->name.str);
std::cout << ("(Kind::");
std::cout << (subdecl->name.str);
std::cout << (") {}\n");
emitStructBody(subt);
std::cout << ("};\n");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
}
break;
} case TypeDeclExpr::Kind::Product: {
auto* type = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)type;
std::cout << ("\nstruct ");
std::cout << (decl->name.str);
std::cout << (" {\n");
emitStructBody(type);
std::cout << ("};\n");
break;
} default: {
({
std::cerr << R"ASSERT(Assert failed: "Do not understand: ", t
)ASSERT";
exit(-1);
});
}
}
}
break;
} default: {
}
}
}
}
std::cout << ("}  // namespace ");
std::cout << (m->mod_name.str);
std::cout << ("\n");
}

}  // namespace production_spec
