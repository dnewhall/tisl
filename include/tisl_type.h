//
// TISL/includ/tisl_type.h
// TISL Ver 4.x
//

#ifndef TISL_TYPE_H
#define TISL_TYPE_H

#include <limits.h>
#include <float.h>

// 32bit? ポインタのサイズにそろえること
typedef int								tINT;
typedef unsigned int					tUINT;
typedef float							tFLOAT;
typedef int								tBOOL;
typedef unsigned char					tCHAR, *tPCHAR;
typedef const unsigned char*			tCSTRING;
typedef unsigned char*					tSTRING;
typedef struct TISL_OBJECT_*			TISL_OBJECT;

// tBOOL
#define tTRUE							1
#define tFALSE							0

#define TISL_MOST_POSITIVE_FLOAT		(FLT_MAX)
#define TISL_MOST_NEGATIVE_FLOAT		(-FLT_MAX)
#define TISL_MOST_POSITIVE_INTEGER		(INT_MAX)
#define TISL_MOST_NEGATIVE_INTEGER		(INT_MIN)

#endif // ifndef TISL_TYPE_H
