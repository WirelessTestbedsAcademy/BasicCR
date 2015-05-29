/**
 * \file tml.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Common header for the Trinity Math Library.
 */

#ifndef TML_H_
#define TML_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* ---------------------------- Definitions ------------------------------- */

#define  TML_EPS   (1.0E-12)

#define  TML_PI  (3.141592653589793115997963468544185161590576171875) /* double precision */   
#define  TML_2PI   (6.28318530717958623199592693708837032318115234375)
#define  TML_PI_2  (1.5707963267948965579989817342720925807952880859375)
#define  TML_PI_4  (0.78539816339744827899949086713604629039764404296875)

#define TML_Gt 1
#define TML_GT TML_Gt
#define TML_Lt 0
#define TML_LT TML_Lt

/** converts degrees to radians */
#define  tml_deg2rad(deg)  ((deg)/180.0 * TML_PI)

/** converts degrees to single precision float radian */
#define  tml_s_deg2rad(deg)  ((float) (tml_deg2rad(deg)))

/** converts degrees to double precision float radian */
#define  tml_d_deg2rad(deg)  ((double)(tml_deg2rad(deg)))

/** converts radian to degrees */
#define  tml_rad2deg(rad)  ((rad)*180.0 / TML_PI)

/** converts radians to single precision float degrees */
#define  tml_s_rad2deg(rad) ((float) (tml_rad2deg(rad)))

/** converts radians to double precision float degrees */
#define  tml_d_rad2deg(rad) ((double) (tml_rad2deg(rad)))

/** returns the max of a and b */
#define  MAX(a,b)  (((a) > (b)) ? (a) : (b))

/** returns the min of a and b */
#define  MIN(a,b)  (((a) < (b)) ? (a) : (b))

#if !defined (false)
#define false 0
#define true 1
#endif

#if !defined (FALSE)
#define  FALSE  0
#define  TRUE   1
#endif

#define TML_Round 0x01
#define TML_TruncZero 0x02
#define TML_TruncNeg 0x04
#define TML_Unsigned 0x08


/* --------------------------- Common Types ------------------------------- */

/* do all this ifdef stuff to avoid multiple definitions of these types */

#if !defined(def_SCplx)
#define def_SCplx
/** single precision floating point complex */
  typedef struct _SCplx {
  float    re;
  float    im;
  } SCplx;
#endif

#if !defined(def_DCplx)
#define def_DCplx
/** double precision floating point complex */
  typedef struct _DCplx {
  double    re;
  double    im;
  } DCplx;
#endif

#if !defined(def_WCplx)
#define def_WCplx
/** short integer complex */
  typedef struct _WCplx {
  short     re;
  short     im;
  } WCplx;
#endif

#if !defined(def_ICplx)
#define def_ICplx
/** integer complex */
  typedef struct _ICplx {
  int     re;
  int     im;
  } ICplx;
#endif

static const SCplx  SCplxZero = {0,0};
static const DCplx  DCplxZero = {(double)0.0,(double)0.0};
static const WCplx  WCplxZero = {(short) 0,  (short) 0};
static const ICplx  ICplxZero = {(int)0,(int)0};

static const SCplx SCplxOneZero = {1,0};
static const DCplx DCplxOneZero = {(double)1.0,(double)0.0};
static const WCplx WCplxOneOne  = {(short) 1,  (short) 1};
static const ICplx ICplxOneOne  = {(int)1,(int)1};
static const SCplx SCplxOneOne  = {(float) 1.0,(float) 1.0};
static const DCplx DCplxOneOne  = {(double)1.0,(double)1.0};


/* ======================================================================== */


#ifdef __cplusplus
extern "C" {
#endif

#if defined( _WIN32 ) && !defined( __GNUC__ )
#define __STDCALL __stdcall
#define __CDECL __cdecl
#define __INT64 __int64
#else
#define __STDCALL
#define __CDECL
#define __INT64 long long 
#endif

/*
#ifndef INLINE
# if __GNUC__
#  define INLINE extern inline
# else
#  define INLINE inline
# endif
#endif
*/

#if !defined(TMLAPI)
#  if defined linux || defined __APPLE__
#  define TMLAPI(type, name, arg) extern type name arg;
#  define TMLIMP(type, name, arg) type name arg
#  define TMLINL(type, name, arg) INLINE type name arg
#  endif
#endif

#if !defined (TMLAPI)
# if !defined(TML_W32DLL)
#  define TMLAPI(type,name,arg) extern  __declspec(dllimport) type __STDCALL name arg;
#  define TMLIMP(type,name,arg) __declspec(dllimport)  type __STDCALL name arg
#  define TMLINL(type,name,arg) TMLIMP(type, name, arg)
# elif defined (__BORLANDC__)
#  define TMLAPI(type,name,arg) extern  type _import __STDCALL name arg;
#  define TMLIMP(type,name,arg)   type _import __STDCALL name arg
#  define TMLINL(type,name,arg) INLINE type name arg
# else
#   if defined (_WIN32)
#    define TMLAPI(type,name,arg) extern  __declspec(dllexport) type __STDCALL name arg;
#    define TMLIMP(type,name,arg)  __declspec(dllexport) type __STDCALL name arg
#    define TMLINL(type,name,arg) INLINE type name arg
#   endif
# endif
#endif

#undef TMLINL
#define TMLINL(type,name,arg) TMLAPI(type,name,arg)

/* --------------------------- Functions here: ------------------------------- */
/* They must be defined like:
 * TMLAPI(<return type>, <function name>, (<parameter list>))
 * 
 * e.g. like:
 * TMLAPI(SCplx, tml_sc_add, (const SCplx a, const SCplx b)) 
 * 
 */


/*-------------------------- Scalar Manipulation ----------------------------- */

#include "tml_arithmetics.h"

/*------------------------------ Vector Manipulation ------------------------ */
/** \defgroup vector Vector Manipulation
 *  Functions for vector arithmetics, either real or complex valued. 
 * \{
 */

/**Computes the absolute values of vector
elements in-place.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
*/
TMLAPI(void, tml_sr_vAbs1,(float *vec, int n))
TMLAPI(void, tml_dr_vAbs1,(double *vec, int n))

/**Computes the absolute values of vector
elements and stores the results in a
second vector.
\param src Pointer to the vector src[n].
\param dst Pointer to the vector dst[n].
\param n The number of elements in the vectors.
*/
TMLAPI(void, tml_sr_vAbs2,(const float *src, float *dst, int n))
TMLAPI(void, tml_dr_vAbs2,(const double *src, double *dst, int n))

/**Adds a value to each element of a vector.
\param val The value used to increment each element of the vector
dst[n].
\param dst Pointer to the vector dst[n].
\param n The number of values in the vector dst.
*/
TMLAPI(void, tml_sr_vAdd1, (const float val, float *dst, int n))
TMLAPI(void, tml_dr_vAdd1, (const double val, double *dst, int n))
TMLAPI(void, tml_sc_vAdd1, (const SCplx val, SCplx *dst, int n))

/**Adds the elements of two vectors.
\param dst Pointer to the vector dst[n]. The vector dst[n] stores
the result of the addition src[n] + dst[n].
\param src Pointer to the vector to be added to dst[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sr_vAdd2, (const float *src, float *dst, int n))
TMLAPI(void, tml_dr_vAdd2, (const double *src, double *dst, int n))
TMLAPI(void, tml_sc_vAdd2, (const SCplx *src, SCplx *dst, int n))

/**Adds the elements of two vectors and
stores the result in a third vector.
\param dst Pointer to the vector dst[n]. This vector stores the
result of the addition srcA[n] + srcB[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sr_vAdd3, (const float *srcA, const float *srcB, float *dst, int n))
TMLAPI(void, tml_dr_vAdd3, (const double *srcA, const double *srcB, double *dst, int n))
TMLAPI(void, tml_sc_vAdd3, (const SCplx *srcA, const SCplx *srcB, SCplx *dst, int n))


/**Converts the elements of a complex
vector to polar coordinate form.
\param src Pointer to the vector src[len].
\param mag Pointer to the vector mag[len] which stores the
magnitude (radius) components of the elements of
vector src[len].
\param phase Pointer to the vector phase[len] which stores the
phase (angle) components of the elements of vector
srclen]. Phase values are in the range (-p,p].
\param len The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vCartToPolar,(const SCplx *src, float *mag, float *phase,
int len))


/**Computes the complex conjugate of a vector.
\param vec Pointer to the vector whose complex conjugate is to be
computed.
\param n The number of values in the vector vec[n].
*/
TMLAPI(void, tml_sc_vConj1, (SCplx *vec, int n))

/**Computes the complex conjugate of a
vector and stores the result in a second vector.
\param src Pointer to the vector whose complex conjugate is to be
computed.
\param dst Pointer to the vector which stores the complex conjugate
of the vector src[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vConj2, (const SCplx *src, SCplx *dst, int n))

/**Initializes a vector with the contents of a
second vector.
\param dst Pointer to the vector to be initialized.
\param n The number of elements to copy.
\param src Pointer to the source vector used to initialize dst[n].
*/
TMLAPI(void, tml_sr_vCopy, (const float *src, float *dst, int n))
TMLAPI(void, tml_dr_vCopy, (const double *src, double *dst, int n))
TMLAPI(void, tml_sc_vCopy, (const SCplx *src, SCplx *dst, int n))



/**Returns the real and imaginary parts of
a complex vector in two respective
vectors.
\param src Pointer to the vector src[n].
\param dstReal Pointer to the vector dstReal[n].
\param dstImag Pointer to the vector dstImag[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vCplxTo2Real, (const SCplx *src, float *dstReal,
float *dstImag, int n))


/** Computes the dot product of two vectors.
 * \param a Pointer to first vector
 * \param b Pointer to second vector
 * \param len Length of both vectors
 * \return The result, sum_n(a[n] * b[n])
 * \todo Write assembly routines for AMD and INTEL processors
 */
TMLAPI(SCplx, tml_sc_vDotProd, (const SCplx *a, const SCplx *b, int len))
TMLAPI(float, tml_sr_vDotProd, (const float *a, const float *b, int len))
TMLAPI(double, tml_dr_vDotProd, (const double *a, const double *b, int len))
TMLAPI(SCplx, tml_scr_vDotProd, (const SCplx *a, const float *b, int len))


/** Test if two vectors of complex values are equal
\param srcA First vector
\param srcB Second vector
\param n The number of elements in the vectors
\return TRUE if the vectors are equal
*/
TMLAPI(int, tml_sc_vEqual, (const SCplx *srcA, const SCplx *srcB, int n))
TMLAPI(int, tml_sr_vEqual, (const float *srcA, const float *srcB, int n)) 
TMLAPI(int, tml_dr_vEqual, (const double *srcA, const double *srcB, int n))


/**Converts the floating-point data of a
vector to integer data and stores the
results in a second vector.
\param src Pointer to the vector src[len].
\param len The number of values in the src[len] vector.
\param dst Pointer to the vector which stores the results of the
conversion to integer data. The type of the vector
dst[len] is void to support different integer word
sizes.
\param wordSize The size of an integer word in bits; must be 8, 16, or 32.
\param flags Flags how to convert the doubles. The following are possible:
  - \c TML_Round Round to the nearest integer number
  - \c TML_TruncZero  Truncate towards zero
  - \c TML_TruncNeg   Truncate towards negative numbers
  - \c TML_Unsigned   Indicates that the integers used are unsigned - can be used
           together with one of the other flags by logical or
*/
TMLAPI(void, tml_sr_vFloatToInt,(const float *src, void *dst, int len,int wordSize, int flags))
TMLAPI(void, tml_dr_vFloatToInt,(const double *src, void *dst, int len,int wordSize, int flags))


/**Converts the integer data of a vector to
floating-point data and stores the results
in a second vector.
\param src Pointer to the vector src[len]. The type of the vector
src[len] is void to support different integer word
sizes.
\param len The number of values in the src[len] vector.
\paramdst Pointer to the vector which stores the results of the
conversion to the floating-point data.
\param wordSize The size of an integer in bits; must be 8, 16, or 32.
//doesn't use flags  
*/
TMLAPI(void, tml_sr_vIntToFloat,(const void *src, float *dst, int len, int wordSize))
TMLAPI(void, tml_dr_vIntToFloat,(const void *src, double *dst, int len, int wordSize))

/**Returns the magnitudes of elements of a
complex vector in a second vector.
\param src Pointer to the vector src[n].
\param mag Pointer to the vector mag[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vMag, (const SCplx *src, float *mag, int n))

/**Returns the maximum value of a vector.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
*/
TMLAPI(float, tml_sr_vMax, (const float *vec, int n))
TMLAPI(double, tml_dr_vMax, (const double *vec, int n))

/**Returns the maximum value of a vector
and the index of the maximum element.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
\param index On exit, contains the index of the maximum element.
*/
TMLAPI(float, tml_sr_vMaxExt, (const float *vec, int n, int *index))
TMLAPI(double, tml_dr_vMaxExt, (const double *vec, int n, int *index))

/**Computes the mean value of a vector.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
*/
TMLAPI(float, tml_sr_vMean,(const float *vec,int n))
TMLAPI(double, tml_dr_vMean,(const double *vec,int n))

/*Returns the minimum value of a vector.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
*/
TMLAPI(float, tml_sr_vMin, (const float *vec, int n))
TMLAPI(double, tml_dr_vMin, (const double *vec, int n))

/**Returns the minimum value of a vector and
the index of the minimum element.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
\param index On exit, contains the index of the minimum element.
*/
TMLAPI(float, tml_sr_vMinExt, (const float *vec, int n, int *index))
TMLAPI(double, tml_dr_vMinExt, (const double *vec, int n, int *index))

/**
Multiplies each element of a vector by a value.
*/
TMLAPI(void, tml_sr_vMpy1, (const float val, float *dst, int n))
TMLAPI(void, tml_dr_vMpy1, (const double val, double *dst, int n))
TMLAPI(void, tml_sc_vMpy1, (const SCplx val, SCplx *dst, int n))

/**
Multiplies each element of two vectors.
\param dst Pointer to the vector dst[n]. This vector stores the
result of the multiplication (src[n] * dst[n]).
\param src Pointer to the vector to be multiplied with dst[n].
*/
TMLAPI(void, tml_sr_vMpy2, (const float *src, float *dst, int n))
TMLAPI(void, tml_dr_vMpy2, (const double *src, double *dst, int n))
TMLAPI(void, tml_sc_vMpy2, (const SCplx *src, SCplx *dst, int n))

/**Multiplies two vectors and stores the
result in a third vector.
\param dst Pointer to the vector dst[n]. This vector stores the
result of the multiplication (srcA[n] * srcB[n]).
\param srcA, srcB Pointers to the vectors whose elements are to be
multiplied together.
*/
TMLAPI(void, tml_sr_vMpy3, (const float *srcA, const float *srcB, float *dst, int n))
TMLAPI(void, tml_dr_vMpy3, (const double *srcA, const double *srcB, double *dst, int n))
TMLAPI(void, tml_sc_vMpy3, (const SCplx *srcA, const SCplx *srcB, SCplx *dst, int n))


/**Subtracts a constant from vector
elements and divides the result by
another constant.
\param src Pointer to the input vector a[n].
\param dst Pointer to the output vector b[n].
\param n The number of elements in each of these vectors.
\param offset The constant subtracted from input vector elements.
\param factor The constant by which the vector elements are divided.
*/
TMLAPI(void, tml_sr_vNormalize, (const float *src, float *dst, int n, float
offset, float factor))

/**Subtracts a constant from vector
elements and divides the result by
another constant.
\param src Pointer to the input vector a[n].
\param dst Pointer to the output vector b[n].
\param n The number of elements in each of these vectors.
\param offset The constant subtracted from input vector elements.
\param factor The constant by which the vector elements are divided.
*/
TMLAPI(void, tml_dr_vNormalize, (const double *src, double *dst, int n, double
offset, double factor))

/**Subtracts a constant from vector
elements and divides the result by
another constant.
\param src Pointer to the input vector a[n].
\param dst Pointer to the output vector b[n].
\param n The number of elements in each of these vectors.
\param offset The constant subtracted from input vector elements.
\param factor The constant by which the vector elements are divided.
*/
TMLAPI(void, tml_sc_vNormalize, (const SCplx *src, SCplx *dst, int n, SCplx
offset, float factor))

/**Returns the phase angles of elements of
complex input vector in a second vector.
\param src Pointer to the vector src[n].
\param phase Pointer to the vector phase[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vPhase, (const SCplx *src, float *phase, int n))

/**Computes the phase angles of elements
of a complex vector whose real and
imaginary components are specified in
two vectors and stores the results in a
third vector.
\param srcReal Pointer to the vector srcReal[n].
\param srcImag Pointer to the vector srcImag[n].
\param phase Pointer to the vector phase[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sr_vrPhase, (const float *srcReal, const float *srcImag,
float *phase, int n))

/**Converts the polar form
magnitude/phase pairs stored in input
vectors to Cartesian coordinate form.
\param mag Pointer to the vector mag[len] which stores the
magnitude (radius) components of the elements.
\param phase Pointer to the vector phase[len] which stores the
phase (angle) components of the elements.
\param dst Pointer to the resulting vector dst[len] which stores
the complex values consisting of magnitude (radius) and
phase (angle).
\param len The number of values in the vectors.
*/
TMLAPI(void, tml_sr_vPolarToCart,(const float *mag, const float *phase,
SCplx *dst, int len))


/**Returns the real part of a complex
vector in a second vector.
\param src Pointer to the vector src[n].
\param dst Pointer to the vector dst[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sc_vReal, (const SCplx *src, float *dst, int n))

/**Returns a complex vector constructed
from the real and imaginary parts of two
real vectors.
\param srcReal Pointer to the vector srcReal[n].
\param srcImag Pointer to the vector srcImag[n].
\param dst Pointer to the vector dst[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sr_v2RealToCplx, (const float *srcReal, const float *srcImag, SCplx *dst, int n))

 /**
Initializes a vector to a specified value.
\param dst Pointer to the vector to be initialized.
\param n The number of elements to initialize.
\param re, im The complex value (re + jim) used to initialize the
vector dst[n].
\param val The real value used to initialize the vector dst[n].
*/
TMLAPI(void, tml_sr_vSet,(float val, float *dst, int n))
TMLAPI(void, tml_dr_vSet,(double val, double *dst, int n))
TMLAPI(void, tml_sc_vSet,(float re, float im, SCplx *dst, int n))


/**Computes a square root of each element
of a vector in-place.
\param vec Pointer to the vector vec[n].
\param n The number of elements in the vector.
*/
TMLAPI(void, tml_sr_vSqrt1, (float *vec, int n))
TMLAPI(void, tml_dr_vSqrt1, (double *vec, int n))
TMLAPI(void, tml_sc_vSqrt1, (SCplx *vec, int n))


/** Subtracts a value from a vector.
 * \param dst Pointer to the dst[n]. The vector dst[n] stores
 * the result of the subtraction dst[n]- val.
 * \param val Value to be subtracted from the vector.
 * \param n Length of dst[n].
 */
 TMLAPI(void, tml_sr_vSub1, (const float val, float *dst, int n))
 TMLAPI(void, tml_dr_vSub1, (const double val, double *dst, int n))
 TMLAPI(void, tml_sc_vSub1, (const SCplx val, SCplx *dst, int n))

/**Subtracts the elements of two vectors.
\param dst Pointer to the vector dst[n]. The vector dst[n] stores
the result of the subtraction dst[n]- src[n].
\param src Pointer to the vector to be substracted from dst[n].
\param n The number of values in the vectors.
*/
TMLAPI(void, tml_sr_vSub2, (const float *src, float *dst, int n))
TMLAPI(void, tml_dr_vSub2, (const double *src, double *dst, int n))
TMLAPI(void, tml_sc_vSub2, (const SCplx *src, SCplx *dst, int n))


/** Subtracts the elements of two vectors and stores the result in a third vector.
 * \param src Pointer to the first vector
 * \param val Pointer to the second vector
 * \param dst Point to the result vector (dst[i] = src[i] - val[i])
 * \param n length of the vectors
 */ 
TMLAPI(void, tml_sr_vSub3, (const float *src, const float *val, float *dst, int n))
TMLAPI(void, tml_dr_vSub3, (const double *src, const double *val, double *dst, int n))
TMLAPI(void, tml_sc_vSub3, (const SCplx *src, const SCplx *val, SCplx *dst, int n))

/**Computes the sum of vector elements.
\param src Pointer to the vector src[n].
\param n The number of elements in the vector
*/
TMLAPI(float, tml_sr_vSum,(const float *src,int n))
TMLAPI(double, tml_dr_vSum,(const double *src,int n))
/** Computes the sum of vector elements.
\param src Pointer to the vector src[n].
\param n The number of elements in the vector
\param sum The sum of vector elements.
*/
TMLAPI(void, tml_sc_vSum,(const SCplx *src, int n, SCplx *sum))

/**Performs the threshold operation on the
elements of a vector in-place by limiting
the element values by thresh.
\param vec Pointer to the vector on whose elements the threshold
operation is performed.
\param n The number of elements in the vector.
\param thresh A value used to limit each element of vec[n]. This
argument must always be real. For complex flavors, it
must be positive and represent magnitude.
\param relOP The values of this argument specify which relational
operator to use and whether thresh is an upper (relOP = TML_LT) or
lower bound (relOP = TML_GT) for the input.
*/
TMLAPI(void, tml_sr_vThresh1, (float *vec, int n, float thresh,int relOP))
TMLAPI(void, tml_dr_vThresh1, (double *vec, int n, double thresh,int relOP))
TMLAPI(void, tml_sc_vThresh1, (SCplx *vec, int n, float thresh, int relOP))

/**Initializes a vector to zero.
\param dst Pointer to the vector to be initialized to zero.
\param n The number of elements to initialize.
*/
TMLAPI(void, tml_sr_vZero, (float *dst, int n))
TMLAPI(void, tml_dr_vZero, (double *dst, int n))
TMLAPI(void, tml_sc_vZero, (SCplx *dst, int n))

/** \} */

/*------------------------------ Memory Allocation ------------------------ */
/** \defgroup Memory Allocation
 * \{
 */

/**Frees a memory block previously
allocated by one of the tml?Malloc
functions
\pram ptr The pointer to a memory block to be freed.
*/
TMLAPI(void, tml_Free, (void* ptr))

/**Allocates a 32-byte aligned memory
block for data of different types.
\param length Number of data items to allocate.
*/
TMLAPI(float*, tml_sr_Malloc, (int length))
TMLAPI(double*, tml_dr_Malloc, (int length))
TMLAPI(SCplx*, tml_sc_Malloc, (int length))

/** \} */


/* ----------------- Signal Processing Header ------ */

#include "tml_processing.h"


/* -------------- Stochastics Header -------------- */

#include "tml_stochastics.h"

#ifdef __cplusplus
}
#endif


#endif /*TML_H_*/
