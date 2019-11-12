namespace production_spec {

void emitNewType(std::ostream& stream, TypeDeclExpr* t);
void emitTypeExpr(std::ostream& stream, TypeDeclExpr* t);
void emitStructBody(std::ostream& stream, ProductTypeDeclExpr* t);
void ImplicitDumpTypes(std::ostream& stream, Module* m);

void emitNewType(std::ostream& stream, TypeDeclExpr* t) {
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
stream << ("new ");
stream << (t->name.str);
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
stream << ("new ");
stream << (t->name.str);
stream << (sub_t->name.str);
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
void emitTypeExpr(std::ostream& stream, TypeDeclExpr* t) {
{
auto __tmp_switch_name = t;
switch (t->getKind()) {
case TypeDeclExpr::Kind::Parametric: {
auto* t = reinterpret_cast<ParametricTypeDeclExpr*>(__tmp_switch_name);
(void)t;
emitTypeExpr(stream, t->base);
stream << ("<");
auto __tmp__notfirst = false;
auto notfirst = std::move(__tmp__notfirst);
for (auto param : t->params) {
if (notfirst) {
stream << (", ");
}
notfirst = true;
emitTypeExpr(stream, param);
}
stream << (">");
break;
} case TypeDeclExpr::Kind::Named: {
auto* t = reinterpret_cast<NamedTypeDeclExpr*>(__tmp_switch_name);
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
stream << (t->name.str);
stream << (sub_t->name.str);
stream << ("*");
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
void emitStructBody(std::ostream& stream, ProductTypeDeclExpr* t) {
for (auto subdecl : t->decls) {
stream << ("  ");
emitTypeExpr(stream, subdecl->type);
stream << (" ");
stream << (subdecl->name.str);
stream << (";\n");
}
}
void ImplicitDumpTypes(std::ostream& stream, Module* m) {
stream << ("namespace ");
stream << (m->mod_name.str);
stream << ("{\n");
for (auto decl : m->decls) {
{
auto __tmp_switch_name = decl;
switch (decl->getKind()) {
case Decl::Kind::Type: {
auto* decl = reinterpret_cast<TypeDecl*>(__tmp_switch_name);
(void)decl;
stream << ("struct ");
stream << (decl->name.str);
stream << (";\n");
auto __tmp__type = decl->type;
auto type = std::move(__tmp__type);
{
auto __tmp_switch_name = type;
switch (type->getKind()) {
case TypeDeclExpr::Kind::Sum: {
auto* type = reinterpret_cast<SumTypeDeclExpr*>(__tmp_switch_name);
(void)type;
for (auto subdecl : type->decls) {
stream << ("struct ");
stream << (subdecl->name.str);
stream << (decl->name.str);
stream << (";\n");
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
stream << ("\nstruct ");
stream << (decl->name.str);
stream << (" {\n");
stream << ("  enum class Kind {\n   ");
for (auto subdecl : type->decls) {
stream << (" ");
stream << (subdecl->name.str);
stream << (",");
}
stream << ("\n  };\n");
stream << ("  ");
stream << (decl->name.str);
stream << ("(Kind kind) : kind_(kind) {}\n");
stream << (" Kind getKind() { return kind_; }\n");
stream << (" private:\n");
stream << ("  Kind kind_;\n");
stream << ("};\n");
for (auto subdecl : type->decls) {
auto __tmp__subt = subdecl->type;
auto subt = std::move(__tmp__subt);
{
auto __tmp_switch_name = subt;
switch (subt->getKind()) {
case TypeDeclExpr::Kind::Product: {
auto* subt = reinterpret_cast<ProductTypeDeclExpr*>(__tmp_switch_name);
(void)subt;
stream << ("\nstruct ");
stream << (subdecl->name.str);
stream << (decl->name.str);
stream << (": public ");
stream << (decl->name.str);
stream << (" {\n");
stream << ("  ");
stream << (subdecl->name.str);
stream << (decl->name.str);
stream << ("()");
stream << (" : ");
stream << (decl->name.str);
stream << ("(Kind::");
stream << (subdecl->name.str);
stream << (") {}\n");
emitStructBody(stream, subt);
stream << ("};\n");
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
stream << ("\nstruct ");
stream << (decl->name.str);
stream << (" {\n");
emitStructBody(stream, type);
stream << ("};\n");
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
stream << ("}  // namespace ");
stream << (m->mod_name.str);
stream << ("\n");
}

}  // namespace production_spec
