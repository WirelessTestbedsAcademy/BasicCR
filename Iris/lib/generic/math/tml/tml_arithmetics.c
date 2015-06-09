/**
 * \file tml_arithmetics.c
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
 * Implementation of the arithmetic operations for the Trinity Math Library.
 */

#define TML_W32DLL
#define INLINE
#include "tml.h"


TMLIMP(int, tml_sc_Equal, (const SCplx a, const SCplx b))
{
  return (a.re == b.re) && (a.im == b.im);
}

TMLIMP(int, tml_dc_Equal, (const DCplx a, const DCplx b))
{
  return (a.re == b.re) && (a.im == b.im);
}


TMLIMP(SCplx, tml_dc2sc, (const DCplx a))
{
  SCplx ret;
  ret.re = (float)a.re;
  ret.im = (float)a.im;
 
  return ret;
}

TMLIMP(DCplx, tml_sc2dc, (const SCplx a))
{
  DCplx ret;
  ret.re = (double)a.re;
  ret.im = (double)a.im;
 
  return ret;
}

TMLIMP(SCplx, tml_sc_Add, (const SCplx a, const SCplx b))
{
  SCplx ret = {a.re + b.re, a.im + b.im};
  return ret; 
}

TMLIMP(DCplx, tml_dc_Add, (const DCplx a, const DCplx b))
{
  DCplx ret = {a.re + b.re, a.im + b.im};
  return ret; 
}


TMLIMP(SCplx  , tml_sc_Sub, (const SCplx a, const SCplx b))
{
  SCplx ret = {a.re - b.re, a.im - b.im};
  return ret; 
}

TMLIMP(DCplx , tml_dc_Sub, (const DCplx a, const DCplx b))
{
  DCplx ret = {a.re - b.re, a.im - b.im};
  return ret; 
}


TMLIMP(SCplx  , tml_sc_Mpy, (const SCplx a, const SCplx b))
{
  SCplx ret = {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
  return ret; 
}

TMLIMP(DCplx  , tml_dc_Mpy, (const DCplx a, const DCplx b))
{
  DCplx ret = {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
  return ret; 
}


 TMLIMP(DCplx  , tml_dc_Div, (const DCplx a, const DCplx b))
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

TMLIMP(SCplx  , tml_sc_Div, (const SCplx a, const SCplx b))
{
  return tml_dc2sc( tml_dc_Div( tml_sc2dc(a), tml_sc2dc(b) ) );
}

TMLIMP(SCplx  , tml_sc_Conj, (const SCplx a))
{
  SCplx ret = {a.re, -a.im};
  return ret;
}

TMLIMP(DCplx  , tml_dc_Conj, (const DCplx a))
{
  DCplx ret = {a.re, -a.im};
  return ret;
}


 TMLIMP(float  , tml_sc_Mag, (const SCplx a))
{
  return (float)sqrt(a.re * a.re + a.im * a.im);
}

TMLIMP(double  , tml_dc_Mag, (const DCplx a))
{
  return sqrt(a.re * a.re + a.im * a.im);
}
 

TMLIMP(float  , tml_sc_Phase, (const SCplx a))
{
  return (float)atan2(a.im, a.re);
}

TMLIMP(double  , tml_dc_Phase, (const DCplx a))
{
  return atan2(a.im, a.re);
}

TMLIMP(SCplx  , tml_sc_Set, (const float re, const float im))
{
  SCplx ret = {re, im};
  
  return ret;
}

TMLIMP(DCplx  , tml_dc_Set, (const double re, const double im))
{
  DCplx ret = {re, im};

  return ret;
}

TMLIMP(SCplx  , tml_sc_PolarToCart, (const float _abs, const float _angle))
{
  SCplx ret;
  
  ret.re = (float)(_abs * cos(_angle));
  ret.im = (float)(_abs * sin(_angle));
  
  return ret;
} 

TMLIMP(DCplx , tml_dc_PolarToCart, (const double _abs, const double _angle))
{
  DCplx ret;
  
  ret.re = (_abs * cos(_angle));
  ret.im = (_abs * sin(_angle));
  
  return ret;
} 
