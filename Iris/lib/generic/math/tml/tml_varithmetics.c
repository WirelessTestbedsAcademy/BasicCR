/**
 * \file tml_varithmetics.h
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
 * Vector arithmetic operations.
 */

#define TML_W32DLL
#include "tml.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <math.h>

#ifdef SSE3
#include <pmmintrin.h> //(for SSE3 intrinsics instructions) 
//should be in "\Program Files\Microsoft Visual Studio_version\VC\include"
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif 


TMLIMP(int, tml_sc_vEqual, (const SCplx *srcA, const SCplx *srcB, int n))
{
  int i = 0;
  int ret = TRUE;

  for (i = 0; i < n && ret; i++)
    ret = ((srcA[i].re == srcB[i].re) && (srcA[i].im == srcB[i].im));
  
  return ret;
}

TMLIMP(int, tml_sr_vEqual, (const float *srcA, const float *srcB, int n))
{
  int i = 0;
  int ret = TRUE;
  
  for (i = 0; i < n && ret; i++)
    ret = (srcA[i] == srcB[i]);
	
  return ret;
}

TMLIMP(int, tml_dr_vEqual, (const double *srcA, const double *srcB, int n))
{
  int i = 0;
  int ret = TRUE;
  
  for (i = 0; i < n && ret; i++)
    ret = (srcA[i] == srcB[i]);
  
  return ret;
}

TMLIMP(void, tml_sr_vSet,(float val, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] = val;
}

TMLIMP(void, tml_dr_vSet,(double val, double *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] = val;
}


TMLIMP(void, tml_sc_vSet,(float re, float im, SCplx *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
	{	
		dst[i].re = re;
		dst[i].im = im;
	}
}

TMLIMP(void, tml_sr_vAdd1, (const float val, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] += val;
}

TMLIMP(void, tml_dr_vAdd1, (const double val, double *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] += val;
}

TMLIMP(void, tml_sc_vAdd1, (const SCplx val, SCplx *dst, int n))
{	
	int i;
	for(i = 0; i < n; i++)
	{	
		dst[i].re += val.re;
		dst[i].im += val.im;
	}
}

TMLIMP(void, tml_sr_vAdd2, (const float *src, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] += src[i];	
}

TMLIMP(void, tml_dr_vAdd2, (const double *src, double *dst, int n))
{
  tml_dr_vAdd3(src, dst, dst, n);
}


TMLIMP(void, tml_sc_vAdd2, (const SCplx *src, SCplx *dst, int n))
{	
	int i;
	for(i = 0; i < n; i++)
	{	
		dst[i].re += src[i].re;
		dst[i].im += src[i].im;
	}
}

TMLIMP(void, tml_sr_vAdd3, (const float *srcA, const float *srcB, float *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
    dst[i] = srcA[i] + srcB[i]; 
}

TMLIMP(void, tml_dr_vAdd3, (const double *srcA, const double *srcB, double *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
    dst[i] = srcA[i] + srcB[i]; 
}

TMLIMP(void, tml_sc_vAdd3, (const SCplx *srcA, const SCplx *srcB, SCplx *dst, int n))
{ 
  int i;
  for(i = 0; i < n; i++)
  { 
    dst[i].re = srcA[i].re + srcB[i].re;
    dst[i].im = srcA[i].im + srcB[i].im;
  }
}

TMLIMP(void, tml_sr_vSub1, (const float val, float *dst, int n))
{
  int i;
  for (i = 0; i < n; i++)
    dst[i] -= val;  
}

TMLIMP(void, tml_dr_vSub1, (const double val, double *dst, int n))
{
  int i;
  for (i = 0; i < n; i++)
    dst[i] -= val;  
}

TMLIMP(void, tml_sc_vSub1, (const SCplx val, SCplx *dst, int n))
{
  int i;
  for (i = 0; i < n; i++)
  {
    dst[i].re -= val.re;
    dst[i].im -= val.im;
  }  
}


TMLIMP(void, tml_sr_vSub2, (const float *src, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] -= src[i];	
}

TMLIMP(void, tml_dr_vSub2, (const double *src, double *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
    dst[i] -= src[i]; 
}

TMLIMP(void, tml_sc_vSub2, (const SCplx *src, SCplx *dst, int n))
{	
	int i;
	for(i = 0; i < n; i++)
	{	
		dst[i].re -= src[i].re;
		dst[i].im -= src[i].im;
	}
}

TMLIMP(void, tml_sc_vSub3, (const SCplx *src, const SCplx *val, SCplx *dst, int n))
{ 
  int i;
  for(i = 0; i < n; i++)
  { 
    dst[i].re = src[i].re - val[i].re;
    dst[i].im = src[i].im - val[i].im;
  }
}

TMLIMP(void, tml_sr_vSub3, (const float *src, const float *val, float *dst, int n))
{ 
  int i;
  for(i = 0; i < n; i++)
  { 
    dst[i] = src[i] - val[i];
  }
}

TMLIMP(void, tml_dr_vSub3, (const double *src, const double *val, double *dst, int n))
{ 
  int i;
  for(i = 0; i < n; i++)
  { 
    dst[i] = src[i] - val[i];
  }
}

TMLIMP(void, tml_sr_vMpy1, (const float val, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] *= val;	
}

TMLIMP(void, tml_dr_vMpy1, (const double val, double *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] *= val;	
}

TMLIMP(void, tml_sc_vMpy1, (const SCplx val, SCplx *dst, int n))
{
  int i;
  float tmp;
  for(i = 0; i < n; i++)
	{
    tmp = dst[i].re;
		dst[i].re = val.re * dst[i].re - val.im * dst[i].im;
		dst[i].im = val.re * dst[i].im + val.im * tmp;
	}
}

TMLIMP(void, tml_sr_vMpy2, (const float *src, float *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
		dst[i] *= src[i];	
}

TMLIMP(void, tml_dr_vMpy2, (const double *src, double *dst, int n))
{
	tml_dr_vMpy3(src, dst, dst, n);
}

TMLIMP(void, tml_sc_vMpy2, (const SCplx *src, SCplx *dst, int n))
{
	int i;
  float tmp;
	for(i = 0; i < n; i++)
	{
    tmp = dst[i].re;
		dst[i].re = src[i].re * dst[i].re - src[i].im * dst[i].im;
    dst[i].im = src[i].re * dst[i].im + src[i].im * tmp;
    
	}
}

TMLIMP(void, tml_sr_vMpy3, (const float *srcA, const float *srcB, float *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
    dst[i] = srcA[i] * srcB[i]; 
}

TMLIMP(void, tml_dr_vMpy3, (const double *srcA, const double *srcB, double *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
    dst[i] = srcA[i] * srcB[i]; 
}

TMLIMP(void, tml_sc_vMpy3, (const SCplx *srcA, const SCplx *srcB, SCplx *dst, int n))
{
  int i;
  for(i = 0; i < n; i++)
  {
    dst[i].re = srcA[i].re * srcB[i].re - srcA[i].im * srcB[i].im;
    dst[i].im = srcA[i].re * srcB[i].im + srcA[i].im * srcB[i].re;
  }
}

TMLIMP(float, tml_sr_vSum,(const float *src, int n))
{ 
	int i;
	float ret = 0.0f;
	for(i = 0; i < n; i++)
		ret += (float)src[i];	
	return (float)ret;
}

TMLIMP(double, tml_dr_vSum,(const double *src, int n))
{ 
  int i;
  double ret = 0.0f;
  for(i = 0; i < n; i++)
    ret += src[i];  
  return ret;
}

TMLIMP(void, tml_sc_vSum,(const SCplx *src, int n, SCplx *sum))
{
	int i;
	sum[0].re = 0;
	sum[0].im = 0;

	for(i = 0; i < n; i++)
	{	
		sum[0].re += src[i].re;
		sum[0].im += src[i].im;
	}
}

TMLIMP(float, tml_sr_vMean,(const float *vec,int n))
{
	return tml_sr_vSum(vec, n) / (float)n;
}

TMLIMP(double, tml_dr_vMean,(const double *vec, int n))
{
  return tml_dr_vSum(vec, n) / (double)n;
}


TMLIMP(void, tml_sr_vNormalize, (const float *src, float *dst, int n, float
offset, float factor))
{
	tml_sr_vCopy(src, dst, n);
	tml_sr_vAdd1(-offset, dst, n);
	tml_sr_vMpy1( (float)(1/factor), dst, n); 
}

TMLIMP(void, tml_dr_vNormalize, (const double *src, double *dst, int n, double
offset, double factor))
{
	tml_dr_vCopy(src, dst, n);
	tml_dr_vAdd1(-offset, dst, n);
	tml_dr_vMpy1((double)(1/factor), dst, n); 		
}

TMLIMP(void, tml_sc_vNormalize, (const SCplx *src, SCplx *dst, int n, SCplx
offset, float factor))
{
	/*
  tml_sc_vCopy(src, dst, n);
  offset.re = -offset.re;
  offset.im = -offset.im;
  tml_sc_vAdd1(offset, dst, n);
  
  tml_sc_vMpy1((float)(1/factor), dst, n); */ /* doesnt work, 1st arg must be complex*/
  
  int i;
	for(i = 0; i < n; i++)
	{	
		dst[i].re = (src[i].re - offset.re) / factor;
		dst[i].im = (src[i].im - offset.im) / factor;
	}
}

TMLIMP(void, tml_sc_vConj1, (SCplx *vec, int n))
{
	int i;
	for(i = 0; i < n; i++)
		vec[i].im = -vec[i].im;
}

TMLIMP(void, tml_sc_vConj2, (const SCplx *src, SCplx *dst, int n))
{
	int i;
	for(i = 0; i < n; i++)
  {
		dst[i].im = -src[i].im;
    dst[i].re = src[i].re;
  }
}
TMLIMP(void, tml_sr_v2RealToCplx, (const float *srcReal, const float *srcImag, SCplx *dst, int n))
{
	int i;
	for (i = 0; i < n; i++)
	{	
		dst[i].re = srcReal[i];
		dst[i].im = srcImag[i];
	}
}

TMLIMP(void, tml_sc_vReal, (const SCplx *src, float *dst, int n))
{
	int i;
	for (i = 0; i < n; i++)
		dst[i]= src[i].re;
}

TMLIMP(void, tml_sr_vSqrt1, (float *vec, int n))
{
	int i;
	for (i = 0; i < n; i++)
		vec[i]= (float)sqrt(vec[i]);
}

TMLIMP(void, tml_dr_vSqrt1, (double *vec, int n))
{
  int i;
  for (i = 0; i < n; i++)
    vec[i]= (double)sqrt(vec[i]);
}


TMLIMP(void, tml_sc_vSqrt1, (SCplx *vec, int n))
{
	int i;
  float mag_sqrt, ang_half;

  for (i = 0; i < n; i++)
	{	
    mag_sqrt = pow(vec[i].re * vec[i].re + vec[i].im * vec[i].im, 0.25f);
    ang_half = atan2(vec[i].im, vec[i].re) * 0.5f;
    vec[i].re = mag_sqrt * cos(ang_half);
    vec[i].im = mag_sqrt * sin(ang_half);
  }
}

TMLIMP(void, tml_sc_vCplxTo2Real, (const SCplx *src, float *dstReal,
float *dstImag, int n))
{
	int i;
	tml_sc_vReal(src, dstReal, n);
	for (i = 0; i < n; i++)
		dstImag[i]= src[i].im;
}

TMLIMP(void, tml_sr_vAbs1,(float *vec, int n))
{
  tml_sr_vAbs2(vec, vec, n);
}

TMLIMP(void, tml_dr_vAbs1,(double *vec, int n))
{
  tml_dr_vAbs2(vec, vec, n);
}

TMLIMP(void, tml_sr_vAbs2,(const float *src, float *dst, int n))
{
	int i;
	for (i = 0; i < n; i++)
		dst[i] = (float)fabs(src[i]);
}

TMLIMP(void, tml_dr_vAbs2,(const double *src, double *dst, int n))
{
  int i;
  for (i = 0; i < n; i++)
    dst[i] = (double)fabs(src[i]);
}


TMLIMP(void, tml_sc_vMag, (const SCplx *src, float *mag, int n))
{
	int i;
	for (i = 0; i < n; i++)
		mag [i] = (float)sqrt(src[i].re * src[i].re + src[i].im * src[i].im); 
}

TMLIMP(void, tml_sr_vPolarToCart,(const float *mag, const float *phase,
SCplx *dst, int len))
{
	int i;
	for (i = 0; i < len; i++)
	{	
		dst[i].re = (float)(mag[i] * cos(phase[i]));
		dst[i].im = (float)(mag[i] * sin(phase[i]));
	}
}

TMLIMP(void, tml_sc_vCartToPolar,(const SCplx *src, float *mag, float *phase,
int len))
{
	tml_sc_vMag(src, mag, len);
	tml_sc_vPhase(src, phase, len);
}

TMLIMP(void, tml_sc_vPhase, (const SCplx *src, float *phase, int n))
{
	int i;
	for (i = 0; i < n; i++)
		phase[i] = (float)atan2(src[i].im, src[i].re);
}

TMLIMP(void, tml_sr_vrPhase, (const float *srcReal, const float *srcImag,
float *phase, int n))
{
	int i;
	for (i = 0; i < n; i++)
		phase[i] = (float)atan2(srcImag[i], srcReal[i]);
}

TMLIMP(void, tml_sr_vZero, (float *dst, int n))
{
	tml_sr_vSet(0, dst, n);
}

TMLIMP(void, tml_dr_vZero, (double *dst, int n))
{
	tml_dr_vSet(0, dst, n);
}

TMLIMP(void, tml_sc_vZero, (SCplx *dst, int n))
{
	tml_sc_vSet(0, 0, dst, n);
}

TMLIMP(void, tml_sr_vCopy, (const float *src, float *dst, int n))
{
	memcpy(dst, src, n * sizeof(float));
}

TMLIMP(void, tml_dr_vCopy, (const double *src, double *dst, int n))
{
	memcpy(dst, src, n * sizeof(double));
}

TMLIMP(void, tml_sc_vCopy, (const SCplx *src, SCplx *dst, int n))
{
	memcpy(dst, src, n * sizeof(SCplx));
}

TMLIMP(float, tml_sr_vMax, (const float *vec, int n))
{
	int index;

	return tml_sr_vMaxExt(vec, n, &index);
}

TMLIMP(double, tml_dr_vMax, (const double *vec, int n))
{
  int index;
  return tml_dr_vMaxExt(vec, n, &index);
}

TMLIMP(float, tml_sr_vMaxExt, (const float *vec, int n, int *index))
{
	int i;
	float ret = vec[0];
	*index = 0; 
	for (i = 1; i < n ; i++)
	{
		if (vec[i] > ret) 
		{	
			ret = vec[i];
			*index = i;	
		}
	}
	return ret;
}

TMLIMP(double, tml_dr_vMaxExt, (const double *vec, int n, int *index))
{
  int i;
  double ret = vec[0];
  *index = 0; 
  for (i = 1; i < n ; i++)
  {
    if (vec[i] > ret) 
    { 
      ret = vec[i];
      *index = i; 
    }
  }
  return ret;
}

TMLIMP(float, tml_sr_vMin, (const float *vec, int n))
{
	int index;

	return tml_sr_vMinExt(vec, n, &index);
}

TMLIMP(double, tml_dr_vMin, (const double *vec, int n))
{
  int index;

  return tml_dr_vMinExt(vec, n, &index);
}

TMLIMP(float, tml_sr_vMinExt, (const float *vec, int n, int *index))
{
	int i;
	float ret = vec [0];
	*index = 0;
	for (i = 1; i < n ; i++)
	{
		if (vec[i] < ret) 
		{	
			ret = vec[i];
			*index = i;
		}
	}
	return ret;
}

TMLIMP(double, tml_dr_vMinExt, (const double *vec, int n, int *index))
{
  int i;
  double ret = vec [0];
  *index = 0;
  for (i = 1; i < n ; i++)
  {
    if (vec[i] < ret) 
    { 
      ret = vec[i];
      *index = i;
    }
  }
  return ret;
}

float ceilf_tml(float f)
{
#ifdef HAVE_CEILF
return ceilf(f);
#else
return (float)ceil((double)f);
#endif
}

float floorf_tml(float f)
{
#ifdef HAVE_FLOORF
return floorf(f);
#else
return (float)floor((double)f);
#endif
}

float trunczero(float f)
{
  double intpart;
  modf(f, &intpart);
  return (float)intpart;
/*  if (f < 0)
  {
    return ceilf(f);
  }
  else
  {
    return floorf(f);
  }*/
}

#ifndef HAVE_ROUND
double round(double f)
{
  double intpart, floatpart;
    
  floatpart = modf(f, &intpart);
  if (fabs(floatpart) >= 0.5f)
  {
    if (floatpart >= 0.5f)
      intpart += 1.0f; 
    else if (floatpart <= 0.5f)
      intpart -= 1.0f;
  }   
  return intpart;
}
#endif


#ifndef HAVE_ROUNDF
float roundf(float f)
{
  return (float)round((double)f);
}
#endif

TMLIMP(void, tml_sr_vFloatToInt,(const float *src, void *dst, int len, int wordSize, int flags))
{
	int i;
	int Sbytes = (int)(wordSize / 8);
  float (*rounding)(float);
  float roundf(float);
  int bunsig = FALSE;

  if (flags & TML_Round)
	rounding = &roundf;
  else if (flags & TML_TruncNeg)
	rounding = &floorf_tml;
	else if (flags & TML_TruncZero)
	rounding = &trunczero; 
  else
	rounding = &roundf; /* defaulting to roundf*/
  if (flags & TML_Unsigned)
	bunsig = TRUE;
 #ifdef FASTASSG
#undef FASTASSG
#endif	
 #define FASTASSG(u) if (!bunsig) ((u*)dst)[i] = (u)(rounding(src[i])); else ((unsigned u*)dst)[i] = (unsigned u)(rounding(src[i]));
  if (Sbytes == sizeof(char))
		for (i = 0; i < len; i++)
			FASTASSG(char)
	else
	{
		if (Sbytes == sizeof(short))	
			for (i = 0; i < len; i++)
				FASTASSG(short)
		else 
		{			
			if (Sbytes == sizeof(int))
				for (i = 0; i < len; i++)				
					FASTASSG(int)
			else 
			{	
				if (Sbytes == sizeof(long))
					for (i = 0; i < len; i++)
						FASTASSG(long)
				else 
				{	
					if (Sbytes == sizeof(__INT64))
						for (i = 0; i < len; i++)		
							FASTASSG(__INT64)
				}
			}
		}
	}
}


double trunczerod(double f)
{
  double intpart;
  modf(f, &intpart);
  return intpart;
/*  if (f < 0)
    return ceil(f);
  else
    return floor(f);
    */
}

TMLIMP(void, tml_dr_vFloatToInt,(const double *src, void *dst, int len, int wordSize, int flags))
{
  int i;
  int Sbytes = (int)(wordSize / 8);
  double (*rounding)(double);
  double round(double);
  int bunsig = false;

  if (flags & TML_Round)
	rounding = &round;
  else if (flags & TML_TruncNeg)
	rounding = &floor;
  else if (flags & TML_TruncZero)
	rounding = &trunczerod;
  else
	rounding = &round;
  if (flags & TML_Unsigned)
	bunsig = TRUE;
#ifdef FASTASSG
#undef FASTASSG
#endif	
 #define FASTASSG(u) if (!bunsig) ((u*)dst)[i] = (u)(src[i]); else ((unsigned u*)dst)[i] = (unsigned u)(src[i]);
  if (Sbytes == sizeof(char))
    for (i = 0; i < len; i++)
       FASTASSG(char)
  else
  {
    if (Sbytes == sizeof(short))  
      for (i = 0; i < len; i++)
        FASTASSG(short)
    else 
    {     
      if (Sbytes == sizeof(int))
        for (i = 0; i < len; i++)       
          FASTASSG(int)
      else 
      { 
        if (Sbytes == sizeof(long))
          for (i = 0; i < len; i++)       
            FASTASSG(long)
        else 
        { 
          if (Sbytes == sizeof(__INT64))
            for (i = 0; i < len; i++)   
              FASTASSG(__INT64)
        }
      }
    }
  }
}

TMLIMP(void, tml_sr_vIntToFloat,(const void *src, float *dst, int len, int wordSize))
{
	int i;
	int Sbytes = (int)(wordSize / 8);
  if (Sbytes == sizeof(char))
		for (i = 0; i < len; i++)
          dst[i] = (float)(((char*)src)[i]);
	else 
	{
		if (Sbytes == sizeof(short))	
			for (i = 0; i < len; i++)
				dst[i] = (float)(((short*)src)[i]);		
		else 
		{
			if (Sbytes == sizeof(int))
				for (i = 0; i < len; i++)				
					dst[i] = (float)(((int*)src)[i]);
			else 
			{
				if (Sbytes == sizeof(long))
					for (i = 0; i < len; i++)				
						dst[i] = (float)(((long*)src)[i]);	
				else 
				{
					if (Sbytes == sizeof(__INT64))
						for (i = 0; i < len; i++)		
							dst[i] = (float)(((__INT64*)src)[i]);
				}
			}
		}
	}
}

TMLIMP(void, tml_dr_vIntToFloat,(const void *src, double *dst, int len, int wordSize))
{
  int i;
  int Sbytes = (int)(wordSize / 8);
  if (Sbytes == sizeof(char))
    for (i = 0; i < len; i++)
          dst[i] = (double)(((char*)src)[i]);
  else 
  {
    if (Sbytes == sizeof(short))  
      for (i = 0; i < len; i++)
        dst[i] = (double)(((short*)src)[i]);   
    else 
    {
      if (Sbytes == sizeof(int))
        for (i = 0; i < len; i++)       
          dst[i] = (double)(((int*)src)[i]);
      else 
      {
        if (Sbytes == sizeof(long))
          for (i = 0; i < len; i++)       
            dst[i] = (double)(((long*)src)[i]);  
        else 
        {
          if (Sbytes == sizeof(__INT64))
            for (i = 0; i < len; i++)   
              dst[i] = (double)(((__INT64*)src)[i]);
        }
      }
    }
  }
}

/* a define to avoid typing the function twice */
#define tml_vThresh1 \
  int i; \
  if (relOP == TML_LT) \
  { \
    for (i = 0; i < n; i++)\
    {\
      if (vec[i] < thresh)\
        vec[i] = thresh;\
    }\
  }\
  else if (relOP == TML_GT)\
  {\
    for (i = 0; i < n; i++)\
      if (vec[i] > thresh)\
        vec[i] = thresh; \
  }


TMLIMP(void, tml_sr_vThresh1, (float *vec, int n, float thresh,int relOP))
{
  tml_vThresh1
}

TMLIMP(void, tml_dr_vThresh1, (double *vec, int n, double thresh,int relOP))
{
  tml_vThresh1
}

TMLIMP(void, tml_sc_vThresh1, (SCplx *vec, int n, float thresh, int relOP))
{
	int i;
	float *mag = tml_sr_Malloc(n); 
	float *phase = tml_sr_Malloc(n);
	tml_sc_vCartToPolar(vec, mag, phase, n);
	if (relOP == TML_LT)
	{	
		for (i = 0; i < n; i++)
		{
			if (mag[i] < thresh)
				mag[i] = thresh;
		}
	}
	else if (relOP == TML_GT)
	{
		for (i = 0; i < n; i++)
			if (mag[i] > thresh)
				mag[i] = thresh; 
	}
	tml_sr_vPolarToCart(mag, phase, vec, n);
	tml_Free(mag);
	tml_Free(phase);
}

TMLIMP(void, tml_Free, (void* ptr))
{
  if (ptr != NULL)
  {
	 free(ptr);
  }
}

TMLIMP(float*, tml_sr_Malloc, (int length))
{
	return (float*)malloc(length*sizeof(float));
}

TMLIMP(double*, tml_dr_Malloc, (int length))
{
	return (double*)malloc(length*sizeof(double));
}

TMLIMP(SCplx*, tml_sc_Malloc, (int length))
{
	return (SCplx*)malloc(length*sizeof(SCplx));
}


TMLIMP(SCplx, tml_sc_vDotProd, (const SCplx *a, const SCplx *b, int len))
{
  int i;
  SCplx res = {0.0f, 0.0f};
  float* tmp1 = (float*)a;
  float* tmp2 = (float*)b;
  for (i = 0; i < len; i++)
  {
    res.re += (tmp1[2*i] * tmp2[2*i] - tmp1[2*i+1] * tmp2[2*i+1]);
    res.im += (tmp1[2*i] * tmp2[2*i+1] + tmp1[2*i+1] * tmp2[2*i]);
  }
  return res;
}

TMLIMP(float, tml_sr_vDotProd, (const float *a, const float *b, int len))
{
#ifdef SSE3
  //SIMD intrinsics implementation(SSE3). This builds in Visual Studio 8 or newer.
  int i;
  float res = 0.0f;
  int rem = len%4;
  __m128 r1, r2, r3, r4;
  r4 = _mm_setzero_ps(); //sets sum to zero  
  if ((len-rem) > 0)
  {	
	for(i=0; i<(len-rem); i+=4)
	{
		//_mm_load_ps is much faster - data must be previously aligned
        r1 = _mm_loadu_ps(a+i); //loads unaligned array a into r1 = a[3]  a[2]  a[1]  a[0]    
		r2 = _mm_loadu_ps(b+i); //loads unaligned array b into r2 = b[3]  b[2]  b[1]  b[0]     
		r3 = _mm_mul_ps(r1, r2); //performs multiplication r3 = a[3]*b[3]  a[2]*b[2]  a[1]*b[1]  a[0]*b[0]
		r3 = _mm_hadd_ps(r3, r3); //horizontal addition    r3 = a[3]*b[3]+a[2]*b[2]  a[1]*b[1]+a[0]*b[0]  a[3]*b[3]+a[2]*b[2]  a[1]*b[1]+a[0]*b[0]
		r4 = _mm_add_ps(r4, r3); //vertical addition (i=0) r4 = a[3]*b[3]+a[2]*b[2]  a[1]*b[1]+a[0]*b[0]  a[3]*b[3]+a[2]*b[2]  a[1]*b[1]+a[0]*b[0]
	}
	r4 = _mm_hadd_ps(r4, r4); //(i=0) r4 = a[3]*b[3]+a[2]*b[2]+a[1]*b[1]+a[0]*b[0] | a[3]*b[3]+a[2]*b[2]+a[1]*b[1]+a[0]*b[0] | a[3]*b[3]+a[2]*b[2]+a[1]*b[1]+a[0]*b[0] | a[3]*b[3]+a[2]*b[2]+a[1]*b[1]+a[0]*b[0]
	_mm_store_ss(&res,r4); //store the first element in the vector (i=0) a[3]*b[3]+a[2]*b[2]+a[1]*b[1]+a[0]*b[0]
  }
  if (rem!=0)
  {
	for (i=(len-rem);i<len;i++)
		res+=a[i]*b[i];
  }
  return res;
#else
//C implementation
  int i;
  float res = 0.0f;
  for (i = 0; i < len; i++)
  {
    res += a[i] * b[i];
  }
  return res;
#endif
}


TMLIMP(double, tml_dr_vDotProd, (const double *a, const double *b, int len))
{
  int i;
  double res = 0.0f;
  for (i = 0; i < len; i++)
  {
    res += a[i] * b[i];
  }
  return res;
}

TMLIMP(SCplx, tml_scr_vDotProd, (const SCplx *a, const float *b, int len))
{
#ifdef SSE3
    int i;
    float * at = (float*)a;
    float tmp[4];
    int rem = (2*len)%4;
    SCplx res = {0.0f, 0.0f};
    int j = 0;
    __m128 r1, r2, r3, r4;
    r4 = _mm_setzero_ps(); //sets sum to zero  
    
    if (2*len-rem > 0)
    {
        for (i=0;i<(2*len-rem);i+=4)
        {
            //would be more efficient with an _mm_load_ps (aligned data)
            /*align data in the filter function*/
            r1 = _mm_set_ps(at[i],at[i+2],at[i+1],at[i+3]);
            r2 = _mm_set_ps(b[j],b[j+1],b[j],b[j+1]);
            r3 = _mm_mul_ps(r1, r2);  //r3 = at[0]*b[0] at[2]*b[1] at[1]*b[0] at[3]*b[1] 
            r4 = _mm_add_ps(r4, r3);  
            j+=2;
        }
        r4 = _mm_hadd_ps(r4, r4);
        //could do here something more efficient ...
        _mm_storeu_ps(&tmp,r4);
        res.re=tmp[0];
        res.im=tmp[1];
    }
    if (rem != 0) 
    {
        //only 2 samples remaining
        res.re+= at[2*len-rem] * b[j];
        res.im+= at[2*len-rem+1] * b[j];
    }
    return res;
#else
  int i;
  SCplx res = {0.0f, 0.0f};
  float * tmpin = (float*)a;
  for (i = 0; i < len; i++)
  {
    res.re += tmpin[2*i] * b[i];
    res.im += tmpin[2*i+1] * b[i];
  }
  return res;
#endif
}
