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

struct NodePair {
  Node* st;
  Node* ed;
};

void MakeSkip(Node* a, Node* b) {
  auto* edge = new SkipToEdge;
  edge->next = b;
  a->edges.push_back(edge);
}

struct DumpNFA {
  int id_assign = 0;
  std::vector<std::pair<Node*, int>> work_list;
  std::map<Node*, int> assigned;

  int getId(Node* node) {
    auto it = assigned.find(node);
    if (it != assigned.end()) return it->second;
    int id = id_assign;
    ++id_assign;
    assigned[node] = id;
    work_list.push_back({node, id});
    return id;
  }
  void prettyEmitChar(char c) {
    if (c == '\\') {
      std::cout << "'\\\\'";
      return;
    }
    if (std::isprint(c)) {
      std::cout << "\'" << c << "\'";
      return;
    } else {
      std::cout << (int)c;
    }
  }
  void dump(Node* node) {
    getId(node);
    while (!work_list.empty()) {
      auto* node = work_list.back().first;
      std::cout << "bb" << (100 + work_list.back().second) << ":\n";
      work_list.pop_back();
      for (auto* edge: node->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range: {
          auto* tmp = reinterpret_cast<RangeEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  [";
          prettyEmitChar(*tmp->start);
          std::cout << ", ";
          prettyEmitChar(*tmp->end);
          std::cout << "] -> bb" << (100 + id) << ";\n";
          break;
        } case Edge::Kind::Unary: {
          auto* tmp = reinterpret_cast<UnaryEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  ";
          prettyEmitChar(*tmp->match);
          std::cout << " -> bb" << (100 + id) << ";\n";
          break;
        } case Edge::Kind::SkipTo: {
          auto* tmp = reinterpret_cast<SkipToEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << " skip_to bb" << (100 + id) << ";\n";
          break;
        } case Edge::Kind::Emit:
          std::cout << "  emit " << reinterpret_cast<EmitEdge*>(edge)->name.str << ";\n";
          break;
        case Edge::Kind::Ignore:
          std::cout << "  ignore;\n";
          break;
        case Edge::Kind::Unexpected:
          std::cout << "  unexpected();\n";
          break;
        }
      }
    }
  }
};

struct GotoDFAEmitter {
  int id_assign = 0;
  std::vector<std::pair<Node*, int>> work_list;
  std::map<Node*, int> assigned;

  int getId(Node* node) {
    auto it = assigned.find(node);
    if (it != assigned.end()) return it->second;
    int id = id_assign;
    ++id_assign;
    assigned[node] = id;
    work_list.push_back({node, id});
    return id;
  }
  void prettyEmitChar(char c) {
    if (c == '\\') {
      std::cout << "'\\\\'";
      return;
    }
    if (std::isprint(c)) {
      std::cout << "'" << c << "'";
      return;
    } else {
      std::cout << (int)c;
    }
  }
  void emitRoot(Node* node) {
    getId(node);
    emitAll();
  }
  void emitAll() {
    while (!work_list.empty()) {
      auto* node = work_list.back().first;
      std::cout << "bb" << work_list.back().second << ":\n";
      work_list.pop_back();
      for (auto* edge: node->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range: {
          auto* tmp = reinterpret_cast<RangeEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  if ";
          prettyEmitChar(*tmp->start);
          std::cout << " - ";
          prettyEmitChar(*tmp->end);
          std::cout << " goto bb" << id << "\n";
          break;
        } case Edge::Kind::Unary: {
          auto* tmp = reinterpret_cast<UnaryEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  if ";
          prettyEmitChar(*tmp->match);
          std::cout << " goto bb" << id << "\n";
          break;
        } case Edge::Kind::SkipTo:
          fprintf(stderr, "Un-eliminated skip-to\n");
          exit(-1);
          break;
        case Edge::Kind::Emit:
          std::cout << "  return " << reinterpret_cast<EmitEdge*>(edge)->name.str << ";\n";
          break;
        case Edge::Kind::Ignore:
          std::cout << "  goto start;\n";
          break;
        case Edge::Kind::Unexpected:
          std::cout << "  unexpected();\n";
          break;
        }
      }
    }
  }
};

struct LoweringToNFA {
  char GetAsChar(RegexExpr* expr) {
    switch (expr->getKind()) {
    case RegexExpr::Kind::String: {
      auto str = reinterpret_cast<StringRegexExpr*>(expr)->value;
      auto tmp = Unescaped(str.str);
      if (tmp.size() != 1) {
        fprintf(stderr, "Not a single char!\n");
        exit(-1);
      }
      return tmp[0];
    } case RegexExpr::Kind::Integer: {
      auto str = reinterpret_cast<IntegerRegexExpr*>(expr)->value;
      return fromString(str.str);
    } default:
      fprintf(stderr, "Not a char expr!!\n");
      exit(-1);
    }
  }
  NodePair visit(const std::map<string_view, RegexExpr*>& lets, RegexExpr* expr) {
    switch (expr->getKind()) {
    case RegexExpr::Kind::String: {
      auto str = reinterpret_cast<StringRegexExpr*>(expr)->value;
      auto* n = new Node;
      NodePair pair = {n, n};
      // std::cout << "Here: \"" << Unescaped(str.str) << "\"\n";
      for (char c : Unescaped(str.str)) {
        auto* edge = new UnaryEdge;
        // Very silly, but I don't want to make the Type emitter more complicated
        // right now.
        edge->match = new char(c);
        edge->next = new Node;
        pair.ed->edges.push_back(edge);
        pair.ed = edge->next;
      }
      return pair;
    } case RegexExpr::Kind::Integer: {
      auto str = reinterpret_cast<IntegerRegexExpr*>(expr)->value;
      NodePair pair = {new Node, new Node};
      auto* edge = new UnaryEdge;
      pair.st->edges.push_back(edge);
      edge->match = new char(fromString(str.str));
      edge->next = pair.ed;
      return pair;
    } case RegexExpr::Kind::Named: {
      auto key = reinterpret_cast<NamedRegexExpr*>(expr)->name.str;
      auto it = lets.find(key);
      if (it == lets.end()) {
        std::cerr << "Could not find: " << key << "\n";
        exit(-1);
      }
      return visit(lets, it->second);
    } case RegexExpr::Kind::Range: {
      auto tmp = reinterpret_cast<RangeRegexExpr*>(expr);
      NodePair pair = {new Node, new Node};
      auto* edge = new RangeEdge;
      edge->start = new char(GetAsChar(tmp->st));
      edge->end = new char(GetAsChar(tmp->ed));
      edge->next = pair.ed;
      pair.st->edges.push_back(edge);
      return pair;
    } case RegexExpr::Kind::Juxta: {
      auto tmp = reinterpret_cast<JuxtaRegexExpr*>(expr);
      auto a = visit(lets, tmp->lhs);
      auto b = visit(lets, tmp->rhs);
      MakeSkip(a.ed, b.st);
      return {a.st, b.ed};
    } case RegexExpr::Kind::Alt: {
      NodePair pair = {new Node, new Node};
      auto tmp = reinterpret_cast<AltRegexExpr*>(expr);
      auto a = visit(lets, tmp->lhs);
      auto b = visit(lets, tmp->rhs);
      MakeSkip(pair.st, a.st);
      MakeSkip(pair.st, b.st);
      MakeSkip(a.ed, pair.ed);
      MakeSkip(b.ed, pair.ed);
      return pair;
    } case RegexExpr::Kind::Star: {
      NodePair pair = {new Node, new Node};
      MakeSkip(pair.st, pair.ed);
      auto tmp = reinterpret_cast<StarRegexExpr*>(expr);
      auto a = visit(lets, tmp->base);
      MakeSkip(a.ed, a.st);
      MakeSkip(pair.st, a.st);
      MakeSkip(a.ed, pair.ed);
      return pair;
    } case RegexExpr::Kind::Plus: {
      auto tmp = reinterpret_cast<PlusRegexExpr*>(expr);
      auto a = visit(lets, tmp->base);
      MakeSkip(a.ed, a.st);
      return a;
    }
    }
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

  void ComputeSuccessors(Node* base, std::vector<Node*> states,
                         std::function<Node*(std::vector<Node*> states)> get_state) {
    // printf("--- Doing emit ---\n");
    Edge* emit_or_ignore = nullptr;
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
    auto addEdge = [&](char st, char ed, Node* n) {
      // printf("addEdge(%d -> %d);\n", st, ed);
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
            printf("(%d -> %d) clashes with: (%d -> %d);\n",  st, ed, it->st, it->ed);
            exit(-1);
          }
        }
      }
      edges.insert(Entry {st, ed, {n}});
    };
    std::set<Node*> visited;
    while (!states.empty()) {
      auto* node = states.back();
      states.pop_back();
      if (visited.find(node) != visited.end()) continue;
      visited.insert(node);
      for (auto* edge : node->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range: {
          auto* tmp = reinterpret_cast<RangeEdge*>(edge);
          addEdge(*tmp->start, *tmp->end, tmp->next);
          break;
        } case Edge::Kind::Unary: {
          auto* tmp = reinterpret_cast<UnaryEdge*>(edge);
          addEdge(*tmp->match, *tmp->match, tmp->next);
          break;
        } case Edge::Kind::SkipTo:
          states.push_back(reinterpret_cast<SkipToEdge*>(edge)->next);
          break;
        case Edge::Kind::Unexpected:
          break;
        case Edge::Kind::Emit:
        case Edge::Kind::Ignore:
          if (emit_or_ignore) {
            fprintf(stderr, "Duplicate Emit/Ignore");
            exit(-1);
          }
          emit_or_ignore = edge;
          break;
        }
      }
    }
    for (auto& edge : edges) {
      auto* next = get_state(Canonicalize(edge.next));
      if (edge.st == edge.ed) {
        auto* nedge = new UnaryEdge;
        nedge->next = next;
        nedge->match = new char(edge.ed);
        base->edges.push_back(nedge);
      } else {
        auto* nedge = new RangeEdge;
        nedge->next = next;
        nedge->start = new char(edge.st);
        nedge->end = new char(edge.ed);
        base->edges.push_back(nedge);
      }
    }
    if (emit_or_ignore) {
      base->edges.push_back(emit_or_ignore);
    } else {
      base->edges.push_back(new UnexpectedEdge);
    }
    // printf("--- Done Doing emit ---\n");
  }

  Node* ToDFA(Node* r) {
    std::map<std::vector<Node*>, Node*> state_map;
    std::vector<std::pair<Node*, std::vector<Node*>>> work_list;
    auto* new_root = new Node;
    work_list.push_back({new_root, {r}});
    while (!work_list.empty()) {
      auto work_item = work_list.back();
      work_list.pop_back();
      ComputeSuccessors(work_item.first, work_item.second, [&](std::vector<Node*> states) -> Node* {
        std::sort(states.begin(), states.end());
        auto it = state_map.find(states);
        if (it == state_map.end()) {
          auto* node = new Node;
          work_list.push_back({node, states});
          return state_map[states] = node;
        }
        return it->second;
      });
    }
    return new_root;
  }
  NFAGraphDecl* rewriteDecl(RegexDecl* decl) {
    auto* result = new NFAGraphDecl;
    result->name = decl->name;
    result->root = new Node;
    std::map<string_view, RegexExpr*> lets;
    std::map<string_view, tok::Token> taken;
    for (auto* item : decl->items) {
      switch (item->getKind()) {
      case TokenDecl::Kind::Let: {
        auto* tmp = reinterpret_cast<LetTokenDecl*>(item);
        lets[tmp->name.str] = tmp->value;
        break;
      } case TokenDecl::Kind::Import: {
        fprintf(stderr, "Import not supported right now\n");
        break;
      } case TokenDecl::Kind::Emit: {
        auto* emit = reinterpret_cast<EmitTokenDecl*>(item);
        auto nfa = visit(lets, emit->value);
        {
          auto* edge = new SkipToEdge;
          edge->next = nfa.st;
          result->root->edges.push_back(edge);
        }
        auto* edge = new EmitEdge;
        edge->name = emit->name;
        nfa.ed->edges.push_back(edge);
        break;
      } case TokenDecl::Kind::Ignore: {
        auto* emit = reinterpret_cast<IgnoreTokenDecl*>(item);
        auto nfa = visit(lets, emit->value);
        auto* edge = new SkipToEdge;
        edge->next = nfa.st;
        result->root->edges.push_back(edge);
        nfa.ed->edges.push_back(new IgnoreEdge);
        break;
      }
      }
    }
    // std::cerr << "\n\nDumping: " << decl->name.str << "\n";
    // DumpNFA().dump(result->root);
    // std::cerr << "\n\n";
    result->root = ToDFA(result->root);
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

struct LoweringToJumpText {
  void visit(Decl* decl) {
    switch (decl->getKind()) {
    case Decl::Kind::Regex: {
      fprintf(stderr, "Unlowered regex expr!!\n");
      exit(-1);
    } case Decl::Kind::NFAGraph: {
      auto* tmp = reinterpret_cast<NFAGraphDecl*>(decl);
      std::cerr << "Emitting: " << tmp->name.str << "\n";
      GotoDFAEmitter().emitRoot(tmp->root);
      return;
    }
    }
  }
  void visit(Module* m) {
    for (auto* decl : m->decls) { visit(decl); }
  }
};

}  // namespace parser_spec

