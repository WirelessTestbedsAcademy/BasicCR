/**
 * \file tml_arithmetics.h
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
 * The arithmetic operations for the Trinity Math Library.
 */

#ifndef TML_ARITHMETICS_H_
#define TML_ARITHMETICS_H_

#include "tml.h"
#include <math.h>


/** \defgroup scalar Scalar Arithmetics
 *  These functions do simple arithmetics on scalar complex numbers.
 * \{
 */

/** Adds two single precision floating point complex numbers
 * \return The summation result
 */
TMLAPI(SCplx, tml_sc_Add, (const SCplx a, const SCplx b))
/*
{
  SCplx ret = {a.re + b.re, a.im + b.im};
  return ret; 
}
*/


/** Adds two double precision floating point complex numbers
 * \return The summation result
 */
TMLAPI(DCplx, tml_dc_Add, (const DCplx a, const DCplx b))
/*
{
  DCplx ret = {a.re + b.re, a.im + b.im};
  return ret; 
}
*/

/** Returns the complex conjugate of a single precision floating point complex */
TMLAPI(SCplx  , tml_sc_Conj, (const SCplx a))
/*
{
  SCplx ret = {a.re, -a.im};
  return ret;
}
*/


/** Returns the complex conjugate of a double precision floating point complex */
TMLAPI(DCplx  , tml_dc_Conj, (const DCplx a))
/*
{
  DCplx ret = {a.re, -a.im};
  return ret;
}
*/

/** Divides two double precision complex numbers  */
 TMLAPI(DCplx  , tml_dc_Div, (const DCplx a, const DCplx b))
/*
{
  DCplx c;
  double den, r;
  
  if (fabs(b.re) < fabs(b.im))
  {
  r = b.re / b.im;
  den = b.im + r * b.re;
  c.re = (a.re * r + a.im) / den;
  c.im = (a.im * r - a.re) / den;
  }
  else
  {
  r = b.im / b.re;
  den = b.re + r * b.im;
  c.re = (a.re + r * a.im) / den;
  c.im = (a.im - r * a.re) / den;
  }
  
  return c; 
}
*/


/** Divides two single precision complex numbers  */
TMLAPI(SCplx  , tml_sc_Div, (const SCplx a, const SCplx b))
/*
{
  return tml_dc2sc( tml_dc_Div( tml_sc2dc(a), tml_sc2dc(b) ) );
}
*/

/** Converts a double precision complex to a single precision complex
 * \param a double complex float
 * \return the single precision complex float
 * */
TMLAPI(SCplx, tml_dc2sc, (const DCplx a))
/*
{
  SCplx ret;
  ret.re = (float)a.re;
  ret.im = (float)a.im;
 
  return ret;
}
*/


/** Test if two complex numbers are equal
\param a First complex number
\param b Second complex number
\return TRUE if the two numbers are equal
*/
TMLAPI(int, tml_sc_Equal, (const SCplx a, const SCplx b))
/*
{
  return (a.re == b.re) && (a.im == b.im);
}
*/

TMLAPI(int, tml_dc_Equal, (const DCplx a, const DCplx b))
/*
{
  return (a.re == b.re) && (a.im == b.im);
}
*/

/** Computes the absolute value of a single precision complex float */
TMLAPI(float  , tml_sc_Mag, (const SCplx a))
/*
{
  return (float)sqrt(a.re * a.re + a.im * a.im);
}
*/

/** Computes the absolute value of a double precision complex float */
TMLAPI(double  , tml_dc_Mag, (const DCplx a))
/*
{
  return sqrt(a.re * a.re + a.im * a.im);
}
 */


/** Multiplies two single precision floating point complex numbers
  */
TMLAPI(SCplx  , tml_sc_Mpy, (const SCplx a, const SCplx b))
/*
{
  SCplx ret = {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
  return ret; 
}
*/

/** Multiplies two double precision floating point complex numbers
  */
TMLAPI(DCplx  , tml_dc_Mpy, (const DCplx a, const DCplx b))
/*
{
  DCplx ret = {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
  return ret; 
}
*/


/** Computes the angle value of a single precision complex float.
 *  \return The result, between -pi and pi. Might be 0.0 if an error occured,
 *  see atan2() in math.h for a description.
*/
 TMLAPI(float  , tml_sc_Phase, (const SCplx a))
/*
{
  return (float)atan2(a.im, a.re);
}
*/

/** Computes the angle value of a double precision complex float.
 *  \return The result, between -pi and pi. Might be 0.0 if an error occured,
 *  see atan2() in math.h for a description.
*/
TMLAPI(double  , tml_dc_Phase, (const DCplx a))
/*
{
  return atan2(a.im, a.re);
}
*/

/** Creates a new single precision complex float from abs and angle */
TMLAPI(SCplx  , tml_sc_PolarToCart, (const float _abs, const float _angle))
/*
{
  SCplx ret;
  
  ret.re = (float)(_abs * cos(_angle));
  ret.im = (float)(_abs * sin(_angle));
  
  return ret;
} 
*/


/** Creates a new double precision complex float from abs and angle */
TMLAPI(DCplx , tml_dc_PolarToCart, (const double _abs, const double _angle))
/*
{
  DCplx ret;
  
  ret.re = (_abs * cos(_angle));
  ret.im = (_abs * sin(_angle));
  
  return ret;
} 
*/

/** Converts a single precision complex to a double precision complex */
TMLAPI(DCplx, tml_sc2dc, (const SCplx a))
/*
{
  DCplx ret;
  ret.re = (double)a.re;
  ret.im = (double)a.im;
 
  return ret;
}
*/

/** Creates a new single precision complex float from real and imag part */
TMLAPI(SCplx  , tml_sc_Set, (const float re, const float im))
/*
{
  SCplx ret = {re, im};
  
  return ret;
}
*/

/** Creates a new double precision complex float from real and imag part */
TMLAPI(DCplx  , tml_dc_Set, (const double re, const double im))
/*
{
  DCplx ret = {re, im};

  return ret;
}
*/

/** Subtracts two single precision floating point complex numbers
*   \return The result of the subtraction
*/
TMLAPI(SCplx  , tml_sc_Sub, (const SCplx a, const SCplx b))
/*
{
  SCplx ret = {a.re - b.re, a.im - b.im};
  return ret; 
}
*/

/** Subtracts two double precision floating point complex numbers
*/
TMLAPI(DCplx , tml_dc_Sub, (const DCplx a, const DCplx b))
/*
{
  DCplx ret = {a.re - b.re, a.im - b.im};
  return ret; 
}
*/


/** \} */


#endif /*TML_ARITHMETICS_H_*/
