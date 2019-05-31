# On an isomorphism between libraries and languages

## A small survey of domain specific language systems

Domain specific languages (DSL) are often juxtaposed against general purposes languages.
The purported benefits of a domain specific language is that users can program in a
syntax that is more convenient for the particular domain, and receive better error messages.
Because a DSL is not sufficient to write its own compiler, it must necessarily be a member of
a system of languages that work together to build the DSL. This is normally a host general
purpose language and the original DSL. This has the unfortunate problem that if a user runs
into a problem with a DSL, they may receive cryptic errors from the host language.  It may also
be true that the DSL compiles down to yet another language. This language may raise cryptic errors
back up to the user as well.

An answer to these problems is to forgo the desire to completely control the syntax and embed a 
DSL (an EDSL) into a general purpose programming language. Some languages advertise varying levels
of EDSL support.  In some languages (c++), EDSLs are not supported at all, but archaic features
(templates + macros) are abused to impressive results of allowing the generation of high performance code.

## What is a library anyways?

As touched on above, EDSLs blur the lines between general purpose languages and DSLs. Stretching this blurring
to the limits of the imagination brings up the suggestion that most if not all programming languages must
necessarily support features for creating and managing EDSLs. These are just given an arbitrary separation
and called objects, libraries, modules, etc. Thus, all libraries can be seen as an EDSL.
They are, however, often a very poor and convoluted embedding. There have been attempts to add rigor to
this general library embedding process in the form of "design patterns" and other such ideas. Some types of
libraries (normally for protocols), however, have taken the approach of providing a DSL (or EDSL) that is
used to define the protocol and then the function that embeds them is used to 
Examples of this type are:
- protobuf
- thrift
- wayland
- vulkan
- webidl
- v8 has some things like this for defining high performance javascript APIs.

This all provides evidence that there are real-world libraries that are defined by DSLs and then generated out as libraries
which roughly illustrates the function from DSLs to libraries, the converse is also interesting, how could you make a pre-existing
library into a DSL.

Unfortuantely, the functor that embeds a DSL into a general purpose language is lossy. It can involve human processes which
can develop inconsistent naming etc. This means that this function mostly requires a human to reverse it. The ugliest way that
this can happen though is via reflection information. This creates a runtime DSL that is the same as the compiled library DSL.
However, not much of the true structure is recovered. There is hope however, some programmers exercise discipline in the structures of
their libraries. These programmers can use reflection information to match particular patterns in their library and match them to
recover the lost fragments of their original library DSL.

## An example of a reversal from a library into a DSL

A vector graphics library (move_to, line_to, curve_to, stroke, fill, set_color). This can be converted into a DSL that would
look very much like SVG. Only for performance reasons do we not fully materialize the SVG file, and the draw that.
This api is just a builder API for this very concrete vector graphics language.
What if, because we're compiling things, we could compile away the intermediary saved state when compiling the
two functions (to SVG, and SVG to pixels). Then, we could capture the intermediaries and save them, etc. All of this could be
done naturally in a generic fasion, just because we expressed it as the true language instead of its builder embedding.

## Pulling it all together

The general summary pattern is that library / DSL development can be thought of as the following interactions of
DSLs:

- A "standard" DSL that allows defining the syntax of a DSL.
- Use that DSL to define the primitive types of the DSL.
- Use a second "standard" DSL that allows defining transformations from the defined DSL and its standard
library into many (or just one) target language.

General purpose programming languages take the role of many (or sometimes all) of these components.
One question to explore is: "what if they did not?" What if DSLs took the place of all of these?
