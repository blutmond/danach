namespace tokenizer_gen {
namespace parser {

SetExpr* ParseSetExpr(Tokenizer& tokens);

SetExpr* ParseSetExprLiteral(Tokenizer& tokens) {
  auto token = tokens.next();
  if (token.type == tok::str) {
    auto* res = new CharSetExpr;
    res->value = token;
    return res;
  } else if (token.type == tok::number) {
    auto* res = new IntegerSetExpr;
    res->value = token;
    return res;
  } else if (token.type == tok::identifier) {
    auto* res = new NamedSetExpr;
    res->name = token;
    return res;
  } else if (token.type == tok::open_paran) {
    auto* result = ParseSetExpr(tokens);
    tokens.expect(tok::close_paran);
    return result;
  } else {
    fprintf(stderr, "unexpected(%d): ", __LINE__);
    PrintToken(token);
    exit(-1);
  }
}

SetExpr* ParseBaseSetExpr(Tokenizer& tokens) {
  auto* current = ParseSetExprLiteral(tokens);
  auto token = tokens.peak();
  if (token.type == tok::colon) {
    auto* res = new RangeSetExpr;
    res->loc = tokens.next();
    res->lhs = current;
    res->rhs = ParseSetExprLiteral(tokens);
    current = res;
  }
  return current;
}

SetExpr* ParseSetExpr(Tokenizer& tokens) {
  SetExpr* current = ParseBaseSetExpr(tokens);
  auto token = tokens.peak();
  while (token.type == tok::plus) {
    auto* res = new SumSetExpr;
    res->loc = tokens.next();
    res->lhs = current;
    res->rhs = ParseBaseSetExpr(tokens);
    current = res;
    token = tokens.peak();
  }
  return current;
}

Stmt* ParseStmt(Tokenizer& tokens);

CompoundStmt* ParseCompoundStmt(Tokenizer& tokens) {
  tokens.expect(tok::open_brace);
  auto* result = new CompoundStmt();
  while (true) {
    auto token = tokens.peak();
    if (token.type == tok::close_brace) {
      break;
    } else {
      result->stmts.push_back(ParseStmt(tokens));
    }
  }
  tokens.expect(tok::close_brace);
  return result;
}

CaseStmt* ParseCaseStmt(Tokenizer& tokens) {
  auto* res = new CaseStmt;
  res->cond = ParseSetExpr(tokens);
  res->true_stmt = ParseCompoundStmt(tokens);
  auto token = tokens.peak();
  if (token.str == "case") {
    tokens.next();
    res->false_stmt = ParseCaseStmt(tokens);
  } else if (token.str == "default") {
    tokens.next();
    res->false_stmt = ParseCompoundStmt(tokens); 
  } else {
    res->false_stmt = new CompoundStmt();
  }
  return res;
}

Stmt* ParseStmt(Tokenizer& tokens) {
  auto token = tokens.next();
  if (token.str == "next") { tokens.expect(tok::semi); return new NextStmt();
  } else if (token.str == "break") { tokens.expect(tok::semi); return new BreakStmt();
  } else if (token.str == "unexpected") { tokens.expect(tok::semi); return new UnexpectedStmt();
  } else if (token.str == "stcase") {
    return ParseCaseStmt(tokens);
  } else if (token.str == "while") {
    auto* res = new WhileStmt;
    res->body = ParseCompoundStmt(tokens);
    return res;
  } else if (token.str == "return") {
    auto name = tokens.expect(tok::identifier);
    tokens.expect(tok::semi);
    auto* res = new ReturnStmt;
    res->name = name;
    return res;
  } else if (token.str == "fatal") {
    tokens.expect(tok::open_paran);
    auto name = tokens.expect(tok::str);
    tokens.expect(tok::close_paran);
    tokens.expect(tok::semi);
    auto* res = new FatalStmt;
    res->err_string = name;
    return res;
  } else {
    fprintf(stderr, "unexpected(%d): ", __LINE__);
    PrintToken(token);
    exit(-1);
  }
}

Module* DoParse(const char* cur) {
  Tokenizer tokens(cur);
  auto* m = new Module();
  while (true) {
    auto token = tokens.next();
    if (token.type == tok::eof) return m;
    if (token.type == tok::identifier) {
      if (token.str == "let") {
        auto name = tokens.expect(tok::identifier);
        tokens.expect(tok::equal);
        auto* set_expr = ParseSetExpr(tokens);
        m->decls[name.str] = new LetDecl{name, set_expr};
        tokens.expect(tok::semi);
      } else if (token.str == "stcase") {
        m->body = ParseCaseStmt(tokens);
        tokens.expect(tok::eof);
        return m;
      } else {
        std::cerr << "unknown: " << token.str << "\n";
      }
    } else {
      fprintf(stderr, "unexpected(%d): ", __LINE__);
      PrintToken(token);
      exit(-1);
    }
  }
}

}  // namespace parser
}  // namespace tokenizer_gen
