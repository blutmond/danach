struct LineInfo {
  int line;
  int col;
};

LineInfo GetLineInfo(const char* st, const char* cur) {
  if (cur < st) {
    std::cerr << "Problem!" << __FILE__ << " " << __LINE__ << "\n";
    exit(255);
  }
  LineInfo res {1, 0};
  const char* last_start = st;
  for (; st < cur; ++st) {
    if (*st == '\n') {
      last_start = st + 1;
      res.line += 1;
    }
  }
  res.col = 1 + (cur - last_start);
  return res;
}
