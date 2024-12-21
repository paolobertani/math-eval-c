math-eval - A math expression evaluator in C
===

`version 2.1`

**math-eval** is a library to evaluate a mathematical expressions: supply the espression as a C string and get the result as a `double` value.

It comes with a command line tool too ready to be used in the terminal.


&nbsp;


TL;DR
===
Example: *Two raised to the power of three plus the sine of* ***π*** *divided by eight factorial, all multiplied by ten to the power of three.*

**in your project:**

```
#include "matheval.h"

main()
{
    const char     *expression = "fooBAR^3 + sin(pi/8!) * 10e3";
    double          result;
    MathEvaluation *me;

    me = MathEvaluationNew( expression, &result );

    MathEvaluationSetParam( me, "fooBAR", 2 );

    MathEvaluationPerform( me, &result );

    printf( "%f.6\n", result); // ==> 8.779165

    MathEvaluationDispose( me );
}
```
&nbsp;

**in the terminal:**

```
$ matheval -p 6 '2^3 + sin(pi/8!) * 10e3'
8.779165
```






&nbsp;


Building the Command Line Tool
===

**Build:**

`$ make`

Compiles the code and put both the executable `matheval` and the tests `matheval-test` in the current working directory.

Tests execution is run at the end of the build process.

**Execution**

Try it on the command line:

```
$ ./matheval -p 5 'cos(-pi)'
-1.00000
```

**Installation:**

`$ sudo make install`

Compiles the code and put the executable into `/usr/local/bin`

Requires root privileges.

If `matheval` is already present on the destination directory you'll be prompted for confirmation before overwriting.

&nbsp;

`$ sudo make clean`

Removes the executable from `/usr/local/bin`

Requires root privileges.

&nbsp;

Command line tool usage
=====

`$ matheval`

Prints usage info and license.

&nbsp;

`$ matheval [-p n] expr`

Evaluates the expression `expr` and prints the result.

Optionally the `-p` flag specifies that the output value should be printed with `n` decimal digits (default is **3**). `n` must be between 0 and 20 (included).

When invoked from the terminal it's advisable
to place the expression between **'**single**'** quotes

&nbsp;

**Supported operators are:**

`+` plus

`-` minus

`*` multiplication

`/` division

`^` exponentiation (*right associative as it should be*)

`!` factorial (using Gamma function)

&nbsp;


**Supported functions are:**

`sin(r)`  sine

`cos(r)`  cosine

`tan(r)`  tangent

`asin(n)` arcsin

`acos(n)` arccos

`atan(n)` arctan

`fact(n)` factorial of `n`. Equivalent to `n!`.

`exp(n)` base **e** exponential function of `n`. Equivalent to `e^n`.

`pow(b, n)` `b` to the power of `n`. Equivalent to `b^n`.

`log(n)` natural logarithm of `n` (with base **e**)

`log(b, n)` logarithm of `n` with base `b`

`max(n1, n2, n3, ...)` maximum of one or more numbers

`min(n1, n2, n3, ...)` minimum of one or more numbers

`average(n1, n2, ...)` or `avg(n1, ...)` average of one or more numbers

&nbsp;

**Numbers can be expressed as follows:**

`0.123`  or  `.123`  or  `12.3E-2`  etc..

&nbsp;

**Recognized constants:**

`e`  euler number

`pi` **Pi**

&nbsp;

**Parentheses:**

Use round brackets `(` `)` to nest expressions.


&nbsp;

**Operators precedence:**

Highest to lowest:

`(` `)`

`!`

`^`

`-` unary minus, right associative

`*` `/`

`+` `-`

Unary minus has **lower** precedence than exponentiation (like in Python); for example:

`- 3 ^ 2` is evaluated as `- (3  ^ 2 )` = `-9`

However it is advisable to use parentheses when the expression is abiguous (ex. JS Chrome raises an arror for `-3**2`.

&nbsp;

**Whitespace:**

Whitespace, tabs and newlines are ignored.

&nbsp;

**Errors:**

If the expression is malformed or it is correct but would result in math operations not allowed (division by zero) or operations that would generate a complex number or too big number (overflow) then an error message is displayed as long with the expression and a caret indicating the point where the error occurred.

&nbsp;

Examples
========

    $ matheval -p 0 '2+2'
    4
&nbsp;

    $ matheval '-.3E2 * sin( -.5 * pi ) - 3 * log( e, e^1E1 ) + 3!'
    6.000

&nbsp;

    $ matheval '(-2)^.5'
    result is complex or too big
    (-2)^.5
          ^

&nbsp;

    $ matheval '(2))^.5'
    unexpected close round bracket
    (2))^.5
        ^

&nbsp;

Embedding math-eval in your project
===============================

Embedding **math-eval** is trivial. Just add `matheval.h`, `matheval-private.h` and `matheval.c` to your project, `#include "matheval.h"`.

&nbsp;
## Structures
&nbsp;

`MathEvaluation`

Stores the data needed to perform a math evaluation.

Normally you'll use a pointer to it: `MathEvaluation *mathEvaluation;`

&nbsp;
### Functions
&nbsp;

### MathEvaluationNew

`MathEvaluation *MathEvaluationNew( const char *expression );`

Allocate and initialize the MathEvaluation structure; `expression` is copied.
In the unlikely case that memory allocation fails then NULL is returned.

&nbsp;

### MathEvaluationSetParam

`MathEvaluationStatus MathEvaluationSetParam( MathEvaluation *mathEvaluation,`
`                                             const char     *name,`
`                                             double         value );`

Define a named parameter and its value.
`name` is copied.
Several defined parameter may be added to the MathEvaluation.
If the parameter already exists its value is overridden.
The parameter name must begin with a letter.
Prameter name is case-sensitive.
Allowed characters are `A-Za-z0-9`.
The parameter must not be a reserved keyword (ex. `cos`, `pi`...)
The returned value is `MathEvaluationSuccess` or `MathEvaluationFailure`.

&nbsp;

### MathEvaluationPerform

`MathEvaluationStatus MathEvaluationPerform( MathEvaluation *mathEvaluation,`
`                                            double         *result );`

Performs the math expression evaluation.

`MathEvaluationStatus MathEvaluationSetParam( MathEvaluation *mathEvaluation,`
`                                             const char     *name,`
`                                             double         value );`

Define a named parameter and its value.
`name` is copied.
Several defined parameter may be added to the MathEvaluation.
If the parameter already exists its value is overridden.
The parameter name must begin with a letter.
Allowed characters are `A-Za-z0-9`.
The parameter must not be a reserved keyword (ex. `cos`, `pi`...)
The returned value is `MathEvaluationSuccess` or `MathEvaluationFailure`.

&nbsp;

### MathEvaluationGetError

`const char *MathEvaluationGetError( MathEvaluation *mathEvaluation,
                                     int            *position );`

In case of *MathEvaluationFailure* returns a string explaining the error.
If the error occurred during the evaluation `MathEvaluationPerform` then the approximate position of the error in the expression is returned.

&nbsp;

### MathEvaluationPrintError

`void MathEvaluationPrintError( MathEvaluation *mathEvaluation );`

Utility function that prints to stdout the error.

&nbsp;

### MathEvaluationGetResult

`double MathEvaluationGetResult( MathEvaluation *mathEvaluation );`

Utility function that returns the result of the last evaluation performed.

&nbsp;

### MathEvaluationDispose

`void MathEvaluationDispose( MathEvaluation *mathEvaluation );`

Free up memory.

&nbsp;

A note about the algorithm
==========================

**Floating point exceptions catching**

Exceptions are catched with the following naive macro (in `matheval.h`):

    #define eexception(n) (isnan(n)||(n)==HUGE_VAL||(n)==INFINITY||(n)==-HUGE_VAL||(n)==-INFINITY)

...that is expected to evaluate as `true` when `n` is the result of a math operation that caused overflow or a number that cannot be represented (ex. a complex number).

Note that this approach is *not guaranted 100% to work on every implementation/platform* as `isnan()` and/or some of the constants used above may not be implemented/defined (or may have a different name).

In that case a compilation error should occurr; furthermore **the test suite checks if floating point exceptions are properly catched**. If build or test fail then the macro will need to be adjusted (or disabled in case you don't mind catching floating point exceptions).

Exception catching can be turned off setting to false the constant in `matheval.h`:

    #define math-eval_catch_fp_exceptions false

Division by zero and factorial of negative value are explicitly checked **before** the operation.

**The double oddity (as usual)**

As numbers are internally represented and handled with variables of type `double` a small inaccuracy occurrs in some operations.

For example:

    $ matheval -p 16 '4.456 -1 -3.456'
    0.0000000000000004

This is an **expected behavior** and is due to the internal representation of the `double` type.

&nbsp;

Contact
=======

visit **[Kalei](https://www.kalei.it)**

&nbsp;

FreeBSD 2-clause license
========================

**Copyright (c) 2024, Paolo Bertani - Kalei S.r.l.**

**All rights reserved.**

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.














