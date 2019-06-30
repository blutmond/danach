#include <assert.h>
#include <functional>
#include <unordered_map>
#include <sstream>
#include "tokens/tokenizer_helper.cc"
#include "gen/llvm_graph/spec_parser.cc"
#include "gen/llvm_graph/graph_parser.cc"

void UnescapedDump(std::ostream& stream, string_view data) {
  // TODO: This is bad.
  data.remove_prefix(1);
  data.remove_suffix(1);
  size_t pos = data.find('\\');
  while (pos != string_view::npos) {
    stream << data.substr(0, pos);
    data.remove_prefix(pos);
    data.remove_prefix(1);
    if (data[0] == 'n') {
      stream << "\n";
      data.remove_prefix(1);
    }
    pos = data.find('\\');
  }
  stream << data;
}

std::string Unescape(string_view data) {
  std::stringstream ss;
  UnescapedDump(ss, data);
  return ss.str();
}

namespace llvm_graph {

class Node {
 public:
  virtual ~Node() {}
  enum class Kind { Call, String, Int, Array };
  virtual Kind getKind() = 0;
};

struct CallNode : public Node {
  Kind getKind() override { return Kind::Call; }
  tok::Token name;
  std::vector<Node*> nodes;
};

struct StringNode : public Node {
  Kind getKind() override { return Kind::String; }
  tok::Token value;
};
struct IntNode : public Node {
  Kind getKind() override { return Kind::Int; }
  tok::Token value;
};
struct ArrayNode : public Node {
  Kind getKind() override { return Kind::Array; }
  std::vector<Node*> nodes;
};

}  // namespace llvm_graph

namespace llvm_graph_spec {
using llvm_graph::Node;
using llvm_graph::CallNode;
using llvm_graph::IntNode;
using llvm_graph::StringNode;
using llvm_graph::ArrayNode;

class GraphSpecIndex {
 public:
  std::unordered_map<string_view, CtorDecl*> ctors;

  CtorDecl* GetName(string_view name) const {
    auto it = ctors.find(name);
    if (it == ctors.end()) {
      std::cerr << "Could not find name: " << name << "\n";
      exit(-1);
    }
    return it->second;
  }
};

GraphSpecIndex LoadSpec(const char* data) {
  Tokenizer tokens(data);
  auto* m = parser::DoParse(tokens);

  GraphSpecIndex index;

  for (auto* _decl: m->decls) {
    if (_decl->getKind() == Decl::Kind::Ctor) {
      auto* decl = reinterpret_cast<CtorDecl*>(_decl);
      index.ctors[decl->name.str] = decl;
    }
  }

  return index;
}

class CppValue {
 public:
  enum class Kind { String, Int, Array, Inline, Call, Hole, WithImplicits };
  explicit CppValue(Kind kind) : kind_(kind) {}

  Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

template <CppValue::Kind kind>
class CppValueT : public CppValue {
 public:
  CppValueT() : CppValue(kind) {}
};

struct StringCppValue : CppValueT<CppValue::Kind::String> {
  string_view value;
};
struct IntCppValue : CppValueT<CppValue::Kind::Int> {
  string_view value;
};
struct InlineCppValue : CppValueT<CppValue::Kind::Inline> {
  std::string value;
};
struct ArrayCppValue : CppValueT<CppValue::Kind::Array> {
  std::vector<CppValue*> values;
};
struct CallCppValue : CppValueT<CppValue::Kind::Call> {
  CppValue* implicit_control_flow_dep = nullptr;
  CppValue* base;
  std::vector<CppValue*> params;
};
struct HoleCppValue : CppValueT<CppValue::Kind::Hole> {
  CppValue* fill_in = nullptr;
};
struct WithImplicitsCppValue : CppValueT<CppValue::Kind::WithImplicits> {
  CppValue* base = nullptr;
  std::unordered_map<string_view, HoleCppValue*> holes;
};

struct CppEmitter {
  std::ostream& stream;
  const GraphSpecIndex& index;
  explicit CppEmitter(std::ostream& stream, const GraphSpecIndex& index) : stream(stream), index(index) {}

  std::vector<CppValue*> known_values;
  struct EvalState {
    std::unordered_map<string_view, HoleCppValue*> holes;
    CppValue* value = nullptr;
  };
  using HoleSet = std::unordered_map<string_view, HoleCppValue*>;
  std::unordered_map<Node*, EvalState> call_stacks;
  std::unordered_map<Node*, CppValue*> values;

  struct FuncLevelContext {
    CppEmitter* base;
    HoleSet& holes;
    CppValue*& result;
    CppValue* implicit_control_flow_dep = nullptr;

    FuncLevelContext(CppEmitter* base, HoleSet& holes, CppValue*& result)
        : base(base), holes(holes), result(result) {}
  };

  CppValue* LLVM_CTX = ([]() {
    auto* res = new InlineCppValue;
    res->value = "LLVM_CONTEXT";
    return res;
  }());

  struct EvalContext {
    EvalContext* base = nullptr;
    FuncLevelContext* ctx;
    std::unordered_map<string_view, CppValue*> vars;

    CppValue* FindName(string_view name) {
      auto it = vars.find(name);
      if (it == vars.end()) {
        if (base) return base->FindName(name);
        else {
          if (name == "LLVM_CTX") {
            return ctx->base->LLVM_CTX;
          }
          std::cerr << "Named found: " << name << " not found\n";
          exit(-1);
        }
      }
      return it->second;
    }

    CppValue* Eval(Expr* _expr) {
      switch (_expr->getKind()) {
      case Expr::Kind::InlineCpp: {
        auto* res = new InlineCppValue;
        res->value = Unescape(reinterpret_cast<InlineCppExpr*>(_expr)->value.str);
        return res;
      } case Expr::Kind::Named: {
        return FindName(reinterpret_cast<NamedExpr*>(_expr)->name.str);
      } case Expr::Kind::Call: {
        auto* expr = reinterpret_cast<CallExpr*>(_expr);
        std::vector<CppValue*> args;
        for (auto* arg : expr->params) {
          args.push_back(Eval(arg));
        }
        if (expr->base->getKind() == Expr::Kind::Named) {
          return ctx->base->DoInlineCall(reinterpret_cast<NamedExpr*>(expr->base)->name.str,
                                  args);
        }
        auto* res = new CallCppValue;
        ctx->base->known_values.push_back(res);
        res->implicit_control_flow_dep = ctx->implicit_control_flow_dep;
        ctx->implicit_control_flow_dep = res;
        res->params = std::move(args);
        res->base = Eval(expr->base);
        return res;
      } case Expr::Kind::Dot:
        std::cerr << "Dot not allowed\n";
        exit(-1);
      case Expr::Kind::Assign: {
        auto* expr = reinterpret_cast<AssignExpr*>(_expr);
        assert(expr->base->getKind() == Expr::Kind::Dot);
        auto* dot = reinterpret_cast<DotExpr*>(expr->base);
        auto* _base = Eval(dot->base);
        assert(_base->getKind() == CppValue::Kind::WithImplicits);
        auto* base = reinterpret_cast<WithImplicitsCppValue*>(_base);
        auto it = base->holes.find(dot->name.str);
        if (it == base->holes.end()) {
          std::cerr << "No implicit: " << dot->name.str << " ...\n";
          exit(-1);
        }
        return it->second->fill_in = Eval(expr->value);
      }
      }
    }

    bool Eval(Stmt* stmt) {
      switch (stmt->getKind()) {
      case Stmt::Kind::Compound: {
        for (auto* item : reinterpret_cast<CompoundStmt*>(stmt)->stmts) {
          if (Eval(item)) return true;
        }
        return false;
      } case Stmt::Kind::Let: {
        auto* _stmt = reinterpret_cast<LetStmt*>(stmt);
        vars[_stmt->name.str] = Eval(_stmt->expr);
        return false;
      } case Stmt::Kind::For: {
        auto* _stmt = reinterpret_cast<ForStmt*>(stmt);
        auto* _sequence = Eval(_stmt->sequence);
        assert(_sequence->getKind() == CppValue::Kind::Array);
        auto* sequence = reinterpret_cast<ArrayCppValue*>(_sequence);
        for (auto* value : sequence->values) {
          EvalContext tmp_ctx(this);
          tmp_ctx.vars[_stmt->name.str] = value;
          if (tmp_ctx.Eval(_stmt->stmts)) return true;
        }
        return false;
      } case Stmt::Kind::Return: {
        ctx->result = Eval(reinterpret_cast<ReturnStmt*>(stmt)->expr);
        return false;;
      } case Stmt::Kind::Implicit: {
        auto* decl = reinterpret_cast<ImplicitStmt*>(stmt)->decl;
        assert(!base && "Implicit must be at global scope");
        auto* hole = new HoleCppValue;
        ctx->holes[decl->name.str] = hole;
        vars[decl->name.str] = hole;
        return false;
      } case Stmt::Kind::Async: {
        return Eval(reinterpret_cast<AsyncStmt*>(stmt)->stmts);
      } case Stmt::Kind::SideEffect: {
        Eval(reinterpret_cast<SideEffectStmt*>(stmt)->value);
        return false;
      }
      }
    }

    EvalContext(CppEmitter* base, HoleSet& holes, CppValue*& result)
        : ctx(new FuncLevelContext(base, holes, result)) {}
    explicit EvalContext(EvalContext* _base) {
      base = _base;
      ctx = base->ctx;
    }
    void Eval(CtorDecl* decl, CallNode* call) {
      assert(decl->args.size() == call->nodes.size() && "Problem");
      for (size_t i = 0; i < decl->args.size(); ++i) {
        vars[decl->args[i]->name.str] = ctx->base->Eval(call->nodes[i]);
      }
      Eval(decl->stmts);
    }
    void Eval(CtorDecl* decl, const std::vector<CppValue*>& args) {
      assert(decl->args.size() == args.size() && "Problem");
      for (size_t i = 0; i < decl->args.size(); ++i) {
        vars[decl->args[i]->name.str] = args[i];
      }
      Eval(decl->stmts);
    }
  };

  CppValue* DoInlineCall(string_view name, const std::vector<CppValue*>& args) {
    auto* state = new WithImplicitsCppValue;
    auto* v = index.GetName(name);
    (new EvalContext(this, state->holes, state->base))->Eval(v, args);
    return state;
  }

  CppValue* Eval(Node* node) {
    auto it = values.find(node);
    if (it != values.end()) {
      return it->second;
    }
    CppValue*& result = values[node];
    EvalWithoutCache(node, result);
    return result;
  }
  void EvalWithoutCache(Node* node, CppValue*& ret) {
    switch (node->getKind()) {
    case Node::Kind::Call: {
      auto* call = reinterpret_cast<CallNode*>(node);
      auto* state = new WithImplicitsCppValue;
      ret = state;
      auto* v = index.GetName(call->name.str);
      (new EvalContext(this, state->holes, state->base))->Eval(v, call);
      return;
    } case Node::Kind::Int: {
      auto* result = new IntCppValue;
      result->value = reinterpret_cast<IntNode*>(node)->value.str;
      ret = result;
      return;
    } case Node::Kind::String: {
      auto* result = new StringCppValue;
      result->value = reinterpret_cast<StringNode*>(node)->value.str;
      ret = result;
      return;
    } case Node::Kind::Array: {
      auto* result = new ArrayCppValue;
      ret = result;
      for (auto* node : reinterpret_cast<ArrayNode*>(node)->nodes) {
        result->values.push_back(Eval(node));
      }
      return;
    }
    }
  }

  std::vector<CppValue*> emit_order;
  std::unordered_map<CppValue*, int> names;
  std::unordered_map<CppValue*, int> ref_count;

  void OrderDeps(const std::vector<CppValue*>& arr) {
    for (auto* child : arr) { OrderDeps(child); }
  }
  void OrderDeps(CppValue* value) {
    assert(value);
    int& count = ref_count[value];
    if (count == 0) {
      OrderDepsWithoutCache(value);
    }
    ++count;
  }
  void OrderDepsWithoutCache(CppValue* _value) {
    switch (_value->getKind()) {
    case CppValue::Kind::Array: {
      // std::cerr << "Array\n";
      OrderDeps(reinterpret_cast<ArrayCppValue*>(_value)->values);
      break;
    } case CppValue::Kind::Call: {
      // std::cerr << "Call\n";
      auto* value = reinterpret_cast<CallCppValue*>(_value);
      OrderDeps(value->params);
      OrderDeps(value->base);
      if (value->implicit_control_flow_dep) {
        OrderDeps(value->implicit_control_flow_dep);
      }
      emit_order.push_back(value);
      break;
    } case CppValue::Kind::Hole: {
      // std::cerr << "Hole\n";
      OrderDeps(reinterpret_cast<HoleCppValue*>(_value)->fill_in);
      break;
    } case CppValue::Kind::WithImplicits: {
      // std::cerr << "WithImplicits\n";
      OrderDeps(reinterpret_cast<WithImplicitsCppValue*>(_value)->base);
      break;
    }
    case CppValue::Kind::String:
    case CppValue::Kind::Int:
    case CppValue::Kind::Inline:
        break;
    }
  }
  
  void EmitValue(const std::vector<CppValue*>& arr) {
    int i = 0;
    for (auto* child : arr) {
      if (i != 0) stream << ", ";
      EmitRef(child);
      ++i;
    }
  }

  void EmitValue(CppValue* _value) {
    switch (_value->getKind()) {
    case CppValue::Kind::Array: {
      stream << "{";
      EmitValue(reinterpret_cast<ArrayCppValue*>(_value)->values);
      stream << "}";
      break;
    } case CppValue::Kind::Call: {
      auto* value = reinterpret_cast<CallCppValue*>(_value);
      EmitRef(value->base);
      stream << "(";
      EmitValue(value->params);
      stream << ")";
      break;
    } case CppValue::Kind::Hole: {
      EmitRef(reinterpret_cast<HoleCppValue*>(_value)->fill_in);
      break;
    } case CppValue::Kind::WithImplicits: {
      EmitRef(reinterpret_cast<WithImplicitsCppValue*>(_value)->base);
      break;
    } case CppValue::Kind::String: {
      stream << reinterpret_cast<StringCppValue*>(_value)->value;
      break;
    } case CppValue::Kind::Int: {
      stream << reinterpret_cast<IntCppValue*>(_value)->value;
      break;
    } case CppValue::Kind::Inline: {
      stream << reinterpret_cast<InlineCppValue*>(_value)->value;
      break;
    }
    }
  }

  void EmitRef(CppValue* v) {
    auto it = names.find(v);
    if (it != names.end()) {
      stream << "_" << it->second;
      return;
    }
    EmitValue(v);
  }

  void Emit(Node* root) {
    auto* result = Eval(root);
    OrderDeps(result);
    for (auto* node : known_values) {
      OrderDeps(node);
    }

    stream << "llvm::Module* makeThing(llvm::LLVMContext& LLVM_CONTEXT) {\n";
    int i = 0;
    for (auto* v : emit_order) {
      stream << "  auto _" << i << " = ";
      EmitValue(v);
      names[v] = i;
      stream << ";\n";
      ++i;
    }
    stream << "  return ";
    EmitRef(result);
    stream << ";\n}\n";
  }
};

void EmitEmitterFn(const GraphSpecIndex& index, Node* root) {
  CppEmitter(std::cout, index).Emit(root);
}

}  // namespace llvm_graph_spec

namespace llvm_graph {
class NodeScope;

struct WorkListItem {
  std::vector<Expr*> exprs;
  std::vector<Node*>* out;
  NodeScope* scope;
};

struct GlobalState {
  std::unordered_map<Expr*, Node*> cache;
  std::vector<WorkListItem*> work_list;
};

class NodeScope {
 public:
  NodeScope* parent = nullptr;
  GlobalState* state;
  std::unordered_map<string_view, Expr*> names;

  void registerName(tok::Token tok, Expr* expr) {
    auto it = names.emplace(tok.str, expr);
    if (!it.second) {
      std::cerr << "Duplicate name: " << tok.str << "\n";
      exit(-1);
    }
  }

  ArrayNode* lazyEvalArray(const std::vector<Decl*>& decls) {
    std::vector<Expr*> exprs;
    for (auto* _decl : decls) {
      switch (_decl->getKind()) {
      case Decl::Kind::Let: {
        auto* decl = reinterpret_cast<LetDecl*>(_decl);
        maybe_eval(decl->expr);
        registerName(decl->name, decl->expr);
        break;
      } case Decl::Kind::Value: {
        auto* decl = reinterpret_cast<ValueDecl*>(_decl);
        maybe_eval(decl->expr);
        exprs.push_back(decl->expr);
        break;
      }
      }
    }
    auto* result = new ArrayNode;
    lazyEvalArrayBase(exprs, &result->nodes);
    return result;
  }
  ArrayNode* lazyEvalArray(const std::vector<Expr*>& exprs) {
    auto* result = new ArrayNode;
    for (auto* expr : exprs) {
      maybe_eval(expr);
    }
    lazyEvalArrayBase(exprs, &result->nodes);
    return result;
  }
  void lazyEvalArrayBase(const std::vector<Expr*>& exprs, std::vector<Node*>* out) {
    auto* item = new WorkListItem;
    item->exprs = exprs;
    item->out = out;
    item->scope = this;
    state->work_list.push_back(item);
  }

  Node* maybe_eval(Expr* _expr) {
    auto* result = maybe_eval_uncached(_expr);
    if (result) {
      state->cache[_expr] = result;
    }
    return result;
  }
  Node* maybe_eval_uncached(Expr* _expr) {
    switch (_expr->getKind()) {
    case Expr::Kind::ScopeBlock: {
      auto* expr = reinterpret_cast<ScopeBlockExpr*>(_expr);
      auto* new_scope = new NodeScope;
      new_scope->state = state;
      new_scope->parent = this;
      return new_scope->lazyEvalArray(expr->decls);
    } case Expr::Kind::Str: {
      auto* expr = reinterpret_cast<StrExpr*>(_expr);
      auto* result = new StringNode;
      result->value = expr->value;
      return result;
    } case Expr::Kind::Number: {
      auto* expr = reinterpret_cast<NumberExpr*>(_expr);
      auto* result = new IntNode;
      result->value = expr->value;
      return result;
    } case Expr::Kind::Named: {
      return nullptr;
    } case Expr::Kind::Array: {
      auto* expr = reinterpret_cast<ArrayExpr*>(_expr);
      return lazyEvalArray(expr->elements);
    } case Expr::Kind::Block: {
      auto* expr = reinterpret_cast<BlockExpr*>(_expr);
      return lazyEvalArray(expr->decls);
    } case Expr::Kind::Call: {
      // TODO: lookup constructor name in global context...
      auto* expr = reinterpret_cast<CallExpr*>(_expr);
      auto* result = new CallNode; 
      result->name = GetName(expr->base); 
      for (auto* expr : expr->params) { maybe_eval(expr); }
      lazyEvalArrayBase(expr->params, &result->nodes);
      return result;
    } case Expr::Kind::Assign: {
      auto* expr = reinterpret_cast<AssignExpr*>(_expr);
      auto name = GetName(expr->base);
      auto* result = maybe_eval(expr->value);
      registerName(name, expr->value);
      return result;
    }
    }
  }
  tok::Token GetName(Expr* expr) {
    assert(expr->getKind() == Expr::Kind::Named);
    return reinterpret_cast<NamedExpr*>(expr)->name;
  }

  Node* eval(Expr* expr) {
    {
      auto it = state->cache.find(expr);
      if (it != state->cache.end()) {
        assert(it->second && "Some sort of cycle...");
        return it->second;
      }
    }
    auto name = GetName(expr);
    auto& tmp = state->cache[expr];
    tmp = nullptr;
    return tmp = FindName(name);
  }

  Node* FindName(tok::Token name) {
    auto it = names.find(name.str);
    if (it == names.end()) {
      if (parent) {
        return parent->FindName(name);
      }
      std::cerr << "could not find: " << name.str << "\n";
      exit(-1);
    }
    return eval(it->second);
  }

  void DoAllGlobalWork() {
    while (!state->work_list.empty()) {
      auto* work = state->work_list.back();
      state->work_list.pop_back();
      for (auto* expr : work->exprs) {
        work->out->push_back(work->scope->eval(expr));
      }
    }
  }

  Node* GetRoot(string_view key = "main") {
    auto it = names.find(key);
    if (it == names.end()) {
      std::cerr << "main does not exist\n";
    }
    Node* result = eval(it->second);
    DoAllGlobalWork();
    return result;
  }

  static Node* EvalRoot(GlobalState* state, const std::vector<Decl*>& decls) {
    auto* result = new NodeScope;
    result->state = state;
    result->lazyEvalArray(decls);
    return result->GetRoot();
  }
};


struct DebugNodeEmit {
  int next_id = 0;
  std::unordered_map<Node*, int> ids;
  std::unordered_map<Node*, std::string> forced_names;
  std::vector<Node*> work_list;
  std::ostream& stream;

  explicit DebugNodeEmit(std::ostream& stream) : stream(stream) {}

  void EmitName(Node* name) {
    auto it = forced_names.find(name);
    if (it != forced_names.end()) {
      stream << it->second;
      return;
    }

    auto it2 = ids.find(name);
    if (it2 != ids.end()) {
      stream << "_" << it2->second;
      return;
    }
    ids[name] = next_id;
    work_list.push_back(name);
    stream << "_" << next_id;
    ++next_id;
  }

  void EmitRef(Node* node) {
    switch (node->getKind()) {
    case Node::Kind::Call:
    case Node::Kind::Array:
      if (num_refs[node] == 1) {
        EmitValue(node);
      } else {
        EmitName(node);
      }
      break;
    case Node::Kind::Int:
    case Node::Kind::String:
      EmitValue(node);
      break;
    }
  }

  void EmitValue(Node* node) {
    switch (node->getKind()) {
    case Node::Kind::Call: {
      auto* _node = reinterpret_cast<CallNode*>(node);
      stream << _node->name.str;
      int i = 0;
      stream << "(";
      for (auto* child_node : _node->nodes) {
        if (i != 0) stream << ", ";
        EmitRef(child_node);
        ++i;
      }
      stream << ")";
      break;
    }
    case Node::Kind::Array: {
      int i = 0;
      stream << "[";
      for (auto* child_node : reinterpret_cast<ArrayNode*>(node)->nodes) {
        if (i != 0) stream << ", ";
        EmitRef(child_node);
        ++i;
      }
      stream << "]";
      break;
    }
    case Node::Kind::Int: {
      stream << reinterpret_cast<IntNode*>(node)->value.str;
      break;
    }
    case Node::Kind::String: {
      stream << reinterpret_cast<StringNode*>(node)->value.str;
      break;
    }
    }
  }

  void EmitRoot(Node* node) {
    forced_names[node] = "main";
    CountRefs(node);
    work_list.push_back(node);
    while (!work_list.empty()) {
      auto* node = work_list.back();
      work_list.pop_back();
      stream << "let ";
      EmitName(node);
      stream << " = ";
      EmitValue(node);
      stream << ";\n";
    }
  }

  std::unordered_map<Node*, int> num_refs;
  void CountRefs(Node* node) {
    auto& refs = num_refs[node];
    ++refs;
    if (refs != 1) return;
    switch (node->getKind()) {
    case Node::Kind::Call:
      for (auto* child_node : reinterpret_cast<CallNode*>(node)->nodes) { CountRefs(child_node); }
      break;
    case Node::Kind::Array:
      for (auto* child_node : reinterpret_cast<ArrayNode*>(node)->nodes) { CountRefs(child_node); }
      break;
    case Node::Kind::Int:
    case Node::Kind::String:
      break;
    }
  }
};

using llvm_graph_spec::GraphSpecIndex;
void BasicTest(Module* m, const GraphSpecIndex& index) {
  auto* state = new GlobalState;
  Node* node = NodeScope::EvalRoot(state, m->decls);
  // DebugNodeEmit(std::cout).EmitRoot(node);
  EmitEmitterFn(index, node);
}

}  // namespace llvm_graph

int main(int argc, char **argv){
  using namespace llvm_graph;
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }
  auto spec = LoadFile("src/llvm_graph/llvm_spec");
  auto index = llvm_graph_spec::LoadSpec(spec.c_str());

  auto contents = LoadFile(argv[1]);
  Tokenizer tokens(contents.c_str());
  auto* m = parser::DoParse(tokens);
  BasicTest(m, index);

}
