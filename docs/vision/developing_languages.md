# On the development of languages

## Evolutionary processes

Written languages have a unique additive evolutionary process. A language is essentially a version history of many similar languages.
From the point where a language became Turing complete onward, we should be able to compile all the newer incantations of the language
to older versions of the language. (With the exception perhaps of exposed hardware features). This raises a path of study where
the many versions of a language can be studied together, and also automatic tools can be provided that take a piece of code from one
version of the language and converts it (upgrades) it to the newest version of the language. That would hopefully be made easier by
having a convenient formal definition of a language (which is amenable to comparing over time).

As before, everything that applies to languages can apply to libraries and DSLs.

## Practical concerns about developing languages

There is this sort of slow problem with libraries that over time, more expressive features are needed. These can often require,
for example, changing a base-class or a class hierarchy in a breaking way that affects many simpler language features and makes them
harder to understand. This transformation is often mechanical, and once complete allows divergence. This suggests that there is a
possible place to study the composition of library features together into a single library. Generalizing language design itself
has many of those practical concerns. Older simpler features may be made more complex (or be found redundant) by newer features.
Formalizing and studying this process may be of interest.
