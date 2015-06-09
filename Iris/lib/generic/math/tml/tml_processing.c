/**
 * \file tml_processing.c
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
 * Implementation of the signal processing operations for the
 * Trinity Math Library.
 */

#define TML_W32DLL
#include "tml.h"
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <math.h>

TMLIMP(void, tml_sr_Conv, (const float *x, int xLen, const float *h, int hLen,
float *y))
{
   /* METHOD using the dotprod - seems to be the fastest */
  int i, yLen, hi, xi, cLen;
  int hl, xl;
  const float *htmp, *xtmp;
  float *hinv;

  /* check for the longer vector (htmp is always the shorter one) */  
  if (hLen > xLen)
  {
    hl = xLen;  xl = hLen;
    htmp = x;   xtmp = h;
  }
  else
  {
    hl = hLen; xl = xLen;
    htmp = h; xtmp = x;
  } 
  
  hinv = tml_sr_Malloc(hl);

  yLen = xl + hl - 1;
    
  /* copy h in a temporary vector in reverse order */
  for (i = 0; i < hl; i++)
    hinv[hl-i-1] = htmp[i];
 
  for (i = 0; i < yLen; i++)
  {
    hi = MAX(hl - i - 1, 0);
    xi = MAX(0, i - hl +1);
    if (i + 1 < hl)
      cLen = i + 1;
    else if (i  > yLen - hl - 1)
      cLen = yLen - i;
    else
      cLen = hl;
          
    /* compute the dot product */
    y[i] = tml_sr_vDotProd(hinv+hi, xtmp+xi, cLen);  
  }
  
  /* free the temporary taps vector */
  free((float*)hinv);
} 

TMLIMP(void, tml_sc_Conv, (const SCplx *x, int xLen, const SCplx *h, int hLen,
SCplx *y))
{
  /* METHOD using the dotprod - seems to be the fastest */
  int i, yLen, hi, xi, cLen;
  int hl, xl;
  const SCplx *htmp, *xtmp;
  SCplx *hinv;

  /* check for the longer vector (htmp is always the shorter one) */  
  if (hLen > xLen)
  {
    hl = xLen;  xl = hLen;
    htmp = x;   xtmp = h;
  }
  else
  {
    hl = hLen; xl = xLen;
    htmp = h; xtmp = x;
  } 
  
  hinv = tml_sc_Malloc(hl);

  yLen = xl + hl - 1;
    
  /* copy h in a temporary vector in reverse order */
  for (i = 0; i < hl; i++)
    hinv[hl-i-1] = htmp[i];
 
  for (i = 0; i < yLen; i++)
  {
    hi = MAX(hl - i - 1, 0);
    xi = MAX(0, i - hl +1);
    if (i + 1 < hl)
      cLen = i + 1;
    else if (i  > yLen - hl - 1)
      cLen = yLen - i;
    else
      cLen = hl;
          
    /* compute the dot product */
    y[i] = tml_sc_vDotProd(hinv+hi, xtmp+xi, cLen);  
  }
  
  /* free the temporary taps vector */
  free((SCplx*)hinv);
} 

/*----------Low Level FIR filtering----------*/

/**
The structure TMLFirTapState defines the length of the
FIR filter, tapsLen, and a pointer to the taps[n] array. 

In addition, the contents of the taps[n] array can be permuted in an
implementation-dependent way to allow faster filtering. The pointer
tapStPtr is used in subsequent calls to reference the taps and filter
structure.
*/

TMLIMP(void, tml_sc_FirlInit,(SCplx *taps, int tapsLen, TMLFirTapState *tapStPtr))
{
	tapStPtr->taps = taps;
	tapStPtr->len = tapsLen;
/*	
	tapStPtr->downFactor
	tapStPtr->downPhase
	tapStPtr->isFilterMode
	tapStPtr->isMultiRate
	tapStPtr->tapsFactor
	tapStPtr->upFactor
	tapStPtr->upPhase
*/
}

TMLIMP(void, tml_sr_FirlInit,(float *taps, int tapsLen, TMLFirTapState *tapStPtr))
{
  tapStPtr->taps = taps;
  tapStPtr->len = tapsLen;
/*  
  tapStPtr->downFactor
  tapStPtr->downPhase
  tapStPtr->isFilterMode
  tapStPtr->isMultiRate
  tapStPtr->tapsFactor
  tapStPtr->upFactor
  tapStPtr->upPhase
*/
}

TMLIMP(void, tml_sc_FirlInitDlyl,(const TMLFirTapState *tapStPtr, SCplx *dlyl, TMLFirDlyState *dlyStPtr))
{
	dlyStPtr->dlyl = dlyl;
	dlyStPtr->len = tapStPtr->len;
	/*
	dlyStPtr->isFilterMode
	*/
}

TMLIMP(void, tml_sr_FirlInitDlyl,(const TMLFirTapState *tapStPtr, float *dlyl, TMLFirDlyState *dlyStPtr))
{
  dlyStPtr->dlyl = dlyl;
  dlyStPtr->len = tapStPtr->len;
  /*
  dlyStPtr->isFilterMode
  */
}

TMLIMP(void, tml_sc_FirlSetDlyl, (const TMLFirTapState *tapStPtr, SCplx *inDlyl,
TMLFirDlyState *dlylStPtr))
{
	dlylStPtr->dlyl = inDlyl;
	/* Only the delay line itself
	dlylStPtr->len = tapStPtr->len;
	dlylStPtr->isFilterMode
	*/	
}

TMLIMP(void, tml_sr_FirlSetDlyl, (const TMLFirTapState *tapStPtr, float *inDlyl,
TMLFirDlyState *dlylStPtr))
{
  dlylStPtr->dlyl = inDlyl;
  /* Only the delay line itself
  dlylStPtr->len = tapStPtr->len;
  dlylStPtr->isFilterMode
  */  
}

TMLIMP(void, tml_sc_FirlSetTaps, (SCplx *inTaps, TMLFirTapState *tapStPtr))
{
	tapStPtr->taps = inTaps;
	/*Only the taps themselfs*/
}

TMLIMP(void, tml_sr_FirlSetTaps, (float *inTaps, TMLFirTapState *tapStPtr))
{
  tapStPtr->taps = inTaps;
  /*Only the taps themselfs*/
}

TMLIMP(float, tml_sr_Firl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const float samp))
{
  float ret;
  
  tml_sr_vFirl(tapStPtr, dlyStPtr, &samp, &ret, 1);
  
  return ret; 
} 

TMLIMP(void, tml_sr_vFirl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const float *inSamps,
float *outSamps, int numIters))
{
  /* assumes dlyl = [ x[-n-1] ... x[-3] x[-2] x[-1] ]   */

  int i, n, dlylen;
  float *temp = (float*)dlyStPtr->dlyl;
  float *taps = tml_sr_Malloc(tapStPtr->len);

  dlylen = dlyStPtr->len;
  
  /* copy the taps in a temporary vector in reverse order */
  for (i = 0; i < tapStPtr->len; i++)
    taps[tapStPtr->len-i-1] = ((float*)(tapStPtr->taps))[i];
 
  for (n = 0; n < numIters; n++)
  {
    /* shift vector one sample to the left and insert the new input at the end */
    memmove(temp, temp+1, (dlylen-1)*sizeof(float)); 
    temp[dlylen-1] = inSamps[n]; 
    
    /* compute the dot product */
    outSamps[n] = tml_sr_vDotProd(temp, taps, tapStPtr->len);  
  }
  
  /* free the temporary taps vector */
  free((float*)taps);
}

TMLIMP(SCplx, tml_sc_Firl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const SCplx samp))
{
  SCplx ret;
  
  tml_sc_vFirl(tapStPtr, dlyStPtr, &samp, &ret, 1);
  
  return ret; 
} 


TMLIMP(void, tml_sc_vFirl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))
{

/* ----- METHOD 1: direct computation ---------- (faster) */
  /* assumes dlyl = [ x[-n-1] ... x[-3] x[-2] x[-1] ]   */

  int i, n, dlylen;
  SCplx *temp = (SCplx*)dlyStPtr->dlyl;
  SCplx *taps = tml_sc_Malloc(tapStPtr->len);

  dlylen = dlyStPtr->len;
  
  /* copy the taps in a temporary vector in reverse order */
  for (i = 0; i < tapStPtr->len; i++)
    taps[tapStPtr->len-i-1] = ((SCplx*)(tapStPtr->taps))[i];
 
  for (n = 0; n < numIters; n++)
  {
    /* shift vector one sample to the left and insert the new input at the end */
    memmove(temp, temp+1, (dlylen-1)*sizeof(SCplx)); 
    temp[dlylen-1] = inSamps[n]; 
    
    /* compute the dot product */
    outSamps[n] = tml_sc_vDotProd(temp, taps, tapStPtr->len);  
  }
  
  /* free the temporary taps vector */
  free((SCplx*)taps);
	
  /* ----- METHOD 2: using convolution ---------- (slowerr) */
  //Method 2: Probably a more efficient method using convolution
	//assumes dlyl = [x[-3] x[-2] x[-1]]  // ok
/*
	int concatLen = numIters + dlyStPtr->len;
	
	SCplx *temp = tml_sc_Malloc(concatLen + tapStPtr->len - 1);
	SCplx *concat = tml_sc_Malloc(concatLen);

	//Append the delay line vector to the input vector
	tml_sc_vCopy(dlyStPtr->dlyl, concat, dlyStPtr->len);

	tml_sc_vCopy(inSamps, &concat[dlyStPtr->len], numIters);

	//temp is the result of the convolution of the appended vector with the filter taps
	tml_sc_Conv(tapStPtr->taps, tapStPtr->len, concat, concatLen,  temp);

	//Copy begining at tapStPtr->len to the output vector (lenght numIters)
	tml_sc_vCopy(&temp[tapStPtr->len], outSamps, numIters);

	//New delay line
	tml_sc_vCopy(&concat[concatLen - dlyStPtr->len], dlyStPtr->dlyl,dlyStPtr->len);

	free((SCplx*)temp);
	free((SCplx*)concat);
	//End of Method 2 ;
   * */
}

TMLIMP(void, tml_scr_vFirl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))
{
    /* assumes dlyl = [ x[-n-1] ... x[-3] x[-2] x[-1] ]   */

  int i, n, dlylen;
  SCplx *temp = (SCplx*)dlyStPtr->dlyl;
  float *taps = tml_sr_Malloc(tapStPtr->len);

  dlylen = dlyStPtr->len;
  
  /* copy the taps in a temporary vector in reverse order */
  for (i = 0; i < tapStPtr->len; i++)
    taps[tapStPtr->len-i-1] = ((float*)(tapStPtr->taps))[i];
 
  for (n = 0; n < numIters; n++)
  {
    /* shift vector one sample to the left and insert the new input at the end */
    memmove(temp, temp+1, (dlylen-1)*sizeof(SCplx)); 
    temp[dlylen-1] = inSamps[n]; 
    
    /* compute the dot product */
    outSamps[n] = tml_scr_vDotProd(temp, taps, tapStPtr->len);  
  }
  
  /* free the temporary taps vector */
  free((float*)taps);
}


/*----------Normal FIR filtering----------*/

/**
\todo all high level filter initialization function are not working - need to be properly tested
use low level initialization funcction instead
*/
TMLIMP(void, tml_sr_FirInit, (const float *tapVals, int tapsLen,
const float *dlyVals, TMLFirState *statePtr))
{
	statePtr->taps = (void*)tapVals;
	statePtr->tapsLen = tapsLen;
	statePtr->dlyl = (void*)dlyVals;
	
	//statePtr->useDlyl = 0;
/*	
	statePtr->dlylLimit
	statePtr->downFactor
	statePtr->downPhase
	statePtr->inpLen
	statePtr->dlylLen
	statePtr->isInit
	statePtr->isMultiRate
	statePtr->sect
	statePtr->tapsBlk
	statePtr->tapsFactor
	statePtr->upFactor
	statePtr->upPhase
	statePtr->useInp
	statePtr->utapsLen
*/
}

TMLIMP(float, tml_sr_Fir, (TMLFirState *statePtr, float samp))
{
	float ret;

	tml_sr_vFir(statePtr, &samp, &ret, 1);
	return ret;
}
/**
\todo not tested - use low level FIR instead
*/
TMLIMP(void, tml_scr_vFir, (TMLFirState *statePtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))
{
  
	TMLFirTapState tapStPtr; /* = malloc(sizeof(TMLFirTapState));*/ 
	TMLFirDlyState dlyStPtr; /* = malloc(sizeof(TMLFirDlyState));*/  

	tapStPtr.taps = statePtr->taps; 
	tapStPtr.len = statePtr->tapsLen; 
	dlyStPtr.dlyl = statePtr->dlyl;
	dlyStPtr.len = statePtr->dlylLen;
	
	tml_scr_vFirl(&tapStPtr, &dlyStPtr, inSamps, outSamps, numIters);

/*
	free(tapStPtr);
	free(dlyStPtr);
  */
  
}

TMLIMP(void, tml_sr_vFir,(TMLFirState *statePtr, const float *inSamps,
float *outSamps, int numIters) )
{
	TMLFirTapState *tapStPtr = malloc(sizeof(TMLFirTapState)); 
	TMLFirDlyState *dlyStPtr = malloc(sizeof(TMLFirTapState));  

	tapStPtr->taps = statePtr->taps; 
	tapStPtr->len = statePtr->tapsLen; 
	dlyStPtr->dlyl = statePtr->dlyl;
	dlyStPtr->len = statePtr->dlylLen;

	tml_sr_vFirl(tapStPtr, dlyStPtr, inSamps, outSamps, numIters);
	
	free(tapStPtr);
	free(dlyStPtr);
}


TMLIMP(int, tml_dr_FirBandpass, (double rLowFreq, double rHighFreq,
double *taps, int tapsLen, TML_WindowType winType))
{
	/*use an even number of taps*/
	int i; /*, index;*/
	double *temp = tml_dr_Malloc(tapsLen);

	//for (i = -((tapsLen)-1)/2; i <= ((tapsLen)-1)/2; i++)
	for (i = -((tapsLen)-1)/2; i <= ((tapsLen)-1)/2 + 1; i++)
		temp[i] = (sin(TML_PI * 2 * rHighFreq * i) / (TML_PI * i)) - (sin(TML_PI * 2 * rLowFreq * i) / (TML_PI * i));  

	temp[0] = 2 * (rHighFreq - rLowFreq); //(for sinc(0) = 1)

	if (winType == TML_WinHamming)
		tml_dr_WinHamming(temp, tapsLen);

	//tml_dr_vCopy(&temp[-((tapsLen)-1)/2], &taps[-((tapsLen)-1)/2], tapsLen);
	tml_dr_vCopy(&temp[-((tapsLen)-1)/2], taps, tapsLen);

	return 0;
	free(temp);
}

TMLIMP(int, tml_dr_FirLowpass, (double rfreq, double *taps, int tapsLen,
TML_WindowType winType))
{
	/*use an even number of taps*/

	int i;
	double *temp = tml_dr_Malloc(tapsLen);

	//for (i = -((tapsLen)-1)/2; i <= ((tapsLen)-1)/2; i++)
	for (i = -((tapsLen)-1)/2; i <= ((tapsLen)-1)/2 + 1; i++)
		temp[i] = sin(TML_PI * 2 * rfreq * i)/(TML_PI * i); 
	
	temp[0] =  2 * rfreq; //(for sinc(0) = 1)

	if (winType == TML_WinHamming)
		tml_dr_WinHamming(temp, tapsLen);

	//tml_dr_vCopy(&temp[-((tapsLen)-1)/2], &taps[-((tapsLen)-1)/2], tapsLen);		
	tml_dr_vCopy(&temp[-((tapsLen)-1)/2], taps, tapsLen);		

	return 0;
	free(temp);
}

TMLIMP(void, tml_FirFree, (TMLFirState *statePtr))
{
	tml_Free(statePtr);
}

TMLIMP(void, tml_sr_WinHann, (float *vec, int N))
{
	/* vec range is [0;N]*/
	int i;
	float *w = tml_sr_Malloc(N);
	
	for(i = 0; i < N; i++)
		w[i] = (float) (0.5 * (1 - cos(2*TML_PI*i/(N-1))));
	tml_sr_vMpy2(w, vec, N);
}

TMLIMP(void, tml_dr_WinHamming, (double *vec, int N))
{
	/* vec range is [-(N-1)/2;(N-1)/2]*/
	int i;
	double *w = tml_dr_Malloc(N);
	
	for(i = 0; i < N; i++)
	{	
		w[i] = 0.54 - 0.46 * cos(2*TML_PI*i/N);
		vec[i-(N-1)/2]*=w[i];
	}
}

TMLIMP(void, tml_sr_UpSample,(const float *src, int srcLen, float *dst,
int *dstLen, int factor, int *phase))
{
	int k;
	*dstLen = srcLen * factor;
	tml_sr_vZero(dst, *dstLen); 
	for (k = 0; k < srcLen; k++)
		dst[k * factor + *phase] = src[k];
}

TMLIMP(void, tml_sc_UpSample,(const SCplx *src, int srcLen, SCplx *dst, int
*dstLen, int factor, int *phase))
{
	int k;
	*dstLen = srcLen * factor;
	tml_sc_vZero(dst, *dstLen); 
	for (k = 0; k < srcLen; k++)
	{
		dst[k * factor + *phase].re = src[k].re;
		dst[k * factor + *phase].im = src[k].im; 
	}
}

TMLIMP(void, tml_sr_DownSample, (const float *src, int srcLen, float *dst,
int *dstLen, int factor, int *phase))
{
	int k;
	*dstLen = (int) (srcLen / factor);
	for (k = 0; k < *dstLen; k++)
		dst[k] = src[k * factor + *phase];
	*phase += *dstLen * factor - srcLen;
}

TMLIMP(void, tml_sc_DownSample, (const SCplx *src, int srcLen, SCplx *dst,
int *dstLen, int factor, int *phase))
{
	int k;
	*dstLen = (int) (srcLen / factor);
	for (k = 0; k < *dstLen; k++)
	{	
		dst[k].re = src[k * factor + *phase].re;
		dst[k].im = src[k * factor + *phase].im;
	}
	*phase += *dstLen * factor - srcLen;
}

/*-----UpSampling with filtering (use only in Resampler component)-------*/

/*
TMLIMP(int, tml_sr_SampInit, (TMLSampState* sampSt, float* factorRange,
float* freq, int nFactors, int nTaps))
{
	sampSt->factorRange
	sampSt->nFactors
	sampSt->freq
	sampSt->nTaps

//	sampSt->dLen
//	sampSt->firState
//	sampSt->isSampInit
//	sampSt->sLen
//	sampSt->taps

}

TMLIMP(int, tml_sr_Samp, (TMLSampState* sampSt,const float* src,
int srcLen, float* dst, int dstLen))
{

}

TMLIMP(void, tml_SampFree, (TMLSampState* sampSt))
{

}
*/
