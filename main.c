//
//  math-eval
//  version 2.0
//
//  command line tool
//
//  Copyright (c) 2016 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include "math-eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Evaluates the expression passed as parameter

int main( int argc, const char * argv[] )
{
    MathEvaluation  *eval;
    double          result;

    long int        precision;
    char            *endptr;

    precision = 3; // default

    const char *usage =
    "\n"
    "usage:\n"
    "\n"
    "eeval [[-p prec] 'expr']\n"
    "\n"
    "where expr is the expression to evaluate\n"
    "and optional prec is the number of decimal digits\n"
    "to be printed in the output (between 0 and 20 included)\n"
    "\n"
    "when invoked from the shell it's recomended\n"
    "to place the expression between 'single' quotes\n"
    "\n"
    "if invoked without parameters usage and license info is printed\n"
    "\n"
    "supported operator are:\n"
    "\n"
    "+ plus\n"
    "- minus\n"
    "* multiplication\n"
    "/ division\n"
    "^ exponentiation\n"
    "! factorial (using Gamma function)\n"
    "\n"
    "supported function are:\n"
    "\n"
    "sin(r)  sine\n"
    "cos(r)  cosine\n"
    "tan(r)  tangent\n"
    "asin(n) arcsin\n"
    "acos(n) arccos\n"
    "atan(n) arctan\n"
    "fact(n) factorial of n; equivalent to n!\n"
    "exp(n) equivalent to e^n\n"
    "pow(b, n) equivalent to b^n\n"
    "log(n) natural logarithm of n (base e)\n"
    "log(b, n) logarithm of n with base b\n"
    "max(n1, n2, n3, ...) maximum of one or more numbers\n"
    "min(n1, n2, n3, ...) minimum of one or more numbers\n"
    "average(n1, n2, ...) average of one or more numbers\n"
    "avg(n1, n2, ...) abbreviated form of the above\n"
    "\n"
    "numbers can be expressed as follows:\n"
    "\n"
    "0.123  or  .123  or  12.3E-2  etc..\n"
    "\n"
    "recognized constants are:\n"
    "\n"
    "e  euler number\n"
    "pi Pi\n"
    "\n"
    "use round brackets to nest expressions\n"
    "whitespace, tabs and newlines are ignored\n"
    "\n";

    if( argc == 2 || argc == 4 )
    {
        // Three arguments are passed so it is expected to find `-p` option
        // followed by the required precision.

        if( argc == 4 )
        {
            if( strncmp( argv[1], "-p", 3 ) == 0)
            {
                precision = strtol( argv[ 2 ], &endptr, 10 );
                if( endptr == argv[2] || *endptr != '\0' )
                {
                    fprintf( stderr, "value specified for precision parameter is not a integer number\n" );
                    exit( 1 );
                }
                if( precision < 0 || precision > 20 )
                {
                    fprintf( stderr, "value specified for precision parameter must be between 0 and 20 (included)\n" );
                    exit( 1 );
                }
            }
            else
            {
                fprintf( stderr, "%s", usage );
                exit( 1 );
            }
        }


        // The passed expression is evaluated.
        // If evaluation succeeds the result is printed.
        // If fails then prints the error.

        eval = MathEvaluationNew( argv[ argc - 1 ] );
        if( ! eval )
        {
            printf( "cannot allocate memory\n" );
            exit(1);
        }

        if( MathEvaluationPerform( eval, &result ) == MathEvaluationSuccess )
        {
            printf( "%.*f\n", (int)precision, result );
        }
        else
        {
            MathEvaluationPrintError( eval );
            exit( 0 );
        }

        MathEvaluationDispose( eval );
    }
    else
    {
        fprintf( stderr, "%s", usage );
        exit( 1 );
    }

    return 0;
}

