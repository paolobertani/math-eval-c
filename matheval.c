/*

math-eval
version 2.0

a math expression evaluator

math eval api to be included in your project

Copyright (c) 2024 Paolo Bertani - Kalei S.r.l.
Licensed under the FreeBSD 2-clause license

-------------------------------------------------------------------------------

FreeBSD 2-clause license

Copyright (c) 2016-2024, Paolo Bertani - Kalei S.r.l.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \AS IS\ AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-------------------------------------------------------------------------------

*/



#include "matheval.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>



// ********************
// * PUBLIC INTERFACE *
// ********************


//
// Returns a pointer to a new `MathEvaluation`
// or NULL in case of failure (cannot allocate
// memory).
//
// `expression` must be a null terminated string
//
// The function makes its own copy of the string
// passed
//
// The MathEvaluation returned must be freed with
// `MathEvaluationDispose`.
//

MathEvaluation* MathEvaluationNew( const char *expression )
{
    MathEvaluation *matheval;

    matheval = malloc( sizeof( MathEvaluation ) );

    if( ! matheval )
    {
        return matheval;
    }

    matheval->expression = malloc( strlen( expression ) + 1 );

    if( ! matheval->expression )
    {
        free( matheval );
        return NULL;
    }

    strcpy( (char *)matheval->expression, expression );

    matheval->params = NULL;
    matheval->cursor = NULL;
    matheval->result = 0.0;
    matheval->roundBracketsCount = 0;
    matheval->error = "";

    return matheval;
}



//
// Disposes MathEvaluation structure
// freeing memory
//

void MathEvaluationDispose( MathEvaluation *matheval )
{
    MathEvalParam *param,
                  *next;

    param = matheval->params;
    while( param != NULL )
    {
        next = param->next;
        free( param );
        param = next;
    }

    free( (void *) matheval->expression );

    free( matheval );
}



//
// Returns the result of a `MathEvaluation`
// If the evaluation has not been performed yet or ended in error
// then 0.0 is returned
//

double MathEvaluationGetResult( MathEvaluation *matheval )
{
    return matheval->result;
}



//
// Returns the description of the error occurred during the last
// call to `MathEvaluationPerform` or `MathEvaluationSetParam`
// And the approximate position the error was detected in the
// expression.
// If no error occurred or none of the functions were called
// then an empty string is returned
//

const char* MathEvaluationGetError( MathEvaluation *matheval, int *position )
{
    *position = ( matheval->cursor != NULL ) ? ( (int) ( matheval->cursor - matheval->expression ) ) : 0;

    return matheval->error;
}



//
// Set the value for the passed parameter.
// If the parameter exists the value is overridden.
// Parameter name can consist alphabetical characters
// and numbers and must not exceed 255 characters.
// Must start with a character and must not be
// an expression keyword or constant.
// Parameters are case sensitive.
//
// The function makes its own copy of the passed
// parameter name
//

MathEvaluationStatus MathEvaluationSetParam(
    MathEvaluation *matheval,
    const char *name,
    double value )
{
    MathEvalParam *param,
                  *current;

    unsigned long i;
    size_t len;
    char c;

    const char *reserved[] =
    {
        "e",
        "exp",
        "fact",
        "pi",
        "pow",
        "cos",
        "sin",
        "tan",
        "log",
        "max",
        "min",
        "acos",
        "asin",
        "atan",
        "average",
        "avg",
        "*" // `*` means "end of array"
    };

    // check for emptyness

    len = strlen( name );

    if( len == 0 )
    {
        matheval->error = "parameter name is empty";
        return MathEvaluationFailure;
    }

    if( len > 255 )
    {
        matheval->error = "parameter name exceeds 255 characters in length";
        return MathEvaluationFailure;
    }

    // check reserved keyw

    i = 0;
    while( true )
    {
        if( reserved[ i ][ 0 ] == '*' )
        {
            break;
        }

        if( strcmp( reserved[ i ], name ) == 0 )
        {
            matheval->error= "parameter name is a reserved keyword";
            return MathEvaluationFailure;
        }

        i++;
    }

    // is a-z A-Z 0-9, 1st char isn't number

    i = 0;
    while( true )
    {
        c = name[ i ];

        if( c == 0 )
        {
            break;
        }

        if( ( c >= '0' && c <= '9' && i > 0 ) ||
            ( c >= 'a' && c <= 'z'          ) ||
            ( c >= 'A' && c <= 'Z'          ) )
        {
            //
        }
        else
        {
            matheval->error = "invalid character in parameter name";
            return MathEvaluationFailure;
        }

        i++;
    }

    // check if param exists and in case overwrite val

    param = matheval->params;
    while( param != NULL )
    {
        if( strcmp( param->name, name ) == 0 )
        {
            param->value = value;
            return MathEvaluationSuccess;
        }

        param = param->next;
    }

    param = NULL;

    // alloc and init param, name is copied

    param = ( MathEvalParam * ) calloc( 1, sizeof( MathEvalParam ) );
    if( ! param )
    {
        matheval->error= "cannot allocate memory";
        return MathEvaluationFailure;
    }

    strcpy( (char *) param->name, name );
    param->len = len;
    param->value = value;
    param->next = NULL;

    // put param in list on top or before param with shorter name

    if( matheval->params == NULL )
    {
        matheval->params = param;
        return MathEvaluationSuccess;
    }

    if( matheval->params->len < len )
    {
        param->next = matheval->params;
        matheval->params = param;
        return MathEvaluationSuccess;
    }

    current = matheval->params;

    while( true )
    {
        if( current->next == NULL )
        {
            current->next = param;
            return MathEvaluationSuccess;
        }

        if( current->next->len < len )
        {
            param->next = current->next;
            current->next = param;
            return MathEvaluationSuccess;
        }

        current = current->next;
    }

    // this point is never reached

    return MathEvaluationSuccess;
}



// Evaluates an expression.
// The function returns a status of success or failure
// The result is in `*result`

MathEvaluationStatus MathEvaluationPerform(
    MathEvaluation *matheval,   // the MathEvaluation structure
    double         *result )    // RETURN: the result of the evaluation
{
    matheval->cursor = matheval->expression;
    matheval->roundBracketsCount = 0;
    matheval->result = 0;
    matheval->error = NULL;

    matheval->result = MathEvalProcessAddends( matheval, -1, true, false, NULL );

    *result = matheval->result;

    if( matheval->error )
    {
        *result = 0;
        return MathEvaluationFailure;
    }
    else
    {
        matheval->error = "";
        return MathEvaluationSuccess;
    }
}



// Utility function to print the error after an
// evaluation failed.
// Prints the error description, the expression
// and a caret under the expression approximately
// where the error occurred.

void MathEvaluationPrintError( MathEvaluation *matheval )
{
    if( matheval->error && strlen( matheval->error ) > 0 )
    {
        fprintf( stderr, "%s\n", matheval->error );
        if( matheval->cursor != NULL )
        {
            fprintf( stderr, "%s\n", matheval->expression );
            fprintf( stderr, "%*c^\n", (int)( matheval->cursor - matheval->expression ), ' ' );
        }
    }
}



// Debug utility function to dump params
// that have been set (if any)

void MathEvalDumpParams( MathEvaluation *matheval )
{
    MathEvalParam *p = matheval->params;
    int count = 1;
    while( p != NULL )
    {
        printf( "%d: %s = %f  ->  0x%lx\n", count, p->name, p->value, (unsigned long) p );
        p = p->next;
        count++;
        if(count>10) break;
    }
    printf( "\n---\n\n" );

}




// *********************
// * PRIVATE FUNCTIONS *
// *********************



// Evaluates a single value or expression A0 or
// sequence of 2 or more addends:
// A1 - A2 [ + A3 [ - A4 ... ] ]
// Addends can be a single values or expressions with
// higher precedence. In the second case the expression is evaluated first.
// "breakOn" parameters define cases where the function must exit.

double MathEvalProcessAddends(
    MathEvaluation *matheval,
    int64_t         breakOnRoundBracketsCount, // If open brackets count goes down to this count then exit;
    bool            breakOnETEof,              // exit if the end of the string '\0' is met;
    bool            breakOnETcom,              // exit if a comma is met;
    MathEvalToken  *tokenThatCausedBreak )     // if pointer is not null the token/symbol that caused the function to exit.
{
    MathEvalToken
           leftOp,
           rightOp;

    double value,
           result;


    // Let's pretend we already computed
    // 0 + ...

    result = 0;
    rightOp = MET_Sum;

    do
    {
        leftOp = rightOp;

        // [ Each addend A is treated as a (potential and higher-precedence)
        //   multiplication and evaluated as 1 * A with the function below ]
        value = MathEvalProcessFactors( matheval, 1, MET_Mul, false, &rightOp );
        if( matheval->error ) return 0;

        result = leftOp == MET_Sum ? ( result + value ) : ( result - value );

        // ...and go on as long there are sums ands subs.
    }
    while( rightOp == MET_Sum || rightOp == MET_Sub );

    // A round close bracket:
    // check for negative count.

    if( rightOp == MET_rbc )
    {
        matheval->roundBracketsCount--;
        if( matheval->roundBracketsCount < 0 )
        {
            matheval->error = "unexpected close round bracket";
            return 0;
        }
    }

    // Returns the token that caused the function to exit

    if( tokenThatCausedBreak )
    {
        *tokenThatCausedBreak = rightOp;
    }

    // Check if must exit

    if( ( matheval->roundBracketsCount == breakOnRoundBracketsCount ) || ( breakOnETEof && rightOp == MET_Eof ) || ( breakOnETcom && rightOp == MET_com ) )
    {
        if( eexception(result) )
        {
            matheval->error = "result is complex or too big";
            return 0;
        }

        return result;
    }

    // If not it's an error.

    switch( rightOp )
    {
        case MET_Eof:
            matheval->error = "unexpected end of expression";
            break;

        case MET_rbc:
            matheval->error = "unexpected close round bracket";
            break;

        case MET_com:
            matheval->error = "unexpeced comma";
            break;

        default:
            matheval->error = "unexpeced symbol";
            break;
    }

    return 0;
}



// Evaluates a sequence of 1 or more multiplies or divisions
// F1 [ * F2  [ / F3 [ * F4 ... ] ] ]
// Where Fn is a value or a higher precedence expression.

double MathEvalProcessFactors(
    MathEvaluation *matheval,
    double          leftValue, // The value (already fetched) on the left to be multiplied(divided);
    MathEvalToken   op,        // is it multiply or divide;
    bool            isExponent,// is an exponent being evaluated ?
    MathEvalToken  *leftOp )  // RETURN: factors are over, this is the next operator (token).
{
    MathEvalToken
           token,
           nextOp;

    double rightValue;
    double sign;

    do
    {
        rightValue = MathEvalProcessToken( matheval, &token );
        if( matheval->error ) return 0;

        // Unary minus or plus ?
        // store the sign and get the next token

        if( token == MET_Sub )
        {
            sign = -1;
            rightValue = MathEvalProcessToken( matheval, &token );
            if( matheval->error ) return 0;
        }
        else if( token == MET_Sum )
        {
            sign = 1;
            rightValue = MathEvalProcessToken( matheval, &token );
            if( matheval->error ) return 0;
        }
        else
        {
            sign = 1;
        }

        // Open round bracket?
        // The expression between brackets is evaluated.

        if( token == MET_rbo )
        {
            matheval->roundBracketsCount++;

            rightValue = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;

            token = MET_Val;
        }

        // A function ?

        if( token == MET_Cos || token == MET_Sin || token == MET_Tan || token == MET_ASi || token == MET_ACo || token == MET_ATa || token == MET_Fac || token == MET_Log || token == MET_Exp || token == MET_Pow || token == MET_Max || token == MET_Min || token == MET_Avg )
        {
            rightValue = MathEvalProcessFunction( matheval, token );
            if( matheval->error ) return 0;

            token = MET_Val;
        }

        // Excluded previous cases then
        // the token must be a number.

        if( token != MET_Val )
        {
            matheval->error = "expected value";
            return 0;
        }

        // Get beforehand the next token
        // to see if it's an exponential or factorial operator

        MathEvalProcessToken( matheval, &nextOp );
        if( matheval->error ) return 0;

        // Unary minus precedence (highest/lowest) affects this section of code

        if( nextOp == MET_Fct )
        {
            rightValue = MathEvalProcessFactorial( matheval, rightValue, &nextOp );
            if( matheval->error ) return 0;
        }

        if( nextOp == MET_Exc )
        {
            rightValue = MathEvalProcessExponentiation( matheval, rightValue, &nextOp );
            if( matheval->error ) return 0;
        }

        // multiplication/division is finally
        // calculated

        if( op == MET_Mul )
        {
            leftValue = leftValue * rightValue * sign;
        }
        else
        {
            if( rightValue == 0 )
            {
                matheval->error = "division by zero";
                return 0;
            }
            leftValue = leftValue / rightValue * sign;
        }

        if( eexception( leftValue ) )
        {
            matheval->error = "result is too big";
            return 0;
        }

        // The next operator has already been fetched.

        op = nextOp;

        // Go on as long multiply or division operators are met...
        // ...unless an exponent is evaluated
        // (because exponentiation ^ operator have higher precedence)
    }
    while( ( op == MET_Mul || op == MET_Div ) && ! isExponent );

    *leftOp = op;

    return leftValue;
}



// Evaluates the expession(s) (comma separated if multiple)
// inside the round brackets then computes the function
// specified by the token `func`.

double MathEvalProcessFunction( MathEvaluation *matheval, MathEvalToken func )
{
    double   result,
             result2;

    uint16_t count;

    MathEvalToken
             tokenThatCausedBreak,
             token;

    // Eat an open round bracket and count it

    MathEvalProcessToken( matheval, &token );
    if( matheval->error ) return 0;

    if( token != MET_rbo )
    {
        matheval->error = "expected open round bracket after function name";
        return 0;
    }

    matheval->roundBracketsCount++;

    switch( func )
    {
        case MET_Sin:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = sin( result );
            break;

        case MET_Cos:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = cos( result );
            break;

        case MET_Tan:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = tan( result );
            break;

        case MET_ASi:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = asin( result );
            break;

        case MET_ACo:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = acos( result );
            break;

        case MET_ATa:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = atan( result );
            break;

        case MET_Fac:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            if( result < 0 )
            {
                matheval->error = "attempt to mathevaluate factorial of negative number";
            }
            else
            {
                result = tgamma( 1 + result );
            }
            break;

        case MET_Exp:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = exp( result );
            break;

        case MET_Pow:
            result = MathEvalProcessAddends( matheval, -1, false, true, NULL );
            if( matheval->error ) return 0;
            result2 = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
            if( matheval->error ) return 0;
            result = pow( result, result2 );
            break;

        case MET_Log:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( matheval->error ) return 0;
            if( tokenThatCausedBreak == MET_rbc )
            {
                // log(n) with one parameter
                result = log( result );
            }
            else
            {
                result2 = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, false, NULL );
                if( matheval->error ) return 0;
                result = log( result2 ) / log( result );
            }
            break;

        case MET_Max:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( matheval->error ) return 0;
            while( tokenThatCausedBreak == MET_com )
            {
                result2 = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( matheval->error ) return 0;

                if( result2 > result )
                {
                    result = result2;
                }
            }
            break;

        case MET_Min:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( matheval->error ) return 0;
            while( tokenThatCausedBreak == MET_com )
            {
                result2 = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( matheval->error ) return 0;

                if( result2 < result )
                {
                    result = result2;
                }
            }
            break;

        case MET_Avg:
            result = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( matheval->error ) return 0;
            count = 1;
            while( tokenThatCausedBreak == MET_com )
            {
                result2 = MathEvalProcessAddends( matheval, matheval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( matheval->error ) return 0;

                result += result2;
                count++;
            }
            result = result / (double)count;
            break;

        default:
            result = 0;
            break;
    }

    if( eexception( result ) )
    {
        matheval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates an exponentiation.

double MathEvalProcessExponentiation( MathEvaluation *matheval,
                                      double          base,     // The base has already been fetched;
                                      MathEvalToken  *rightOp ) // RETURN: the token (operator) that follows.
{
    double exponent,
           result;

    exponent = MathEvalProcessFactors( matheval, 1, MET_Mul, true, rightOp );
    if( matheval->error ) return 0;

    result = pow( base, exponent );
    if( eexception( result ) )
    {
        matheval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates a factorial using the Gamma function.

double MathEvalProcessFactorial( MathEvaluation *matheval,
                                 double          value,     // The value to compute has already been fetched;
                                 MathEvalToken  *rightOp )  // RETURN: the token (operator) that follows.
{
    double result;

    if( value < 0 )
    {
        matheval->error = "attempt to mathevaluate factorial of negative number";
        *rightOp = MET_Err;
        return 0;
    }

    result = tgamma( value + 1 );

    if( eexception( result ) )
    {
        matheval->error = "result is complex or too big";
        return 0;
    }

    MathEvalProcessToken( matheval, rightOp );
    if( matheval->error ) return 0;

    return result;
}



// Parses the next token and advances the cursor.
// The function returns a number if the token is a value a const. or a param.
// Whitespace is ignored.

double MathEvalProcessToken( MathEvaluation *matheval,
                             MathEvalToken  *token ) // RETURN: the token.
{
    MathEvalToken
           t;
    double v;
    MathEvalParam
          *param;

    t = MET_Blk;
    v = 0;

    while( t == MET_Blk )
    {
        // value maybe

        if( ( *matheval->cursor >= '0' && *matheval->cursor <= '9' ) || *matheval->cursor == '.' )
        {
            v = MathEvalProcessValue( matheval );
            if( matheval->error )
            {
                t = MET_Err;
                return t;
            }
            else
            {
                t = MET_Val;
            }
            break;
        }
        else
        {
            // parameter maybe

            param = matheval->params;
            while( param != NULL )
            {
                if( strncmp( param->name, matheval->cursor, param->len ) == 0 )
                {
                    *token = MET_Val;
                    matheval->cursor += param->len;
                    return param->value;
                }

                param = param->next;
            }

            // token maybe

            switch( *matheval->cursor )
            {
                case '\n':
                case '\r':
                case '\t':
                case ' ':
                    t = MET_Blk;
                    matheval->cursor++;
                    break;

                case '+':
                    MathEvalProcessPlusToken( matheval, &t);
                    break;

                case '-':
                    t = MET_Sub;
                    matheval->cursor++;
                    break;

                case '*':
                    t = MET_Mul;
                    matheval->cursor++;
                    break;

                case '/':
                    t = MET_Div;
                    matheval->cursor++;
                    break;

                case '^':
                    t = MET_Exc;
                    matheval->cursor++;
                    break;

                case '!':
                    t = MET_Fct;
                    matheval->cursor++;
                    break;

                case '(':
                    t = MET_rbo;
                    matheval->cursor++;
                    break;

                case ')':
                    t = MET_rbc;
                    matheval->cursor++;
                    break;

                case '\0':
                    t = MET_Eof;
                    matheval->cursor++;
                    break;

                case ',':
                    t = MET_com;
                    matheval->cursor++;
                    break;

                case 'e':
                    if( strncmp( matheval->cursor, "exp", 3 ) == 0 )
                    {
                        t = MET_Exp;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        v = exp( 1 );
                        t = MET_Val;
                        matheval->cursor++;
                    }
                    break;

                case 'f':
                    if( strncmp( matheval->cursor, "fact", 4 ) == 0 )
                    {
                        t = MET_Fac;
                        matheval->cursor += 4;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 'p':
                    if( strncmp( matheval->cursor, "pi", 2 ) == 0 )
                    {
                        v = M_PI;
                        t = MET_Val;
                        matheval->cursor += 2;
                    }
                    else if( strncmp( matheval->cursor, "pow", 3 ) == 0 )
                    {
                        t = MET_Pow;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 'c':
                    if( strncmp( matheval->cursor, "cos", 3 ) == 0 )
                    {
                        t = MET_Cos;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 's':
                    if( strncmp( matheval->cursor, "sin", 3 ) == 0 )
                    {
                        t = MET_Sin;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 't':
                    if( strncmp( matheval->cursor, "tan", 3 ) == 0 )
                    {
                        t = MET_Tan;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 'l':
                    if( strncmp( matheval->cursor, "log", 3 ) == 0 )
                    {
                        t = MET_Log;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 'm':
                    if( strncmp( matheval->cursor, "max", 3 ) == 0 )
                    {
                        t = MET_Max;
                        matheval->cursor += 3;
                    }
                    else if( strncmp( matheval->cursor, "min", 3 ) == 0 )
                    {
                        t = MET_Min;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;

                case 'a':
                    if( strncmp( matheval->cursor, "asin", 4 ) == 0 )
                    {
                        t = MET_ASi;
                        matheval->cursor += 4;
                    }
                    else if( strncmp( matheval->cursor, "acos", 4 ) == 0 )
                    {
                        t = MET_ACo;
                        matheval->cursor += 4;
                    }
                    else if( strncmp( matheval->cursor, "atan", 4 ) == 0 )
                    {
                        t = MET_ATa;
                        matheval->cursor += 4;
                    }
                    else if( strncmp( matheval->cursor, "average", 7 ) == 0 )
                    {
                        t = MET_Avg;
                        matheval->cursor += 7;
                    }
                    else if( strncmp( matheval->cursor, "avg", 3 ) == 0 )
                    {
                        t = MET_Avg;
                        matheval->cursor += 3;
                    }
                    else
                    {
                        t = MET_Err;
                    }
                    break;


                default:
                    t = MET_Err;
                    break;
            }
        }
    }

    if( t == MET_Err )
    {
        matheval->error = "unexpected symbol";
    }

    *token = t;

    return v;
}



// Parses what follows a (already fetched) plus token
// ensuring that two consecutive plus are not present.
// Expressions such as 2++2 (binary plus
// followed by unitary plus) are not allowed.
// Advances the cursor.
// Always returns 0.

double MathEvalProcessPlusToken( MathEvaluation *matheval, MathEvalToken *token )
{
    do
    {
        matheval->cursor++;
    } while( *matheval->cursor == ' ' || *matheval->cursor == '\n' || *matheval->cursor == '\r' || *matheval->cursor == '\t' );

    if( *matheval->cursor == '+' )
    {
        *token = MET_Err;
    }
    else
    {
        *token = MET_Sum;
    }

    return 0;
}



// Parses a number and advances the cursor.
// The cursor is positioned after an eventually
// `+` or `-` operator that comes before the value.

double MathEvalProcessValue( MathEvaluation *matheval )
{
    char   *endptr;
    double value;

    value = strtod( matheval->cursor, &endptr );

    if( endptr == matheval->cursor )
    {
        matheval->error = "expected value";
        value = 0;
    }
    else
    {
        matheval->cursor = endptr;

        if( eexception( value ) )
        {
            matheval->error = "value is too big";
            return 0;
        }
    }

    return value;
}