# On the composition of languages

This document hopes to show how languages can currently be composed, and then open up speculation on
what possible other compositional systems may exist.

## Statement and expression system

The most primal of all compositional systems is the expression tree. Through the addition of lambda expressions,
we can allow for a possibly infinite "computation dag". Pushing this (as the singular composition system) to its
limits is the current purview of functional programming. Functional programs eventually break down and add some
statement composition features. Imperative programs start instead with a statement composition system, and eventually
add the expression tree composition features.

For example,
```
syntax: _ * _
associtivity, precidence
type checking: ...How this extends the type inference system...
implementation for integers: a * b -> integerMultiplication(a, b)
implementation for floats: a * b -> floatMultiplication(a, b)
```

One can see how such definitions could compose together to be a complete operator experience similar to what you would expect
from normal programming languages. An operator could also be defined differently in multiple different languages, and we would
want to compose those languages together. This would just be a matter of reconciling the differences. Hopefully, this would trivially
compose during type-checking / inference (or runtime dispatch for those so inclined). This suggests that there will be interesting
composition rules to explore during type-checking in the pursuit of highly general language systems.

Names can be considered as a language feature that is composed in, or a methodology to add custom extensions to a language. Either way,
 they follow interesting composition rules as well. For example, name conflicts become composition failures which can be resolved by renaming
 (aliasing) the names or via namespacing.

In this model, a simple function definition becomes a very trivial language (like the operator definition above). In this sense,
it is like defining an operator that conforms to a strict definition.  Similarly, type definitions instantiate a small language
of composition rules for that type (pre-composed with the dependencies of the type). These composition rules then become available
in the dependent code.

## Foreign function interfaces (FFI)

Less esoterically, FFI interfaces constitute another real-world example of language composition. Normally, however, this is a
very painful experience because both languages make many assumptions about the runtime behavior of things. None of these details 
are normally particularly relevant to the particular library that you are interested in FFI'ing into. Ideally, we would compose
directly with the language that naturally embeds the target library. This necessitates that the library not privilege any particular
language, lest it loses some of the power to embed into other languages. That, or the library be written in a language purpose
designed to have a generally compatible runtime. Not that they are incredibly compelling, but common runtime languages (like java, scala, etc
or the .net family) allow language more trivially composition. These compositions are less about different languages having
different strengths, but more of a new language trying to bootstrap its libraries and user-base by extending another language.

## Interface composition

There is a notion of an interface. This separates implementation details from the rules of composition that may be available to
users of a particular module. Implementation and interfaces need not be in the same language. This leads to some interesting composition
rules. Two language implementations may not compose syntactically, but if the interfaces can compose, then they can be used together.
In addition, an implementation might have a compilation process that takes it deterministically to only a single general purpose language,
but the interface might take a far more round-about journey, being composed into many different client languages.
