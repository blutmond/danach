namespace parser_spec {

std::string Unescaped(string_view data) {
  std::string out;
  // TODO: This is bad (unsafe)
  data.remove_prefix(1);
  data.remove_suffix(1);
  size_t pos = data.find('\\');
  while (pos != string_view::npos) {
    out.append(data.data(), pos);
    data.remove_prefix(pos);
    data.remove_prefix(1);
    if (data[0] == 'n') {
      out.append(1, '\n');
      data.remove_prefix(1);
    } else if (data[0] == '\\') {
      out.append(1, '\\');
      data.remove_prefix(1);
    }
    pos = data.find('\\');
  }
  out.append(data.data(), data.size());
  return out;
}

int fromString(string_view data) {
  int v = 0;
  for (char c : data) {
    v = v * 10 + (c - '0');
  }
  return v;
}

std::vector<Node*> Canonicalize(std::vector<Node*> states) {
  std::vector<Node*> nstates;
  while (!states.empty()) {
    auto* node = states.back();
    states.pop_back();
    bool isSkipOnly = true;
    for (auto* edge : node->edges) {
      if (edge->getKind() != Edge::Kind::SkipTo) isSkipOnly = false;
    }
    if (!isSkipOnly) nstates.push_back(node);
    for (auto* edge : node->edges) {
      if (edge->getKind() == Edge::Kind::SkipTo) {
        states.push_back(reinterpret_cast<SkipToEdge*>(edge)->next); 
      }
    }
  }
  std::sort(nstates.begin(), nstates.end());
  nstates.erase(std::unique(nstates.begin(), nstates.end()), nstates.end());
  return nstates;
}

struct EdgeSetContext {
  struct Entry {
    char st;
    char ed;
    std::vector<Node*> next;
    bool operator<(const Entry& other) const {
      return st < other.st;
    }
    bool operator<(char c) const {
      return ed < c;
    }
  };
  std::set<Entry, std::less<>> edges;
  Edge* emit_or_ignore = nullptr;

  void add_edge(char st, char ed, Node* n) {
    auto it = edges.lower_bound(st);
    if (it != edges.end()) {
      if (it->st <= ed) {
        if (it->st == st && it->ed == ed) {
          auto dup = *it;
          dup.next.push_back(n);
          edges.erase(it);
          edges.insert(dup);
          return;
        } else {
          fprintf(stderr, "(%d -> %d) clashes with: (%d -> %d);\n",  st, ed, it->st, it->ed);
          exit(-1);
        }
      }
    }
    edges.insert(Entry {st, ed, {n}});
  }
  void SetEmitOrIgnore(Edge* edge) {
    if (emit_or_ignore) {
      fprintf(stderr, "Duplicate Emit/Ignore");
      exit(-1);
    }
    emit_or_ignore = edge;
  }

  std::set<Node*> visited;
  std::vector<Node*> work_list;
  
  void AddWork(Node* node) {
    if (visited.find(node) == visited.end()) {
      visited.insert(node);
      work_list.push_back(node);
    }
  }
};

struct DFAMappingContext {
  std::map<std::vector<Node*>, Node*> state_map;
  std::vector<std::pair<Node*, std::vector<Node*>>> work_list;

  Node* getNode(std::vector<Node*> states) {
    std::sort(states.begin(), states.end());
    auto it = state_map.find(states);
    if (it == state_map.end()) {
      auto* node = new Node;
      work_list.push_back({node, states});
      return state_map[states] = node;
    }
    return it->second;
  }
};

} // namespace parser_spec

namespace lower_regex_to_nfa {
bool isChar(int v) { return v >= 0 && v <= 255; }

using namespace parser_spec;

class LetContext {
 public:
  void declare(string_view name, RegexExpr* decl) {
    decls[name] = decl;
  }
  RegexExpr* find(string_view name) {
    auto* decl = decls[name];
    if (!decl) {
      std::cerr << "Could not find: " << name << "\n";
      exit(-1);
    }
    return decl;
  }
  std::unordered_map<string_view, RegexExpr*> decls;
};

} // namespace lower_regex_to_nfa

#include "gen/parser/lower_nfa_to_dfa.cc"
#include "gen/parser/lower_to_nfa.cc"

namespace parser_spec {
struct LoweringToNFA {
  NFAGraphDecl* rewriteDecl(RegexDecl* decl) {
    auto* result = lower_regex_to_nfa::rewriteRegexDecl(decl);
    result->root = toDFA(result->root);
    return result;
  }
  Decl* visit(Decl* decl) {
    switch (decl->getKind()) {
    case Decl::Kind::Regex: {
      return rewriteDecl(reinterpret_cast<RegexDecl*>(decl));
    }
    default: return decl;
    }
  }
  Module* visit(Module* m) {
    auto* result = new Module;
    result->mod_name = m->mod_name;
    for (auto* decl : m->decls) {
      result->decls.push_back(visit(decl));
    }
    return result;
  }
};

}  // namespace parser_spec

