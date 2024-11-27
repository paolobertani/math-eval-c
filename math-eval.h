//
//  math-eval
//  version 1.0
//
//  a math expression evaluator
//
//  math-eval.h
//
//  eeval header
//
//  Copyright (c) 2024 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#ifndef math_eval_main_h
#define math_eval_main_h



#include <stdbool.h>
#include <inttypes.h>



// build settings


// FLOATING POINT EXCEPTIONS CATCHING

// leave to true (default) to catch exceptions on math operations (ex. overflows)
#define math_eval_catch_fp_exceptions true


// PRECEDENCE OF UNARY MINUS OPERATOR

// leave to true (default) to give unitary minus highest precedece like in most programming languages
// set to false to give lowest precedence like in math notation (see README.md for details)
#define math_eval_unary_minus_has_highest_precedence true




// tokens

enum MathEvalToken
{
    ETBlk,   // white space, tab, newline...
    ETErr,   // unrecognized token (error)
    ETEof,   // end of string (null termination character)
    ETSum,   // +
    ETSub,   // -
    ETMul,   // *
    ETDiv,   // /
    ETExc,   // ^ exponentiation
    ETFct,   // ! factorial
    ETSin,   // sin(r)
    ETCos,   // cos(r)
    ETTan,   // tan(r)
    ETASi,   // arcsin(n)
    ETACo,   // arccos(n)
    ETATa,   // arctan(n)
    ETFac,   // fact(n) - factorial, equivalent to n!
    ETExp,   // exp(n) - equivalent to e^n
    ETPow,   // pow(b,n) - equivalent to b^n
    ETLog,   // log(b, n) logarithm of n with base b - or log(n) natural logarithm of n
    ETMax,   // max(n1, n2, n3...) maximum of 1 or more numbers
    ETMin,   // min(n1, n2, n3...) minimum of 1 or more numbers
    ETAvg,   // average(n1, n2, n3...) or avg(n1, ...) average of 1 or more numbers
    ETrbo,   // round bracket open  (round bracket count increases)
    ETrbc,   // round bracket close (round bracket count decreases)
    ETcom,   // comma - argument separator inside functions
    ETVal    // a number in scientific notation (1 .1 0.1 1.2E-3) or `e` (euler number) or `pi`
};
typedef enum MathEvalToken MathEvalToken;



enum MathEvaluationStatus
{
    MathEvaluationFailure = 0,
    MathEvaluationSuccess = 1
};
typedef enum MathEvaluationStatus MathEvaluationStatus;



struct MathEvaluation
{
    const char  *expression;
    const char  *cursor;
    double      result;
    int64_t     roundBracketsCount;
    const char  *error;
};
typedef struct MathEvaluation MathEvaluation;



// Public

MathEvaluationStatus MathEvaluationPerform    ( MathEvaluation *eval, const char *expression, double *result );
void                 MathEvaluationPrintError ( MathEvaluation *eval );



// Private

double MathEvalProcessAddends        ( MathEvaluation *eval, int64_t breakOnRoundBracketsCount, bool breakOnETEof, bool breakOnETcom, MathEvalToken *tokenThatCausedBreak );
double MathEvalProcessFactors        ( MathEvaluation *eval, double leftValue, MathEvalToken op, bool isExponent, MathEvalToken *leftOp );
double MathEvalProcessFunction       ( MathEvaluation *eval, MathEvalToken func );
double MathEvalProcessExponentiation ( MathEvaluation *eval, double base, MathEvalToken *rightOp );
double MathEvalProcessFactorial      ( MathEvaluation *eval, double value, MathEvalToken *rightOp );
double MathEvalProcessToken          ( MathEvaluation *eval, MathEvalToken *token );
double MathEvalProcessPlusToken      ( MathEvaluation *eval, MathEvalToken *token );
double MathEvalProcessValue          ( MathEvaluation *eval );



// exception catcher

#if math_eval_catch_fp_exceptions
#define eexception(n) (isnan(n)||(n)==HUGE_VAL||(n)==INFINITY||(n)==-HUGE_VAL||(n)==-INFINITY)
#else
#define eexception(n) (false)
#endif


#endif