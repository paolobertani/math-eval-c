//
//  test suite
//
//  Copyright (c) 2024 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include "matheval.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>



int main( int argc, char **argv );
void MathEvalRunTests( void );
void MathEvalTest( int lineNumber, MathEvaluationStatus expectedStatus, double expectedResult, char *expression );



int main( int argc, char **argv )
{
    MathEvalRunTests();
    return 0;
}


//
// Execute all tests.
//

void MathEvalRunTests( void )
{
    double b,
           e,
           r;

    // Plus and minus (unary/binary) mixing cases

    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,       "+2" );         // plus as unary operator
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,       "2+-2" );       // plus as binary operator, minus as unary: 2 + ( -2 )
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,       "2-+2" );       // vice-versa: 2 - ( -2 )
    MathEvalTest( __LINE__, MathEvaluationSuccess, 4,       "2--2" );       // minus as both binary and unary operator 2 - ( -2 )
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,       "+2-(+2)" );    // leading plus
    MathEvalTest( __LINE__, MathEvaluationSuccess, 6,       "+2*(+3)" );    //
    MathEvalTest( __LINE__, MathEvaluationSuccess, -3,      "1*-3" );       //
    MathEvalTest( __LINE__, MathEvaluationSuccess, 6,       "2*+3" );       //
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "-+3" );        // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "+-3" );        // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "2++2" );       // * two plus as consecutive binary and unary operators not allowed
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "2---2" );      // * three minus ? not allowed
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "--2" );        // * beginning with two minus ? no, a value is expected

    // Single numbers

    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,       "2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,       "02" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, .2,      ".2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -.2,     "-.2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1234,    "1234" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 12.34,   "12.34" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1200,    "12E2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.12,    "12E-2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 12,      "12E0" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 254,     "0xfE" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "12a0" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "12E2.5");      // * decimal exponent not allowed
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       ".-2" );        // * not a number

    // Round brackets

    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "(1)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 42,      "1+(2*(3+(4+5+6))-1)+6" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "(((((((((((1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,      "-(((((((((((1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "+(((((((((((1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,      "+(((((((((((-1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "-(((((((((((-1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "+2*(+-3)" );                   // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "1+(2*(3+(4+5+6))-1+6" );       // * missing close bracket
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "1+(2*(3+(4+5+6))-1))+6" );     // * too many close brackets
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "1+()" );                       // * empty expression
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       ".(((((((((((1)))))))))))" );   // *

    // Constants

    MathEvalTest( __LINE__, MathEvaluationSuccess, -M_PI,   "-pi" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, exp(1),  "e" );

    // Functions

    MathEvalTest( __LINE__, MathEvaluationSuccess, pow(6,5),        "pow(6,5)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, exp(2.5),        "exp(2.5)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, log(3)/log(2),   "log(2,3)" );   // base is the first parameter
    MathEvalTest( __LINE__, MathEvaluationSuccess, log(3),          "log(e,3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, log(4),          "log(4)" );     // log with one parameter (base e)
    MathEvalTest( __LINE__, MathEvaluationSuccess, sin(M_PI*.3),    "sin(pi*.3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, cos(M_PI*.3),    "cos(pi*.3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, tan(M_PI*.3),    "tan(pi*.3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, asin(.123),      "asin(.123)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, acos(.123),      "acos(.123)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, atan(.123),      "atan(.123)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 3,       "max(-1,2,3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,      "min(-1,2,3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,       "average(1,2,3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 20,      "avg(10,20,30)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 3,       "max(3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,      "min(-1)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,       "average(2)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 6.2,     "avg(6.2)" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "pow()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "exp()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "log()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "sin()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "cos()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "tan()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "asin()" );     // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "acos()" );     // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "atan()" );     // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "max()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "min()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "average()" );  // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "avg()" );      // * empty function
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "pow(1,2,3)" ); // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "exp(1,2,3)" ); // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "log(1,2,3)" ); // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "sin(4,5)" );   // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "cos(6,7)" );   // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "tan(8,9)" );   // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "asin(10,0)" ); // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "acos(1,2)" );  // * too many parameters
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "atan(3,4)" );  // * too many parameters

    // Factorial

    r = tgamma(1+3.456);

    MathEvalTest( __LINE__, MathEvaluationSuccess, 24,      "4!" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 24,      "+4!" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "0!" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, r,       "3.456!");      // gamma function
    MathEvalTest( __LINE__, MathEvaluationSuccess, -24,     "-(4!)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 24,      "fact(4)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,       "fact(0)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, r,       "fact(3.456)");
    MathEvalTest( __LINE__, MathEvaluationSuccess, -24,     "-fact(4)" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "(-4)!" );      // * factorial of negative number
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "!" );          // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "fact(-4)" );   // * factorial of negative number
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "fact()" );     // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,       "fact(1,2)" );  // *

    // Exponentiation

    b = 2;
    e = 1;
    e = -e / 3;
    r = pow(b,e);

    MathEvalTest( __LINE__, MathEvaluationSuccess, 8,               "2^3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, pow(2,3.2),      "2^3.2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, pow(2,81),       "2^3^4" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -27,             "(-3)^3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, r,               "2^(-1/3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.5/3,           "2^-1/3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, .5/3,            "(2^-1)/3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.5/3+1,         "2^-1/3+1" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -pow(2,-0.5),    "-1*2^(-1/2)" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,               "^3" );             // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,               "3^" );             // *
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,               "^" );              // *

    // Equivalent forms

    b = exp(1);
    e = 3.5;
    r = pow(b,e)-exp(3.5);

    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,   "e        -  exp(1)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, r,   "e^3.5    -  exp(3.5)" );   // result slightly different from 0 due to double internal representation
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,   "log(3.2) -  log(e,3.2)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,   "1.234!   -  fact(1.234)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,   "1.2^3.4  -  pow(1.2,3.4)" );

    // Operator precedencedence

    MathEvalTest( __LINE__, MathEvaluationSuccess, 14,  "2+3*4" );  // + < *
    MathEvalTest( __LINE__, MathEvaluationSuccess, 19,  "1+2*3^2" );// + < * < ^
    MathEvalTest( __LINE__, MathEvaluationSuccess, 10,  "1+3^2" );  //
    MathEvalTest( __LINE__, MathEvaluationSuccess, 15,  "2+3*4+1" );//
    MathEvalTest( __LINE__, MathEvaluationSuccess, 20,  "1+2*3^2+1");
    MathEvalTest( __LINE__, MathEvaluationSuccess, 11,  "1+3^2+1" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 24,  "2^3*3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 64,  "2^3!" );   // ^ < !
    MathEvalTest( __LINE__, MathEvaluationSuccess, -6,  "2*-3" );   // unary minus > *
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1.5,"3/-2" );   // unary minus > /
    MathEvalTest( __LINE__, MathEvaluationSuccess,1/9.0,"3^-2" );   // unary minus > ^

    // Unary minus precedence

    // Unary minus has lowest precedence
    MathEvalTest( __LINE__, MathEvaluationSuccess, -9,  "-3^2" );   // -(3^2)
    MathEvalTest( __LINE__, MathEvaluationSuccess, .25, "2^-2" );   // to make sense unary minus has highest precedence after a binary operator but...
    MathEvalTest( __LINE__, MathEvaluationSuccess,  1,  "5+-2^2" ); // ...has lowest precedence after `+`
    MathEvalTest( __LINE__, MathEvaluationSuccess, -4,  "-2^2" );   // -(2^.5)
    MathEvalTest( __LINE__, MathEvaluationSuccess, -6,  "-3!" );    // -(3!)


    // Whitespace (with some of the above)

    b = 2;
    e = 1;
    e = -e / 3;
    r = pow(b,e);

    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,           "  +  2  " );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,           "2+ - 2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0,           "2- +2" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 42,          "1+\t(2*(3 +\n\n( 4 +5+6) )-1)+6" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,           "((((((  ((( (( 1)))  ))) ))) ))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,          "  -  ((( (((( (((( 1)))))))))))" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, asin(.123),  "asin   (.123  )" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, acos(.123),  "acos(  .123)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, atan(.123),  "atan(.123  )" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 3,           "max  (-1,  2,3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -1,          "   min(-1,2 ,3   ) " );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2,           "average  (1, 2, 3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 24,          "4  !" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 1,           "  0 ! " );
    MathEvalTest( __LINE__, MathEvaluationSuccess, -24,         "-( 4 !)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, r,           "  2  ^(  -1 / 3)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.5/3,       " 2 ^ -1 / 3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, .5/3,        "(2 ^ -1 \n\n) / 3" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.5/3+1,     "2^-1/3+1" );
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,           "2+  +2" );   // *

    // Complicate expressions - tested with http://developer.wolframalpha.com/widgetbuilder/

    MathEvalTest( __LINE__, MathEvaluationSuccess, 0.999449080234467150824,    ".2^sin(log(e,3)*4*pi/8!)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2.417851639229258349412E24, "2^3^4-sin((pi*4!)/0.333)" );
    MathEvalTest( __LINE__, MathEvaluationSuccess, 2.940653537774626349957,    "log(6,atan((pi*4!)/0.333)*123.987)" );

    // Common exceptions (always catched, always raise error)

    MathEvalTest( __LINE__, MathEvaluationFailure, 0,   "1/0" );    // * division by zero
    MathEvalTest( __LINE__, MathEvaluationFailure, 0,   "(-1)!" );  // * negative factorial

    // Exceptions catched with eeval_catch_fp_exceptions set to true

    #if eeval_catch_fp_exceptions
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "(-2)^(-1/2)" );                          // * complex
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "(-3)^3.5" );                             // * complex
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "pow(-2,-1/2)");                          // * complex
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "(-2)^0.5" );                             // * complex
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "log(-6,atan((pi*4!)/0.333)*123.987)" );  // * complex
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "9^9^9" );                                // * huge
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "-(9^9^9)" );                             // * huge
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "average(-9^9^9,9^9^9" );                 // * huge
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "max(-(9^9^9),9^9^9" );                   // * huge
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "min(-(9^9^9),9^9^9" );                   // * huge
    MathEvalTest( __LINE__, MathEvaluationFailure, 0, "pow(9,pow(9,9))" );                      // * huge
    #endif

    // All tests passed

    printf( "All tests passed\n");
}



//
// Test function: compare expected status and result with those generated by EEvaluate().
//

void MathEvalTest( int lineNumber, MathEvaluationStatus expectedStatus, double expectedResult, char *expression )
{
    MathEvaluation       *matheval;
    MathEvaluationStatus status;
    double               result;

    matheval = MathEvaluationNew( expression );
    status = MathEvaluationPerform( matheval, &result );

    if( status == expectedStatus && result == expectedResult ) return;

    printf( "Test at line number %d failed\n\n", lineNumber );
    printf( "Expression: %s\n\n", expression );
    printf( "Expected status is: %s\n", expectedStatus == MathEvaluationSuccess ? "success" : "failure" );
    printf( "Test     status is: %s\n\n",       status == MathEvaluationSuccess ? "success" : "failure" );
    printf( "Expected result is: %f\n", expectedResult );
    printf( "Test     result is: %f\n\n", result );
    if( status == MathEvaluationFailure )
    {
        printf( "Error:\n" );
        MathEvaluationPrintError( matheval );
        printf( "\n" );
    }
    MathEvaluationDispose( matheval );
}