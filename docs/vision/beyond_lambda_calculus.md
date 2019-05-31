# Going beyond lambda calculus

## On Analogies between language concepts

The most powerful analogy is an isomorphism or a correspondence. This means that any property in one system must
have a translation in the other system. This means that any idea that arises in one side of the analogy must have
a corresponding translated version in the other system. A great example of this is the
[Curry-Howard Correspondence](https://en.wikipedia.org/wiki/Curry%E2%80%93Howard_correspondence)
which brings together type theory and logic.

## The analogy between category theory and languages

The analogy between programming languages and category theory is very simple. Basically, everything is a category,
so programming languages must be categories. So the work must be to stretch this analogy to its useful limits.
One problem with studying this is the expensive nature of building a programming language. In order to study anything,
it gets a lot easier if there are many examples to choose from. It is thus imperative to bring this cost down in order
to unblock this work. Uniformity between languages can be instrumental in keeping the costs low.

Since everything with rules of composition is a category, these things are also languages. This gives us a fruitful source
of languages to use in order to study this composition. We simply need a methodology that forces each of these hidden and fundamental
languages from all of these domains to expose themselves in order to be studied.

## On Turing completeness

Turing completeness is the selling point of a general purpose language. Because everything can be done, there is no
hypothetical reason to go further. However, the problem comes from problems with performance and ease of expressing concepts.
Thus, once you've gotten to a point where you have something that is Turing complete, the question is then, "where to go
from here?" The natural progression of events has been that a few general purpose languages were constructed and then
many people set out on the task of embedding solutions to their domains into these languages. From the previous section on
libraries, we can see that each of these domains have their own languages. Some of these domains are more financially important
or fundamental than others, and these were prioritized in extending the languages in order to improve flexibility. One thing
that is depressing about this is that all this information is essentially "lost" once it has been encoded into a general purpose
language. The worst of all is assembly (the process of compilation). This makes it so that the original categories are completely lost,
even the names that humans might use to recover the original domain language have been lost to the compilation process.

An alternative to this general direction is that we should improve the tools necessary to keep the information in the highest
level (least expressive; most domain specific) of abstraction and then compile down only when necessary to actually run the code.
This, hopefully, will be a boon to people wishing to casually study expression of their particular favorite ideas.

When going beyond the most basic complete computation system, there is another trend that saw what was going on above, and
instead endeavoured to build very simple languages that have very powerful compile-time or run-time metaprogramming capabilities.
This is a great alternative to extending the language for each new domain feature, but it is still missing the mark by a little bit.
The problems are that the metaprogramming facilities are deeply tied to the host language, and host language features can be
mixed into the language after it has been defined. This increases the dependencies of a particular piece of information.
If we are explicit about, for example, a particular DSL needing to do general purpose math and imported a syntax for doing that,
it would be easier to constrain what is possible to do, and then it would thus be easier to compile this DSL to any number of target
host languages.
