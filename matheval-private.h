//
//  math-eval-private
//
//  private header
//
//  Copyright (c) 2024 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>



#ifndef math_eval_private_h
#define math_eval_private_h



// tokens

enum MathEvalToken
{
    MET_Blk,   // white space, tab, newline...
    MET_Err,   // unrecognized token (error)
    MET_Eof,   // end of string (null termination character)
    MET_Sum,   // +
    MET_Sub,   // -
    MET_Mul,   // *
    MET_Div,   // /
    MET_Exc,   // ^ exponentiation
    MET_Fct,   // ! factorial
    MET_Sin,   // sin(r)
    MET_Cos,   // cos(r)
    MET_Tan,   // tan(r)
    MET_ASi,   // arcsin(n)
    MET_ACo,   // arccos(n)
    MET_ATa,   // arctan(n)
    MET_Fac,   // fact(n) - factorial, equivalent to n!
    MET_Exp,   // exp(n) - equivalent to e^n
    MET_Pow,   // pow(b,n) - equivalent to b^n
    MET_Log,   // log(b, n) logarithm of n with base b - or log(n) natural logarithm of n
    MET_Max,   // max(n1, n2, n3...) maximum of 1 or more numbers
    MET_Min,   // min(n1, n2, n3...) minimum of 1 or more numbers
    MET_Avg,   // average(n1, n2, n3...) or avg(n1, ...) average of 1 or more numbers
    MET_rbo,   // round bracket open  (round bracket count increases)
    MET_rbc,   // round bracket close (round bracket count decreases)
    MET_com,   // comma - argument separator inside functions
    MET_Val    // a number in scientific notation (1 .1 0.1 1.2E-3) or `e` (euler number) or `pi`
};
typedef enum MathEvalToken MathEvalToken;



// Private structures

struct MathEvalParam
{
    char                    name[256];
    size_t                  len;
    double                  value;
    struct MathEvalParam    *next;
};
typedef struct MathEvalParam MathEvalParam;



struct MathEvaluation
{
    const char      *expression;
    MathEvalParam   *params;
    const char      *cursor;
    double          result;
    int64_t         roundBracketsCount;
    const char      *error;
};
typedef struct MathEvaluation MathEvaluation;



// Private functions

double MathEvalProcessAddends        ( MathEvaluation *eval, int64_t breakOnRoundBracketsCount, bool breakOnETEof,
                                       bool breakOnETcom, MathEvalToken *tokenThatCausedBreak );
double MathEvalProcessFactors        ( MathEvaluation *eval, double leftValue, MathEvalToken op, bool isExponent,
                                       MathEvalToken *leftOp );
double MathEvalProcessFunction       ( MathEvaluation *eval, MathEvalToken func );
double MathEvalProcessExponentiation ( MathEvaluation *eval, double base, MathEvalToken *rightOp );
double MathEvalProcessFactorial      ( MathEvaluation *eval, double value, MathEvalToken *rightOp );
double MathEvalProcessToken          ( MathEvaluation *eval, MathEvalToken *token );
double MathEvalProcessPlusToken      ( MathEvaluation *eval, MathEvalToken *token );
double MathEvalProcessValue          ( MathEvaluation *eval );
void   MathEvalDumpParams            ( MathEvaluation *eval );



// Exception catcher

#if math_eval_catch_fp_exceptions
#define eexception(n) (isnan(n)||(n)==HUGE_VAL||(n)==INFINITY||(n)==-HUGE_VAL||(n)==-INFINITY)
#else
#define eexception(n) (false)
#endif


#endif
