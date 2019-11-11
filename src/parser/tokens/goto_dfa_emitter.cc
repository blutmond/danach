#include "parser/tokens-passes.h"
#include "gen/parser/parser-spec.h"
#include "gen/parser/tokenizer-spec.h"
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace parser_spec {

struct GotoDFACPPEmitter {
  int id_assign = 0;
  std::vector<std::pair<Node*, int>> work_list;
  std::unordered_map<Node*, int> assigned;

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
          std::cout << "  if (c >= ";
          prettyEmitChar(tmp->start);
          std::cout << " && c <= ";
          prettyEmitChar(tmp->end);
          std::cout << ") { \n";
          std::cout << " ++cur; \n";
          std::cout << "c = *cur; \n";
          std::cout << " goto bb" << id << "; }\n";
          break;
        } case Edge::Kind::Unary: {
          auto* tmp = reinterpret_cast<UnaryEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  if (c ==";
          prettyEmitChar(tmp->match);
          std::cout << ") { \n";
          // never advance 0.
          if (tmp->match != 0) {
            std::cout << " ++cur; \n";
            std::cout << "c = *cur; \n";
          }
          std::cout << " goto bb" << id << "; }\n";
          break;
        } case Edge::Kind::SkipTo:
          fprintf(stderr, "Un-eliminated skip-to\n");
          exit(-1);
          break;
        case Edge::Kind::Emit:
          std::cout << "  return MakeToken(tok::" <<
              reinterpret_cast<EmitEdge*>(edge)->name.str <<
              ", st, cur);\n";
          break;
        case Edge::Kind::Ignore:
          std::cout << "  goto start;\n";
          break;
        case Edge::Kind::Unexpected:
          std::cout << "  unexpected(c);\n";
          break;
        }
      }
    }
  }
};

struct TokenizerIndex {
  NFAGraphDecl* decl;

  std::set<string_view> all_tokens;
  TokenizerIndex(NFAGraphDecl* decl) : decl(decl) {
    std::set<Node*> visited;
    std::vector<Node*> work_list = {decl->root};
    while (!work_list.empty()) {
      auto* nxt = work_list.back();
      work_list.pop_back();
      auto add_item = [&](Node* n) {
        if (visited.find(n) == visited.end()) {
          work_list.push_back(n);
          visited.insert(n);
        }
      };
      for (auto* edge : nxt->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range:
          add_item(reinterpret_cast<RangeEdge*>(edge)->next);
          break;
        case Edge::Kind::Unary:
          add_item(reinterpret_cast<UnaryEdge*>(edge)->next);
          break;
        case Edge::Kind::Emit:
          all_tokens.insert(reinterpret_cast<EmitEdge*>(edge)->name.str);
          break;
        default:
          break;
        }
      }
    }
//    for (auto token : all_tokens) { std::cout << "Found token: \"" << token << "\"\n"; }
  }

  bool isToken(string_view str) const { return all_tokens.find(str) != all_tokens.end(); }

  void Emit(std::ostream& stream, bool is_header) const {
    stream << "namespace tok {\n";
    stream << "enum T {";
    bool first = false;
    for (auto& name : all_tokens) {
      if (first) stream << ", ";
      stream << name;
      first = true;
    }
    stream << "};\n";
    stream << "const char* StringifyType(T t)";
    if (is_header) { stream << ";\n"; }
    else {
    stream << " {\n";
    stream << "switch(t) {\n";
    for (auto& name : all_tokens) {
      stream << "case " << name << ": return \"" << name << "\";";
    }
    stream << "}\n";
    stream << "}\n";
    }
    stream << R"(
struct Token {
  T type = tok::eof;
  string_view str;
};

Token MakeToken(T t, const char* st, const char* ed))"; if (is_header) { stream << ";"; } else { stream << R"( {
  return Token{t, string_view(st, ed - st)};
})"; } stream << R"(

void PrintToken(Token t))"; if (is_header) { stream << ";"; } else { stream << R"( {
  std::cout << "tok::" << StringifyType(t.type) << " : \"" << t.str << "\"\n";
})"; } stream << R"(

void unexpected(char c))"; if (is_header) { stream << ";"; } else { stream << R"( {
  fprintf(stderr, "unexpected: \"%c\"\n", c);
  exit(-1);
})"; } stream << R"(

Token GetNext(const char*& cur))"; if (is_header) { stream << ";\n"; } else { stream << R"( {
    const char* st;
    int c;
  start:
    st = cur;
    c = *cur;
    goto bb0;
)";

    GotoDFACPPEmitter().emitRoot(decl->root);

    stream << "}\n";
}

    stream << "} // namespace tok\n";
    stream << R"(
struct Tokenizer {
  explicit Tokenizer(const char* cursor_inp) : cursor(cursor_inp) {
    start = cursor;
    current = tok::GetNext(cursor);
  }
  tok::Token peak() {
    return current;
  }
  tok::Token next() {
    auto res = current;
    current = tok::GetNext(cursor);
    return res;
  }
  tok::Token expect(tok::T t) {
    auto res = next();
    if (t != res.type) {
      auto pos = GetLineInfo(start, res.str.data());
      fprintf(stderr, "error:%d:%d: expected: tok::%s but got:", pos.line, pos.col, tok::StringifyType(t));
      std::cerr << "tok::" << StringifyType(res.type) << " : \"" << res.str << "\"\n";
      exit(-1);
    }
    return res;
  }
  tok::Token expect(const char* c) {
    auto res = next();
    if (c != res.str) {
      auto pos = GetLineInfo(start, res.str.data());
      fprintf(stderr, "error:%d:%d: expected: \"%s\" but got:", pos.line, pos.col, c);
      std::cerr << "tok::" << StringifyType(res.type) << " : \"" << res.str << "\"\n";
      exit(-1);
    }
    return res;
  }
  bool peak_check_str(const char* str) {
    return peak().str == str;
  }
  bool peak_check(tok::T type) {
    return peak().type == type;
  }
  void unexpected() __attribute__ ((__noreturn__)) {
    unexpected(peak());
  }
  void unexpected(tok::Token tok) __attribute__ ((__noreturn__)) {
    auto pos = GetLineInfo(start, tok.str.data());
    fprintf(stderr, "error:%d:%d: unexpected:", pos.line, pos.col);
    std::cerr << "tok::" << StringifyType(tok.type) << " : \"" << tok.str << "\"\n";
    exit(-1);
  }

 private:
  const char* start;
  const char* cursor;
  tok::Token current;
};
)";
  }
};

struct TokenizerModuleIndex {
  TokenizerModuleIndex(Module* m) {
    for (auto* decl : m->decls) {
      if (decl->getKind() == Decl::Kind::NFAGraph) {
        auto* nfa_decl = reinterpret_cast<NFAGraphDecl*>(decl);
        tokenizers[nfa_decl->name.str].reset(new TokenizerIndex(nfa_decl));
      }
    }
  }
  std::map<string_view, std::unique_ptr<TokenizerIndex>> tokenizers;

  std::set<string_view> getTokenSet(string_view str) {
    auto it = tokenizers.find(str);
    if (it == tokenizers.end()) {
      std::cerr << "No such tokenizer: " << str << "\n";
      exit(-1);
    }
    return it->second->all_tokens;
  }

  void EmitTokenizer(string_view str, std::ostream& stream, bool is_header) const {
    auto it = tokenizers.find(str);
    if (it == tokenizers.end()) {
      std::cerr << "No such tokenizer: " << str << "\n";
      exit(-1);
    }
    it->second->Emit(stream, is_header);
  }
};

TokenizerPreEmit* FetchTokenizer(Module* m, string_view name) {
  TokenizerModuleIndex idx(m);
  auto it = idx.tokenizers.find(name);
  if (it == idx.tokenizers.end()) {
    std::cerr << "No such tokenizer: " << name << "\n";
    exit(-1);
  }
  auto* result = new TokenizerPreEmit;
  result->decl = it->second->decl;
  result->all_tokens = it->second->all_tokens;
  return result;
}

void EmitTokenizer(TokenizerPreEmit* tokens, std::ostream& stream, bool is_header) {
  TokenizerIndex idx(tokens->decl);
  idx.Emit(stream, is_header);
}

}  // namespace parser_spec
