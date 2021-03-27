# MinTPL
**Min**imal **T**emplate **P**rogramming **L**anguage

## Description
A minimalistic language for automatic text processing, implemented as a C (17)
library for embedding. It is made specifically to be extended by the embedding
application. As a language it bears some similarities to Tcl.

### Example
File: input.mtpl
```
Have a look at this [for>[=>adjectives] adj {[=>adj], }]example.
```
Invokation using `mintpl-cli`:
```
$ mintpl-cli -p adjectives="small;silly" input.mtpl
Have a look at this small, silly, example.
```

### Features/characteristics

- Everything is a string.
- Depth-first evaluation.
- Variables are available as key-value properties.
- Dynamically scoped variable lookup through linked hashtables. 
- Not built for speed or continuous operation -- this is a "batch job" language.
- Small -- at the time of writing a static release build of the entire library
  is well below 32 KiB.
- Text encoding agnostic (for all eight bit text formats with null termination,
  including UTF-8).
- Simple syntax:
  - Substitution: `[generator>arguments]`  
    This invokes `generator`, which is a named function that is implementation
    defined, with the string `arguments`. The output is inserted into the
    current context.
  - Quote: `{quoted text}`  
    Substitutions within quoted text are not evaluated within the current
    context, but are inserted verbatim.
  - Escape: `\c`  
    Inserts the character `c` unprocessed into the current context.
  - No special syntax forms; everything else is done using substitutions and
    quotes.
- Basic built-in generators:
  - `:`  
    Copy generator. The default generator when template parsing begins. Simply
    outputs its argument string unadultered.
  - `=`  
    Lookup generator. Outputs the value of the property with the key specified
    by the argument string.
  - `!`  
    Nop generator. Does nothing with its argument string. Can be used for
    comments.
  - `has_prop`  
    Returns `#t` if the property named by the argument string exists, or `#f` if
    it doesn't.
  - `\\`  
    Escape generator. Outputs its string argument with all spaces escaped by
    backslash.
  - `macro`  
    Syntax: `[macro>NAME PARAMLIST BODY]`  
    Defines a parameterized macro identified by `NAME`. `PARAMLIST` is a
    semicolon separated list of property names that will be substituted with
    arguments provided when expanding the macro, and `BODY` is the contents on
    which the substitutions will operate.
  - `**`  
    Syntax: `[**>NAME ARGS]`  
    Expands the macro `NAME`, substituting each parameter in its parameter list
    with subsequent words from ARGS. Whitespace in arguments needs to be
    escaped.
  - `let`  
    Syntax: `[let>VARIABLE VALUE]`  
    Evaluates VARIABLE as a substitution, then sets the property named by the
    substitution result to the result of evaluating VALUE.
  - `for`  
    Syntax: `[for>LIST VARIABLE SUBSTITUTION]`  
    Iterates over a semicolon separated list of items (items containing
    semicolons need to escape them). For each iteration, the property `VARIABLE`
    will be set to the current list item, and can be accessed from within
    `SUBSTITUTION` if the latter is quoted.
  - `if`  
    Syntax: `[if>BOOLEAN T_SUBSTITUTION F_SUBSTITUTION]`  
    Evaluates `T_SUBSTITUTION` if `BOOLEAN` is the string `#t`, or
    `F_SUBSTITUTION` if it is `#f`. Any other value of `BOOLEAN` will result in
    a syntax error.
  - `not`  
    Evaluates its boolean argument, turning `#t` to `#f` and vice versa, and
    throwing syntax error on any other values.
  - `eq` `gt` `lt` `ge` `le`  
    Binary comparison generators -- equals, greater than, less than, greater or
    equal, and less or equal. Compares the first argument with the second. The
    arguments can be quoted substitutions, which will be evaluated.
  - `#`  
    Arithmetics generator. Implements a minimal infix arithmetics parser, that
    works with floating point numbers, and understands parentheses as well as
    the following set of operators:
    - `+`: Addition
    - `-`: Subtraction
    - `*`: Multiplication
    - `/`: Division
    - `%`: Modulo (remainder)
    - `^`: Power of

### Known omissions

- Currently there's a lack of built in string manipulation generators.
- No API documentation.

## Building/installing

The project is built using [CMake](https://cmake.org) (version 3.5 or higher),
and has a dependency on [cmocka](https://cmocka.org) for running the tests. The
build script will automatically download and build cmocka if possible.

1. Create a build directory (preferrably named `build`) in the repository root.
2. From within the build directory, issue `cmake .. -DCMAKE_BUILD_TYPE=Release`.
   - To create a debug build, change `CMAKE_BUILD_TYPE` to `Debug`.
   - To build a shared library, set the `SHARED_LIBRARY` variable from the
     command line: `cmake .. -DCMAKE_BUILD_TYPE=Debug -DSHARED_LIBRARY=1`
3. A project will now have been generated, based on whatever build system is the
   platform default. Build and install as you most commonly would (if in doubt,
   type `make`).
4. Upon successful completion, the library file will be located directly within
   the build directory.
   - Tests can be run by invoking the executables in the `tests` subdirectory.
   - There's a proof-of-concept standalone tool in the `standalone` folder,
     called `mintpl-cli`. It can be used to process templates that only make use
     of built-in generators.
5. To install, build the `install` target, with superuser privileges if
   necessary. This will install both the library and a `pkg-config` recipe. On
   Linux you may need to run `ldconfig` after installing.

## License

MinTPL is released under the [MIT License](LICENSE).

