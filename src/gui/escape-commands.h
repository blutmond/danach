#pragma once

#include "gui/buffer.h"

#include <exception>
#include <vector>
#include <memory>

struct NoMoreKeyvalException : public std::exception {
  const char * what() const throw () {
    return "No more keyval. (This is not an error)";
  }
};

struct InvalidKeyvalException : public std::exception {
  const char * what() const throw () {
    return "Keyval is invalid.";
  }
};

struct KeyFeed {
  uint32_t Next() {
    if (i < command_history.size()) return command_history[i++];
    throw NoMoreKeyvalException();
  }
  const std::vector<uint32_t>& command_history;
  size_t i = 0;
};

enum class Mode {
  INSERT,
  ESCAPE,
  COLON,
};

struct EscapeCommandApply;

class PasteAction {
 public:
  virtual ~PasteAction() {}
  virtual void Apply(EscapeCommandApply& ctx, bool before) = 0;
};

struct EscapeEditContext {
  BufferPos mark_a{10000000000,10000000000};
};

struct EscapeCommandApply {
  // EscapeCommandApply(Mode& mode, Buffer& buffer) : mode(mode), buffer(buffer) {}

  Mode& mode;
  Buffer* buffer;
  BufferPos& cursor;
  EscapeEditContext& esc_ctx;
  std::unique_ptr<PasteAction>& paste_action;
};

struct CharRange {
  char st;
  char ed;
};

using CommandFn = void(EscapeCommandApply&);

struct CommandList {
  class Matcher {
   public:
    virtual ~Matcher() {}
  //  virtual bool IsFixed() = 0;
  //  virtual CharRange PopRange() = 0;
    virtual void Match(KeyFeed& feed, EscapeCommandApply& ctx) = 0;
  };
  std::vector<std::unique_ptr<Matcher>> matchers;
  void addStrict(string_view match, CommandFn fn);
  bool HandleCommand(EscapeCommandApply& ctx, std::vector<uint32_t>& command_history);
};

void AddDefaultCommands(CommandList& list);
