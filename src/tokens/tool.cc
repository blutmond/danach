#include <string>
#include <experimental/string_view>
#include <map>
#include <set>
#include <vector>
#include <stdio.h>
#include <typeinfo>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using std::experimental::string_view;

#include "tokens/line_number_helper.cc"
#include "gen/tokens/tokenizer.cc"
#include "gen/tokens/types.cc"
#include "tokens/parser.cc"

namespace tokenizer_gen {

class EmitterState {
 public:
  EmitterState(Module* m) : m_(m) {}

  std::ostream& stream() {
    if (line_start) {
      if (indent > 0) {
        for (int i = 0; i < indent; ++i) {
          std::cout << "  ";
        }
      }
      line_start = false;
    }
    return std::cout;
  }

  void EmitValue(SetExpr* expr) {
    switch (expr->getKind()) {
    case SetExpr::Kind::Integer:
      stream() << static_cast<IntegerSetExpr*>(expr)->value.str;
      break;
    case SetExpr::Kind::Char: {
      auto tmp = static_cast<CharSetExpr*>(expr)->value.str;
      tmp.remove_prefix(1);
      tmp.remove_suffix(1); 
      stream() << "'" << tmp << "'";
      break;
    } default:
      fprintf(stderr, "problem!\n");
      exit(-1);
    }
  }

  void visit(SetExpr* expr) {
    switch (expr->getKind()) {
    case SetExpr::Kind::Integer:
    case SetExpr::Kind::Char: {
      stream() << "(c == ";
      EmitValue(expr);
      stream() << ")";
      break;
    } case SetExpr::Kind::Range: { auto* self = static_cast<RangeSetExpr*>(expr);
      stream() << "((c >= ";
      EmitValue(self->lhs);
      stream() << ") && (c <=";
      EmitValue(self->rhs);
      stream() << "))";
      break;
    } case SetExpr::Kind::Sum: { auto* self = static_cast<SumSetExpr*>(expr);
      stream() << "(";
      visit(self->lhs);
      stream() << " || ";
      visit(self->rhs);
      stream() << ")";
      break;
    } case SetExpr::Kind::Named: { auto* self = static_cast<NamedSetExpr*>(expr);
      visit(getLiteral(self->name));
      break;
    }
    }
  }

  void visit(Stmt* stmt) {
    switch (stmt->getKind()) {
    case Stmt::Kind::While: { auto* self = static_cast<WhileStmt*>(stmt);
      stream() << "while (true) {"; Indent(); NL();
      visit(self->body);
      DeIndent(); stream() << "}"; NL();
      break;
    } case Stmt::Kind::Case: { auto* self = static_cast<CaseStmt*>(stmt);
      stream() << "if ";
      visit(self->cond);
      stream() << " {"; Indent(); NL();
      visit(self->true_stmt);
      if (self->false_stmt->getKind() == Stmt::Kind::Case) {
        auto* else_stmt = static_cast<CaseStmt*>(self->false_stmt);
        DeIndent(); stream() << "} else ";
        visit(else_stmt);
      } else {
        DeIndent(); stream() << "} else {"; Indent(); NL();
        visit(self->false_stmt);
        DeIndent(); stream() << "}"; NL();
      }
      break;
    } case Stmt::Kind::Compound: { auto* self = static_cast<CompoundStmt*>(stmt);
      for (auto* stmt : self->stmts) visit(stmt);
      break;
    } case Stmt::Kind::Fatal: { auto* self = static_cast<FatalStmt*>(stmt);
      stream() << "fprintf(stderr, " << self->err_string.str << ");"; NL();
      stream() << "exit(-1);"; NL();
      break;
    } case Stmt::Kind::Return: { auto* self = static_cast<ReturnStmt*>(stmt);
      stream() << "return MakeToken(tok::" << self->name.str << ", st, cur);"; NL();
      break;
    } case Stmt::Kind::Break: {
      stream() << "break;"; NL();
      break;
    } case Stmt::Kind::Unexpected: {
      stream() << R"(fprintf(stderr, "unexpected: \"%c\"\n", c);)"; NL();
      stream() << "exit(-1);"; NL();
      break;
    } case Stmt::Kind::Next: {
      stream() << "++cur;"; NL();
      stream() << "c = *cur;"; NL();
      break;
    } 
    }
  }

  SetExpr* getLiteral(tok::Token t);

  void Indent() { ++indent; }
  void DeIndent() { --indent; }

  void NL() { std::cout << "\n"; line_start = true;}

 private:
  int indent = 2;
  bool line_start = true;
  Module* m_;
};

struct CollectTokenSet {
  void visit(Stmt* body) {
    switch (body->getKind()) {
    case Stmt::Kind::Compound: { auto* self = static_cast<CompoundStmt*>(body);
      for (auto* stmt : self->stmts) visit(stmt);
      break;
    } case Stmt::Kind::Case: { auto* self = static_cast<CaseStmt*>(body);
      visit(self->true_stmt);
      visit(self->false_stmt);
      break;
    } case Stmt::Kind::While: { auto* self = static_cast<WhileStmt*>(body);
      visit(self->body);
      break;
    } case Stmt::Kind::Return: { auto* self = static_cast<ReturnStmt*>(body);
      tokens.emplace(self->name.str);
      break;
    }
    default: break;
    }
  }
  std::set<string_view> tokens;
};

SetExpr* EmitterState::getLiteral(tok::Token t) {
  auto it = m_->decls.find(t.str);
  if (m_->decls.end() == it) {
    std::cout << "Cannot find: " << t.str << "\n";
    exit(-1);
  }
  return it->second->expr;
}

void DumpTokenizer(Module* m, string_view name) {
  std::cout << "namespace " << name << " {\n";
  std::cout << "namespace tok {\n";

  CollectTokenSet token_set;
  token_set.visit(m->body);
  {
    std::cout << "\nenum T {";
    bool first = true;
    for (auto token : token_set.tokens) {
      if (!first) {
        std::cout << ", ";
      } else {
        std::cout << " ";
      }
      first = false;
      std::cout << token;
    }
    if (!first) std::cout << " ";
    std::cout << "};\n";
  }
  std::cout << R"(
const char* StringifyType(T t) {
  switch (t) {
)";
  for (auto token : token_set.tokens) {
    std::cout << "  case " << token << ": return \"" << token << "\";\n";
  }
  std::cout << "  }\n}\n";
  std::cout << R"(
struct Token {
  T type = tok::eof;
  string_view str;
};

Token MakeToken(T t, const char* st, const char* ed) {
  return Token{t, string_view(st, ed - st)};
}

void PrintToken(Token t) {
  std::cout << "tok::" << StringifyType(t.type) << " : \"" << t.str << "\"\n";
}

Token GetNext(const char*& cur) {
  while (true) {
    const char* st = cur;
    int c = *cur;
)";

  EmitterState state(m);
  state.visit(m->body);

  std::cout << "  }\n}\n\n";
  std::cout << "}  // namespace tok\n";

  std::cout << R"(

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
  void unexpected(tok::Token tok) {
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

  std::cout << "}  // namespace " << name << "\n";
}
}  // namespace tokenizer_gen

int main(int argc, char **argv){
  using namespace tokenizer_gen;
  if (argc <= 2) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  std::ifstream in(argv[1], std::ios::in | std::ios::binary);
  if (!in) {
    fprintf(stderr, "Could not read file: %s\n", argv[1]);
    exit(-1);
  }
  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();

  Module* m = parser::DoParse(contents.c_str());
  DumpTokenizer(m, argv[2]);
}
