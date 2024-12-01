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



#include "math-eval.h"

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
    MathEvaluation *eval;

    eval = malloc( sizeof( MathEvaluation ) );

    if( ! eval )
    {
        return eval;
    }

    eval->expression = malloc( strlen( expression ) + 1 );

    if( ! eval->expression )
    {
        free( eval );
        return NULL;
    }

    strcpy( (char *)eval->expression, expression );

    eval->params = NULL;
    eval->cursor = NULL;
    eval->result = 0.0;
    eval->roundBracketsCount = 0;
    eval->error = "";

    return eval;
}



//
// Disposes MathEvaluation structure
// freeing memory
//

void MathEvaluationDispose( MathEvaluation *eval )
{
    MathEvalParam *param,
                  *next;

    param = eval->params;
    while( param != NULL )
    {
        next = param->next;
        free( param );
        param = next;
    }

    free( (void *) eval->expression );

    free( eval );
}



//
// Returns the result of a `MathEvaluation`
// If the evaluation has not been performed yet or ended in error
// then 0.0 is returned
//

double MathEvaluationGetResult( MathEvaluation *eval )
{
    return eval->result;
}



//
// Returns the description of the error occurred during the last
// call to `MathEvaluationPerform` or `MathEvaluationSetParam`
// And the approximate position the error was detected in the
// expression.
// If no error occurred or none of the functions were called
// then an empty string is returned
//

const char* MathEvaluationGetError( MathEvaluation *eval, int *position )
{
    *position = ( eval->cursor != NULL ) ? ( (int) ( eval->cursor - eval->expression ) ) : 0;

    return eval->error;
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
    MathEvaluation *eval,
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
        eval->error = "parameter name is empty";
        return MathEvaluationFailure;
    }

    if( len > 255 )
    {
        eval->error = "parameter name exceeds 255 characters in length";
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
            eval->error= "parameter name is a reserved keyword";
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
            eval->error = "invalid character in parameter name";
            return MathEvaluationFailure;
        }

        i++;
    }

    // check if param exists and in case overwrite val

    param = eval->params;
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
        eval->error= "cannot allocate memory";
        return MathEvaluationFailure;
    }

    strcpy( (char *) param->name, name );
    param->len = len;
    param->value = value;
    param->next = NULL;

    // put param in list on top or before param with shorter name

    if( eval->params == NULL )
    {
        eval->params = param;
        return MathEvaluationSuccess;
    }

    if( eval->params->len < len )
    {
        param->next = eval->params;
        eval->params = param;
        return MathEvaluationSuccess;
    }

    current = eval->params;

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
    MathEvaluation *eval,       // the MathEvaluation structure
    double         *result )    // RETURN: the result of the evaluation
{
    eval->cursor = eval->expression;
    eval->roundBracketsCount = 0;
    eval->result = 0;
    eval->error = NULL;

    eval->result = MathEvalProcessAddends( eval, -1, true, false, NULL );

    *result = eval->result;

    if( eval->error )
    {
        *result = 0;
        return MathEvaluationFailure;
    }
    else
    {
        eval->error = "";
        return MathEvaluationSuccess;
    }
}



// Utility function to print the error after an
// evaluation failed.
// Prints the error description, the expression
// and a caret under the expression approximately
// where the error occurred.

void MathEvaluationPrintError( MathEvaluation *eval )
{
    if( eval->error && strlen( eval->error ) > 0 )
    {
        fprintf( stderr, "%s\n", eval->error );
        if( eval->cursor != NULL )
        {
            fprintf( stderr, "%s\n", eval->expression );
            fprintf( stderr, "%*c^\n", (int)( eval->cursor - eval->expression ), ' ' );
        }
    }
}



// Debug utility function to dump params
// that have been set (if any)

void MathEvalDumpParams( MathEvaluation *eval )
{
    MathEvalParam *p = eval->params;
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
    MathEvaluation *eval,
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
    rightOp = ETSum;

    do
    {
        leftOp = rightOp;

        // [ Each addend A is treated as a (potential and higher-precedence)
        //   multiplication and evaluated as 1 * A with the function below ]
        value = MathEvalProcessFactors( eval, 1, ETMul, false, &rightOp );
        if( eval->error ) return 0;

        result = leftOp == ETSum ? ( result + value ) : ( result - value );

        // ...and go on as long there are sums ands subs.
    }
    while( rightOp == ETSum || rightOp == ETSub );

    // A round close bracket:
    // check for negative count.

    if( rightOp == ETrbc )
    {
        eval->roundBracketsCount--;
        if( eval->roundBracketsCount < 0 )
        {
            eval->error = "unexpected close round bracket";
            return 0;
        }
    }

    // Returns the token that caused the function to exit

    if( tokenThatCausedBreak )
    {
        *tokenThatCausedBreak = rightOp;
    }

    // Check if must exit

    if( ( eval->roundBracketsCount == breakOnRoundBracketsCount ) || ( breakOnETEof && rightOp == ETEof ) || ( breakOnETcom && rightOp == ETcom ) )
    {
        if( eexception(result) )
        {
            eval->error = "result is complex or too big";
            return 0;
        }

        return result;
    }

    // If not it's an error.

    switch( rightOp )
    {
        case ETEof:
            eval->error = "unexpected end of expression";
            break;

        case ETrbc:
            eval->error = "unexpected close round bracket";
            break;

        case ETcom:
            eval->error = "unexpeced comma";
            break;

        default:
            eval->error = "unexpeced symbol";
            break;
    }

    return 0;
}



// Evaluates a sequence of 1 or more multiplies or divisions
// F1 [ * F2  [ / F3 [ * F4 ... ] ] ]
// Where Fn is a value or a higher precedence expression.

double MathEvalProcessFactors(
    MathEvaluation *eval,
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
        rightValue = MathEvalProcessToken( eval, &token );
        if( eval->error ) return 0;

        // Unary minus or plus ?
        // store the sign and get the next token

        if( token == ETSub )
        {
            sign = -1;
            rightValue = MathEvalProcessToken( eval, &token );
            if( eval->error ) return 0;
        }
        else if( token == ETSum )
        {
            sign = 1;
            rightValue = MathEvalProcessToken( eval, &token );
            if( eval->error ) return 0;
        }
        else
        {
            sign = 1;
        }

        // Open round bracket?
        // The expression between brackets is evaluated.

        if( token == ETrbo )
        {
            eval->roundBracketsCount++;

            rightValue = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;

            token = ETVal;
        }

        // A function ?

        if( token == ETCos || token == ETSin || token == ETTan || token == ETASi || token == ETACo || token == ETATa || token == ETFac || token == ETLog || token == ETExp || token == ETPow || token == ETMax || token == ETMin || token == ETAvg )
        {
            rightValue = MathEvalProcessFunction( eval, token );
            if( eval->error ) return 0;

            token = ETVal;
        }

        // Excluded previous cases then
        // the token must be a number.

        if( token != ETVal )
        {
            eval->error = "expected value";
            return 0;
        }

        // Get beforehand the next token
        // to see if it's an exponential or factorial operator

        MathEvalProcessToken( eval, &nextOp );
        if( eval->error ) return 0;

        // Unary minus precedence (highest/lowest) affects this section of code

        if( nextOp == ETFct )
        {
            #if eeval_unary_minus_has_highest_precedence
                rightValue = MathEvalProcessFactorial( eval, rightValue * sign, &nextOp );
                sign = 1;
            #else
                rightValue = MathEvalProcessFactorial( eval, rightValue, &nextOp );
            #endif
            if( eval->error ) return 0;
        }

        if( nextOp == ETExc )
        {
            #if eeval_unary_minus_has_highest_precedence
                rightValue = MathEvalProcessExponentiation( eval, rightValue * sign, &nextOp );
                sign = 1;
            #else
                rightValue = MathEvalProcessExponentiation( eval, rightValue, &nextOp );
            #endif
            if( eval->error ) return 0;
        }

        // multiplication/division is finally
        // calculated

        if( op == ETMul )
        {
            leftValue = leftValue * rightValue * sign;
        }
        else
        {
            if( rightValue == 0 )
            {
                eval->error = "division by zero";
                return 0;
            }
            leftValue = leftValue / rightValue * sign;
        }

        if( eexception( leftValue ) )
        {
            eval->error = "result is too big";
            return 0;
        }

        // The next operator has already been fetched.

        op = nextOp;

        // Go on as long multiply or division operators are met...
        // ...unless an exponent is evaluated
        // (because exponentiation ^ operator have higher precedence)
    }
    while( ( op == ETMul || op == ETDiv ) && ! isExponent );


    *leftOp = op;

    return leftValue;
}



// Evaluates the expession(s) (comma separated if multiple)
// inside the round brackets then computes the function
// specified by the token `func`.

double MathEvalProcessFunction( MathEvaluation *eval, MathEvalToken func )
{
    double   result,
             result2;

    uint16_t count;

    MathEvalToken
             tokenThatCausedBreak,
             token;

    // Eat an open round bracket and count it

    MathEvalProcessToken( eval, &token );
    if( eval->error ) return 0;

    if( token != ETrbo )
    {
        eval->error = "expected open round bracket after function name";
        return 0;
    }

    eval->roundBracketsCount++;

    switch( func )
    {
        case ETSin:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = sin( result );
            break;

        case ETCos:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = cos( result );
            break;

        case ETTan:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = tan( result );
            break;

        case ETASi:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = asin( result );
            break;

        case ETACo:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = acos( result );
            break;

        case ETATa:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = atan( result );
            break;

        case ETFac:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            if( result < 0 )
            {
                eval->error = "attempt to evaluate factorial of negative number";
            }
            else
            {
                result = tgamma( 1 + result );
            }
            break;

        case ETExp:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = exp( result );
            break;

        case ETPow:
            result = MathEvalProcessAddends( eval, -1, false, true, NULL );
            if( eval->error ) return 0;
            result2 = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = pow( result, result2 );
            break;

        case ETLog:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            if( tokenThatCausedBreak == ETrbc )
            {
                // log(n) with one parameter
                result = log( result );
            }
            else
            {
                result2 = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
                if( eval->error ) return 0;
                result = log( result2 ) / log( result );
            }
            break;

        case ETMax:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

                if( result2 > result )
                {
                    result = result2;
                }
            }
            break;

        case ETMin:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

                if( result2 < result )
                {
                    result = result2;
                }
            }
            break;

        case ETAvg:
            result = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            count = 1;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = MathEvalProcessAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

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
        eval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates an exponentiation.

double MathEvalProcessExponentiation( MathEvaluation *eval,
                                      double          base,     // The base has already been fetched;
                                      MathEvalToken  *rightOp ) // RETURN: the token (operator) that follows.
{
    double exponent,
           result;

    exponent = MathEvalProcessFactors( eval, 1, ETMul, true, rightOp );
    if( eval->error ) return 0;

    result = pow( base, exponent );
    if( eexception( result ) )
    {
        eval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates a factorial using the Gamma function.

double MathEvalProcessFactorial( MathEvaluation *eval,
                                 double          value,     // The value to compute has already been fetched;
                                 MathEvalToken  *rightOp )  // RETURN: the token (operator) that follows.
{
    double result;

    if( value < 0 )
    {
        eval->error = "attempt to evaluate factorial of negative number";
        *rightOp = ETErr;
        return 0;
    }

    result = tgamma( value + 1 );

    if( eexception( result ) )
    {
        eval->error = "result is complex or too big";
        return 0;
    }

    MathEvalProcessToken( eval, rightOp );
    if( eval->error ) return 0;

    return result;
}



// Parses the next token and advances the cursor.
// The function returns a number if the token is a value a const. or a param.
// Whitespace is ignored.

double MathEvalProcessToken( MathEvaluation *eval,
                             MathEvalToken  *token ) // RETURN: the token.
{
    MathEvalToken
           t;
    double v;
    MathEvalParam
          *param;

    t = ETBlk;
    v = 0;

    while( t == ETBlk )
    {
        // value maybe

        if( ( *eval->cursor >= '0' && *eval->cursor <= '9' ) || *eval->cursor == '.' )
        {
            v = MathEvalProcessValue( eval );
            if( eval->error )
            {
                t = ETErr;
                return t;
            }
            else
            {
                t = ETVal;
            }
            break;
        }
        else
        {
            // parameter maybe

            param = eval->params;
            while( param != NULL )
            {
                if( strncmp( param->name, eval->cursor, param->len ) == 0 )
                {
                    *token = ETVal;
                    eval->cursor += param->len;
                    return param->value;
                }

                param = param->next;
            }

            // token maybe

            switch( *eval->cursor )
            {
                case '\n':
                case '\r':
                case '\t':
                case ' ':
                    t = ETBlk;
                    eval->cursor++;
                    break;

                case '+':
                    MathEvalProcessPlusToken( eval, &t);
                    break;

                case '-':
                    t = ETSub;
                    eval->cursor++;
                    break;

                case '*':
                    t = ETMul;
                    eval->cursor++;
                    break;

                case '/':
                    t = ETDiv;
                    eval->cursor++;
                    break;

                case '^':
                    t = ETExc;
                    eval->cursor++;
                    break;

                case '!':
                    t = ETFct;
                    eval->cursor++;
                    break;

                case '(':
                    t = ETrbo;
                    eval->cursor++;
                    break;

                case ')':
                    t = ETrbc;
                    eval->cursor++;
                    break;

                case '\0':
                    t = ETEof;
                    eval->cursor++;
                    break;

                case ',':
                    t = ETcom;
                    eval->cursor++;
                    break;

                case 'e':
                    if( strncmp( eval->cursor, "exp", 3 ) == 0 )
                    {
                        t = ETExp;
                        eval->cursor += 3;
                    }
                    else
                    {
                        v = exp( 1 );
                        t = ETVal;
                        eval->cursor++;
                    }
                    break;

                case 'f':
                    if( strncmp( eval->cursor, "fact", 4 ) == 0 )
                    {
                        t = ETFac;
                        eval->cursor += 4;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'p':
                    if( strncmp( eval->cursor, "pi", 2 ) == 0 )
                    {
                        v = M_PI;
                        t = ETVal;
                        eval->cursor += 2;
                    }
                    else if( strncmp( eval->cursor, "pow", 3 ) == 0 )
                    {
                        t = ETPow;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'c':
                    if( strncmp( eval->cursor, "cos", 3 ) == 0 )
                    {
                        t = ETCos;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 's':
                    if( strncmp( eval->cursor, "sin", 3 ) == 0 )
                    {
                        t = ETSin;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 't':
                    if( strncmp( eval->cursor, "tan", 3 ) == 0 )
                    {
                        t = ETTan;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'l':
                    if( strncmp( eval->cursor, "log", 3 ) == 0 )
                    {
                        t = ETLog;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'm':
                    if( strncmp( eval->cursor, "max", 3 ) == 0 )
                    {
                        t = ETMax;
                        eval->cursor += 3;
                    }
                    else if( strncmp( eval->cursor, "min", 3 ) == 0 )
                    {
                        t = ETMin;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'a':
                    if( strncmp( eval->cursor, "asin", 4 ) == 0 )
                    {
                        t = ETASi;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "acos", 4 ) == 0 )
                    {
                        t = ETACo;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "atan", 4 ) == 0 )
                    {
                        t = ETATa;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "average", 7 ) == 0 )
                    {
                        t = ETAvg;
                        eval->cursor += 7;
                    }
                    else if( strncmp( eval->cursor, "avg", 3 ) == 0 )
                    {
                        t = ETAvg;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;


                default:
                    t = ETErr;
                    break;
            }
        }
    }

    if( t == ETErr )
    {
        eval->error = "unexpected symbol";
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

double MathEvalProcessPlusToken( MathEvaluation *eval, MathEvalToken *token )
{
    do
    {
        eval->cursor++;
    } while( *eval->cursor == ' ' || *eval->cursor == '\n' || *eval->cursor == '\r' || *eval->cursor == '\t' );

    if( *eval->cursor == '+' )
    {
        *token = ETErr;
    }
    else
    {
        *token = ETSum;
    }

    return 0;
}



// Parses a number and advances the cursor.
// The cursor is positioned after an eventually
// `+` or `-` operator that comes before the value.

double MathEvalProcessValue( MathEvaluation *eval )
{
    char   *endptr;
    double value;

    value = strtod( eval->cursor, &endptr );

    if( endptr == eval->cursor )
    {
        eval->error = "expected value";
        value = 0;
    }
    else
    {
        eval->cursor = endptr;

        if( eexception( value ) )
        {
            eval->error = "value is too big";
            return 0;
        }
    }

    return value;
}