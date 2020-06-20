#include "gui/escape-commands.h"

// using CommandFn = void(EscapeCommandApply&);
bool CommandList::HandleCommand(EscapeCommandApply& ctx, std::vector<uint32_t>& command_history) {
  for (auto& matcher : matchers) {
    try {
      KeyFeed feed{command_history};
      matcher->Match(feed, ctx);
      return true;
    } catch(InvalidKeyvalException& e) {}
  }
  return false;
}

class StrictMatcher : public CommandList::Matcher {
 public:
  string_view match_;
  CommandFn* fn_;

  StrictMatcher(string_view match, CommandFn fn) : match_(match), fn_(fn) {}
  
  void Match(KeyFeed& feed, EscapeCommandApply& ctx) override {
    if (ctx.buffer == nullptr) return;
    for (size_t i = 0; i < match_.size(); ++i) {
      if (match_[i] != feed.Next()) throw InvalidKeyvalException();
    }
    fn_(ctx);
  }
 
//  string_view match_;
//  CommandFn fn_;
};

class LinePaste : public PasteAction {
 public:
  explicit LinePaste(std::string data) : data(data) {}
  void Apply(EscapeCommandApply& ctx, bool before) override {
    if (!before) ctx.cursor.row += 1;
    ctx.buffer->lines.insert(ctx.buffer->lines.begin() + ctx.cursor.row, data);
    ctx.cursor.col = 0;
  }
  std::string data;
};

void CommandList::addStrict(string_view match, CommandFn fn) {
  matchers.emplace_back(new StrictMatcher(match, fn));
}

void AddDefaultCommands(CommandList& list) {
  using C = EscapeCommandApply;
  list.addStrict("i", +[](C& c) { c.mode = Mode::INSERT; });
  list.addStrict("o", +[](C& c) {
    c.mode = Mode::INSERT;
    c.cursor.col = c.buffer->lines[c.cursor.row].size();
    Insert(*c.buffer, c.cursor, '\n');
  });
  list.addStrict("dd", +[](C& c) {
    auto& lines = c.buffer->lines;
    auto data = std::move(lines[c.cursor.row]);
    c.paste_action = std::make_unique<LinePaste>(std::move(data));
    lines.erase(lines.begin() + c.cursor.row);
    if (c.cursor.row == lines.size()) {
      if (c.cursor.row == 0) {
        lines.push_back("");
      } else {
        c.cursor.row -= 1;
      }
    }
    c.cursor.col = 0;
  });
  list.addStrict("yy", +[](C& c) {
    auto& lines = c.buffer->lines;
    auto data = lines[c.cursor.row];
    c.paste_action = std::make_unique<LinePaste>(std::move(data));
  });
  list.addStrict("p", +[](C& c) { if (c.paste_action) c.paste_action->Apply(c, false); });
  list.addStrict("P", +[](C& c) { if (c.paste_action) c.paste_action->Apply(c, true); });
}
