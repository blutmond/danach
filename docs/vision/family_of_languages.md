# A family of languages

## All information in its best form

The ability to define new language features and compose those into existing composition frameworks can make a piece of information
entirely self-contained with all the necessary explicit dependencies listed. This will also abstract away unimportant details enabling
better retroactive and non-invasive performance optimizations. The interesting point on this space that begs to be investigated is
bringing down the costs of defining a domain specific language enough such that most libraries can become a definition of a DSL
and a straightforward lowering into actual interfaces and implementations that can be used in many languages. This, in turn,
will probably warp the actual concept of what a language is. No longer will languages be vying to express everything, but
the end-users who want to compose multiple libraries together will possibly use a pre-written DSL or composition framework that only
does simple compositions or composes code from multiple languages (sort of like how HTML + CSS + javascript works) together.
Then, there will actually be no need to compose everything together into a single general purpose language.

## A case study, string (sequence) optimizations

A string is just a sequence of text. String algorithms are normally relatively simple, and have certain performance characteristics
based on how consecutive the text is. Because strings can be small, and the functional operators are very simple (concat + split),
strings can wreck havoc on a memory management system. Multiple copies can be made on what might otherwise be cheep operations, and
overheads can add up from all the allocations, pointer chasing, etc. Optimization techniques might involve: arena allocation,
interning, lazy string concatenation, fast substring optimizations, copy on write, null termination, etc.
All these techniques can also be applied to arrays, tensors or even maybe expression trees. Ideally, such performance could be an
orthogonal concern from correctness. If large scale string processing work can be written in an analyzable fashion, code could
be written with a single string class, and then during the compilation process, the user could specify whole classes of string usages
to use a particular optimization type. In addition, normal compilation processes could promote some strings to be aliasing.

## Mathematics

Floating point computations only approximate computations over real numbers. There are many transformations over functions that you
might want to do: taking limits, composing purpose built optimizers, algebraic transformations for improved numerical stability,
generating monte-carlo processes to approximate integrals, swapping out the number system for Rationals or Algebraic number representations.
There are also convenient mathematical symbols that you might want to use. Embedding only a subset of "Math" into your language
prevents a large amount of language complexity, but having a co-language that could express all of these concepts would help these
exist in a single place with the greater expressive power that they deserve.

## Keeping the sanity

Most likely, a new language would be on the level of a new large library. These large libraries cover important concepts in computer science
like graphics, numerical computing, algorithms, image processing, or compilers. This should keep things more manageable. Also, like
how features like function definitions and imports of other libraries essentially give the ability to define language extensions,
sub-languages would be most useful when they compile down to an interface language that is most compatible to the language it is integrated
in. In this form, the composition would have pieces of syntax that are naturally designed to be extended by language extensions.
This is done in order to improve composition, but will have the effect of being far less surprising to users who will probably find
such language extensions very similar syntactically to important a macro-based DSL.

A possible outcome is that there will be two types of languages, ones that compose together to make a very simple functional form
(eg: String -> String) and ones that have high user complexity (a complex interaction of many types), but are slightly improved by
becoming a language (dropping builder API definitions for semantics-oriented API definitions). In the first case, the user will be
mostly unaffected (this code is already a black box) or in the second case, the "generated" APIs will simply be more internally
consistent and complete.

## What language to consume in

It may yet be that general purpose languages are the best way to consume software in, but such an understanding
need not be incompatible with the ideas here. Being honest about the fact that a library is the result of an embedding
would make the source code more self-similar as when changes are made, these diffs could be hypothetically be translated
(possibly with lag) into a delta in the true generalized syntax. This would probably demand interesting features from
the compilation processes. Pure users, however, need not see the generalized form. They can consume versions of the library
that have been composed into binaries and interface definitions that normal tools can 

## A cautionary tale

Many of the benefits of composition can also be achieved through tight integration and extensive top-down planning.
The problem with this approach is that anything that is not anticipated in the vision is hard to extend into the system.
This bundling can become troubling if a particular component does not carry its own weight, and the attractiveness of the other
components of the system make up for the shortcomings of this particular system. It is thus imperative that the system
be analyzed from time to time as it reaches greater maturity that any individual component can be entirely extracted with minimal
dependencies. The proposed framework thus far allows for this. Especially since it allows for separating optimizations from semantics.
Compiled languages allow for this dependency analysis (although it is seldom used).
