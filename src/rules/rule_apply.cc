// This file contains hacks needed to finish rule_apply functionality.
// TODO: Fold this into a cpp_subset supported infra.

#include <string.h>
#include <sstream>
#include <map>
#include <memory>

struct CommandEval {
  std::vector<std::string> args;
  std::string outfile;

  std::vector<CommandEval*> deps;

  void Run() {
    int i = 0;
    for (auto& arg : args) {
      if (i != 0) { std::cout << " "; }
      ++i;
      std::cout << arg;
    }
    if (!outfile.empty()) {
      std::cout << " > " << outfile;
    }
    std::cout << "\n";
  }
};

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

namespace rule_spec {
std::string GetIdentifier(tok::Token token) {
  if (token.type == tok::identifier) {
    return std::string(token.str);
  }
  return Unescape(token.str);
}

std::vector<std::string> ProcessArgList(const std::vector<tok::Token>& tokens) {
  std::vector<std::string> args;
  for (auto& tok : tokens) {
    args.push_back(Unescape(tok.str));
  }
  return args;
}

struct MakeCommands {
  void visitFwd(Decl* decl_) {
    switch (decl_->getKind()) {
    case Decl::Kind::File:
      auto* decl = reinterpret_cast<FileDecl*>(decl_);
      cmds[GetIdentifier(decl->name)] = new CommandEval;
      return;
    }
  }
  std::string GetFileName(CommandEval* cmd, FileSource* src) {
    switch (src->getKind()) {
    case FileSource::Kind::Const:
      return Unescape(reinterpret_cast<ConstFileSource*>(src)->name.str);
    case  FileSource::Kind::Rule:
      return ".build/" + GetIdentifier(reinterpret_cast<ConstFileSource*>(src)->name);
    }
  }
  std::vector<std::string> ProcessDepList(CommandEval* cmd, const std::vector<FileSource*>& srcs) {
    std::vector<std::string> out;
    for (auto* src : srcs) {
      out.push_back(GetFileName(cmd, src));
    }
    return out;
  }

  std::vector<std::string> concat(std::vector<std::string> a, std::vector<std::string> b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
  }

  void visit(Decl* decl_) {
    switch (decl_->getKind()) {
    case Decl::Kind::File: {
      auto* decl = reinterpret_cast<FileDecl*>(decl_);
      auto id = GetIdentifier(decl->name);
      auto* cmd = cmds[id];
      auto* invoke_ = decl->invoke;
      switch (invoke_->getKind()) {
      case ToolInvoke::Kind::CppGen: {
        auto* invoke = reinterpret_cast<CppGenToolInvoke*>(invoke_);
        cmd->args = concat(std::vector<std::string>({
          GetFileName(cmd, invoke->tool),
          GetFileName(cmd, invoke->src)
        }), ProcessArgList(invoke->extra_flags));
        cmd->outfile = ".build/" + id;
        break;
      } case ToolInvoke::Kind::ClangBinary: {
        auto* invoke = reinterpret_cast<ClangBinaryToolInvoke*>(invoke_);
        ProcessDepList(cmd, invoke->dep_only);
        cmd->args = concat(concat(concat(std::vector<std::string>({
          // Get this from a config??
          "/usr/bin/clang-6.0",
        }), ProcessDepList(cmd, invoke->srcs)), std::vector<std::string>({
          "-o", ".build/" + id,
        })), ProcessArgList(invoke->flags));
        break;
      }
      }
      break;
    }
    }
  }
  void visit(Module* m) {
    for (auto* decl : m->decls) { visitFwd(decl); } 
    for (auto* decl : m->decls) { visit(decl); } 

    std::vector<CommandEval*> cmd_list;
    for (auto& pair : cmds) {
      cmd_list.push_back(pair.second);
    }
    for (auto* cmd : cmd_list) {
      cmd->Run();
    }
  }
  std::map<std::string, CommandEval*> cmds;
};
}  // namespace rule_spec

void DoEvalAll(rule_spec::Module* m) {
  rule_spec::MakeCommands().visit(m);
}

/*
    var pid: pid_t;
    var action: posix_spawn_file_actions_t;
    posix_spawn_file_actions_init(addr(action));
    posix_spawn_file_actions_addclose(addr(action), 0);
    
    if (ne(posix_spawn(addr(pid), "/usr/bin/clang-6.0", addr(action), nullptr,
                  argv, nullptr), 0)) {
      fprintf(stderr, "Could not start process: %s\n", inline_cpp("std::strerror(errno)"));
      exit(255);
    }
    posix_spawn_file_actions_destroy(addr(action));

    var stat_loc: int;
    printf("hmm: %d\n", wait(addr(stat_loc)));
    fprintf(stderr, "Problem with wait: %s\n", inline_cpp("std::strerror(errno)"));
    printf("hmm: %d\n", wait(addr(stat_loc)));
    fprintf(stderr, "Problem with wait: %s\n", inline_cpp("std::strerror(errno)"));
    exit(255);
    */

/*
    loop {
      var token: Token = tokens.next();
      if (eq(inline_cpp("rule_spec::tok::eof"), token.type)) {
        break;
      }
      PrintToken(token);
    }
*/
