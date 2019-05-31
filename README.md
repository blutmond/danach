# Welcome!

You've stumbled on this obscure "compiler/language design" project somehow. If this piques your interest,
sending a pull request or an issue for anything is the best way to get in touch and show interest.
This project itself is an explicit solicitation for collaborators, so please error on the side of reaching out.

# About

The purpose of this project is to explore a collection of tightly correlated concepts relating to
category theory, languages, traditional style compilers, libraries and the general evolution and
execution of those things. One intent is to challenge conceptions on what a programming language really is
At the same time, obviously nothing really will be challenged because this all builds on solid 
existing programming language theories.
There are [docs](docs/vision/index.md) that expand on these topics more.

Chiefly among those concepts are the following:
- Part of category theory explores functors between languages. A compiler is thus one such functor. What would
a rigorous approach to exploring this analogy uncover?
- Information is embedded into code and code is also information itself. As much as possible, we as programmers should
be capturing directly the information we seek to encode into programming structure into the most generally useful form.
- Libraries and Domain Specific Languages are deeply intertwined.

# Building things so far.

Note: All commands must be run from the root directory.
Also note: There are some cycles in the build graph.
src/rules intends to manage this.

```bash
# Get things started
./tools/bootstrapping.sh

# Build everything.
./tools/simple-build.sh
```


# FAQ?

This section is added as a place to send prs for questions.

### You got 'X' wrong about category theory.

I plead ignorance! Please send a correction.
