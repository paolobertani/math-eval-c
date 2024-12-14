//
//  math-eval-private
//
//  private header
//
//  Copyright (c) 2024 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#ifndef math_eval_h
#define math_eval_h



//
// Build settings
//


// FLOATING POINT EXCEPTIONS CATCHING

// leave to true (default) to catch exceptions on math operations (ex. overflows)
#define math_eval_catch_fp_exceptions true


// PRECEDENCE OF UNARY MINUS OPERATOR

// leave to true (default) to give unitary minus highest precedece like in most programming languages
// set to false to give lowest precedence like in math notation (see README.md for details)
#define math_eval_unary_minus_has_highest_precedence false



#include "math-eval-private.h"



//
// Enum
//

enum MathEvaluationStatus
{
    MathEvaluationFailure = 0,
    MathEvaluationSuccess = 1
};
typedef enum MathEvaluationStatus MathEvaluationStatus;



//
// Public functions
//

MathEvaluation *     MathEvaluationNew        ( const char *expression );
void                 MathEvaluationDispose    ( MathEvaluation *eval );
MathEvaluationStatus MathEvaluationSetParam   ( MathEvaluation *eval, const char *name, double value );
MathEvaluationStatus MathEvaluationPerform    ( MathEvaluation *eval, double *result );
double               MathEvaluationGetResult  ( MathEvaluation *eval );
const char *         MathEvaluationGetError   ( MathEvaluation *eval, int *position );
void                 MathEvaluationPrintError ( MathEvaluation *eval );

#endif
