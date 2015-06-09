/**
 * \file tml_processing.h
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
 * The signal processing operations for the Trinity Math Library.
 */

#ifndef TMLPROCESSING_H_
#define TMLPROCESSING_H_

#include "tml.h"

/* --- FIR low level filter structure -------------------------------------*/

#define FirLmsTapState \
   void *taps; \
   int   len; \
   int   upFactor; \
   int   upPhase; \
   int   downFactor; \
   int   downPhase; \
   int   isMultiRate; \
   int   isFilterMode

#define FirLmsDlyState \
   void *dlyl; \
   int   len; \
   int   isFilterMode

		/* --- Finite impulse response filter -------------------------------------*/

			typedef struct _TMLFirTapState
				{
				FirLmsTapState;
				int tapsFactor;
				} TMLFirTapState;

		/* --- Delay line of FIR low level filter ---------------------------------*/

			typedef struct _TMLFirDlyState
				{
				FirLmsDlyState;
				} TMLFirDlyState;

	
	/*=== FIR high-level ======================================================*/

  typedef struct
    {
    int num;       /* section number          */
    int len;       /* section taps length in use    */
    int xId;       /* section input data index    */
    int tId;       /* section taps index        */
    } firSect;

  typedef struct _TMLFirState
    {
    int   upFactor;    /* up                 */
    int   upPhase;     /*  parameters            */
    int   downFactor;  /* down               */
    int   downPhase;   /*  parameters            */
    int   isMultiRate;   /* multi-rate mode flag       */
    void* isInit;    /* init flag            */

    int   tapsLen;     /* number of filter taps      */
    int   tapsBlk;
    void* taps;      /* taps pointer in use        */

    int   dlylLen;     /* delay line length        */
    int   inpLen;    /* input buffer length        */
    int   useInp;    /* input buffer length used     */
    void* dlyl;      /* extended dilter delay line     */
    void* dlylLimit; /* delay line buffer end pointer  */
    void* useDlyl; /* delay line pointer in use    */

    firSect* sect;     /* FIR sections           */

    int   tapsFactor;  /* taps scale factor        */
    int   utapsLen;    /* number of filter taps (user def) */
    } TMLFirState;

	typedef enum
    {
    TML_WinRect,
    TML_WinHamming,
    TML_WinHann
    } TML_WindowType;

	/*============================sampling======================*/
	  typedef struct {
    int  nFactors;
    int  nTaps;
    int  sLen;
    int  dLen;
    int  isSampInit;
    float *factorRange;
    float *freq;
    void  *taps;
    TMLFirState firState;
   } TMLSampState;


/*------------------------------ Signal Processing Function Implementation ------------------------ */
/** \defgroup Signal Processing Operations
 * \{
 */

/*----------Convolution----------*/

 /**Performs finite, linear convolution of
* two sequences.
* \param h, x Pointers to the arrays to be convolved.
* \param hLen Number of samples in the array h[n].
* \param xLen Number of samples in the array x[n].
* \param y Pointer to the array which stores the result of the
* convolution.
*/
TMLAPI(void, tml_sr_Conv, (const float *x, int xLen, const float *h, int hLen,
float *y))
TMLAPI(void, tml_sc_Conv, (const SCplx *x, int xLen, const SCplx *h, int hLen,
SCplx *y))

/*----------Low Level FIR filtering----------*/

/**Initialize the coefficients and structure
* of a single-rate Low Level FIR filter
* \param taps Pointer to the array which specifies the filter coefficients.
* \param tapsLen The number of taps in the taps[n] array.
* \param tapStPtr Pointer to the TMLFirTapState structure.
*/
TMLAPI(void, tml_sc_FirlInit,(SCplx *taps, int tapsLen, TMLFirTapState *tapStPtr))
TMLAPI(void, tml_sr_FirlInit,(float *taps, int tapsLen, TMLFirTapState *tapStPtr))

/**Initialize the structure of a delay line.
* The delay line is associated with a particular set of taps. Multiple delay
* lines for a given set of taps can be initialized by calling this function
* multiple times, but there should be only one call for each delay line.
* \param dlyl Pointer to the array which specifies the initial values for
* the delay line for the TML_?_FirlInitDlyl() function.
* \param dlylStPtr Pointer to the TMLFirDlylState structure.
* \param tapStPtr Pointer to the TMLFirTapState structure.
*/
TMLAPI(void, tml_sc_FirlInitDlyl,(const TMLFirTapState *tapStPtr, SCplx *dlyl, TMLFirDlyState *dlyStPtr))
TMLAPI(void, tml_sr_FirlInitDlyl,(const TMLFirTapState *tapStPtr, float *dlyl, TMLFirDlyState *dlyStPtr))

/**Sets the tap coefficients for a low-level FIR filter
* \param inDlyl Pointer to the array holding copies of the delay line
* values for the tml_?_FirlSetDlyl() function.
* \param tapStPtr Pointer to the TMLFirTapState structure.
* \param dlylStPtr Pointer to the TMLFirDlylState structure.
*/
TMLAPI(void, tml_sc_FirlSetDlyl, (const TMLFirTapState *tapStPtr, SCplx *inDlyl,
TMLFirDlyState *dlylStPtr))
TMLAPI(void, tml_sr_FirlSetDlyl, (const TMLFirTapState *tapStPtr, float *inDlyl,
TMLFirDlyState *dlylStPtr))

/**Sets the tap coefficients for a low-level FIR filter
* \param inTaps Pointer to the array holding copies of the tap
* coefficients.
* \param tapStPtr Pointer to the TMLFirTapState structure.
*/
TMLAPI(void, tml_sc_FirlSetTaps, (SCplx *inTaps, TMLFirTapState *tapStPtr))
TMLAPI(void, tml_sr_FirlSetTaps, (float *inTaps, TMLFirTapState *tapStPtr))


/** Filters one sample through a low-level FIR filter.
* \param tapStPtr Pointer to the TMLFirTapState structure.
* \param dlylStPtr Pointer to the TMLFirDlylState structure.
* \param samp Input sample to be filtered.
* \return filter output.
*/
TMLAPI(SCplx, tml_sc_Firl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const SCplx samp))
TMLAPI(float, tml_sr_Firl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const float samp))

/**Filters a block of samples through a low-level FIR filter
*complex input, complex taps; single precision
* \param tapStPtr Pointer to the TMLFirTapState structure.
* \param dlylStPtr Pointer to the TMLFirDlylState structure.
* \param inSamps Pointer to the array which stores the input samples to be
* filtered by the tml_?_vFirl() function.
* \param outSamps Pointer to the array which stores the output samples
* filtered by the tml_?_vFirl() function.
* \param numIters The number of samples to be filtered by
* the tml_?_vFirl() function.
*/
TMLAPI(void, tml_sc_vFirl, (const TMLFirTapState *tapStPtr,
TMLFirDlyState *dlyStPtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))

TMLAPI(void, tml_sr_vFirl, (const TMLFirTapState *tapStPtr,
TMLFirDlyState *dlyStPtr, const float *inSamps,
float *outSamps, int numIters))

TMLAPI(void, tml_scr_vFirl, (const TMLFirTapState *tapStPtr, TMLFirDlyState *dlyStPtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))

/*----------Normal FIR filtering----------*/

/**Initialize the coefficients, delay line,
* and structure of a single-rate FIR filter
* \param tapVals Pointer to the array containing the filter coefficient
* (taps) values.
* \param tapsLen The number of values in the array containing the filter
* coefficients (taps).
* \param dlyVals Pointer to the array containing the delay line values.
* \param statePtr Pointer to the TMLFirState structure.
*/
TMLAPI(void, tml_sr_FirInit, (const float *tapVals, int tapsLen,
const float *dlyVals, TMLFirState *statePtr))

/**Filters a single sample through an FIR filter
* \param statePtr Pointer to the TMLFirState structure.
* \param samp The input sample to be filtered by the tml_?_Fir()
* function.
*/
TMLAPI(float, tml_sr_Fir, (TMLFirState *statePtr, float samp))

/**Filters a block of samples through an FIR filter
* \param statePtr Pointer to the TMLFirState structure.
* \param inSamps Pointer to the array which stores the input samples to be
* filtered by the tml_?_vFir() function.
* \param outSamps Pointer to the array which stores the output samples
* filtered by the tml_?_vFir() function.
* \param numIters Parameter associated with the number of samples to be
* filtered by the tml_?_vFir() function. For single-rate
* filters, the numIters samples in the array inSamps[n]
* are filtered and the resulting numIters samples are
* stored in the array outSamps[n]. 
* complex delay line, real taps; single precision
*/
TMLAPI(void, tml_scr_vFir, (TMLFirState *statePtr, const SCplx *inSamps,
SCplx *outSamps, int numIters))

/**Filters a block of samples through an FIR filter
* \param statePtr Pointer to the TMLFirState structure.
* \param inSamps Pointer to the array which stores the input samples to be
* filtered by the tml_?_vFir() function.
* \param outSamps Pointer to the array which stores the output samples
* filtered by the tml_?_vFir() function.
* \param numIters Parameter associated with the number of samples to be
* filtered by the tml_?_vFir() function. For single-rate
* filters, the numIters samples in the array inSamps[n]
* are filtered and the resulting numIters samples are
* stored in the array outSamps[n]. 
* real delay line, real taps; single precision+
*/
TMLAPI(void, tml_sr_vFir,(TMLFirState *statePtr, const float *inSamps,
float *outSamps, int numIters) )


/** Computes taps for a bandpass FIR filter.
* taps are normalized
* \param rLowFreq Low cut-off frequency (0 < rLowFreq < rHighFreq).
* \param rHighFreq High cut-off frequency (rLowFreq < rHighFreq < 0.5)
* \param taps Pointer to the vector of taps to be computed.
* \param tapsLen The number of taps in taps (tapsLen greater than or equal to 5).
* \param winType Specifies the smoothing window type. Can be one of the following:
*	TML_WinRect no smoothing (rectangular window);
*	TML_WinHamming smoothing by Hamming window;
* \Return one of the following values:
*	TML_StsOk if the taps have been computed successfully
*	TML_fStsBadPointer if the pointer to the taps vector is null
*	TML_fStsBadLen if the number of taps (tapsLen) is less than five
*	TML_fStsBadFreq if the frequencies are not in the interval (0, 0.5)
*	TML_fStsBadRel ifrLowFreq greater than or equal rHighFreq .
* \todo define error handling TML_StsOk ...
*/
TMLAPI(int, tml_dr_FirBandpass, (double rLowFreq, double rHighFreq,
double *taps, int tapsLen, TML_WindowType winType))

/**Computes taps for a lowpass FIR filter.
* taps are normalized
* \param rfreq Cut-off frequency value (0 < rfreq < 0.5).
* \param taps Pointer to the vector of taps to be computed.
* \param tapsLen The number of taps in taps (tapsLen greater than or equal to 5).
* \param winType Specifies the smoothing window type. Can be one of the following:
* 	TML_WinRect no smoothing (rectangular window);
* 	TML_WinHamming smoothing by Hamming window;
* \Return one of the following values:
* 	TML_StsOk if the taps have been computed successfully
*	TML_fStsBadPointer if the pointer to the taps vector is null
*	TML_fStsBadLen if the number of taps (tapsLen) is less than five
*	TML_fStsBadFreq if the frequencies are not in the interval (0, 0.5)
*/
TMLAPI(int, tml_dr_FirLowpass, (double rfreq, double *taps, int tapsLen,
TML_WindowType winType))

/**Frees dynamic memory associated
* with the FIR filter.
* \param statePtr Pointer to the TMLFirState structure.
*/
TMLAPI(void, tml_FirFree, (TMLFirState *statePtr))

/**Multiplies a vector by a Hann windowing function.
* \param vec Pointer to the vector to be multiplied by the chosen
* windowing function.
* \param N The number of samples in the input and output vectors.
*/
TMLAPI(void, tml_sr_WinHann, (float *vec, int N))

/**Multiplies a vector by a Hamming windowing function.
* \param vec Pointer to the vector to be multiplied by the chosen
* windowing function.
* \param N The number of samples in the input and output vectors.
*/
TMLAPI(void, tml_dr_WinHamming, (double *vec, int N))

/*---------------------------------------------------------------------------------------*/
/*------------------------------------------Sampling-------------------------------------*/
/*---------------------------------------------------------------------------------------*/



/*============================ Resampling with filtering =================================*/
 
/**Initializes resampling parameters structure.
* \param sampSt The pointer to a TMLSampState structure which will
* contain the resampling parameters data.
* \param factorRange The vector of predefined resampling factor quotients.
* Each value is equal to the quotient of the input vector length divided 
* by the output vector length.
* \param freq The vector of cut-off frequencies for the lowpass filters
* \param nFactors The length of frequency and resampling factor vectors.
* \param nTaps The number of filter taps.
* \todo define error handling (to return TMLStatus instead of int)
*/
TMLAPI(int, tml_sr_SampInit, (TMLSampState* sampSt, float* factorRange,
float* freq, int nFactors, int nTaps))

/**Performs resampling of the input signal using the multi-rate FIR filter.
* \param sampSt The pointer to TMLSampState structure which contains the
* resampling parameters.
* \param src The input signal vector.
* \param srcLen Length of the input vector.
* \param dst The output signal vector.
* \param dstLen Length of the output vector.
*/
TMLAPI(int, tml_sr_Samp, (TMLSampState* sampSt,const float* src,
int srcLen, float* dst, int dstLen))

/**Frees work array memory which is pointed to
* in the resampling data structure sampSt.
* \param sampSt The pointer to a TMLSampState structure which contains the
* resampling parameters.
*/
TMLAPI(void, tml_SampFree, (TMLSampState* sampSt))

/*====================Up/Down Sampling ====================================*/

/** Up-samples a signal, conceptually increasing its sampling rate by an
* integer factor.
* \param dst Pointer to the output array.
* \param dstLen An output parameter: the number of samples in the dst
* array. It is equal to the product (srcLen * factor).
* \param factor The factor by which the signal is up-sampled. That is,
* factor -1 zeros are inserted after each sample of
* src[n].
* \param phase A parameter which determines where each sample from
* src[n] lies within each output block of factor
* samples. The value of phase is required to be 0 =< phase < factor.
* The value of this parameter can be used for the next up-sampling 
* with the same factor and next src[n].
* \param src Pointer to the input array (the signal to be up-sampled).
* \param srcLen The number of samples in the src array.
*/
TMLAPI(void, tml_sr_UpSample,(const float *src, int srcLen, float *dst,
int *dstLen, int factor, int *phase))

TMLAPI(void, tml_sc_UpSample,(const SCplx *src, int srcLen, SCplx *dst, int
*dstLen, int factor, int *phase))

/**Down-samples a signal, conceptually decreasing its sampling rate by an
* integer factor.
* \param src Pointer to the input array holding the signal samples to
* be down-sampled.
* \param srcLen The number of samples in the input array src[n].
* \param dst Pointer to the array that holds the output of the
* tml_?_DownSample() function.
* \param dstLen The number of samples in the dst array.
* \param factor The factor by which the signal is down-sampled. That is,
* factor - 1 samples are discarded from every block of
* factor samples in src[n].
* \param phase The input value of phase determines which of the
* samples within each block is not discarded. It is required
* to satisfy the condition 0 =< phase < factor.
* The function adjusts the output phase if srcLen is not
* a multiple of factor. The output value of phase is the
* input phase + dstLen*factor - srcLen.
* This is to allow for the continuous down-sampling of
* several arrays.
*/
TMLAPI(void, tml_sr_DownSample, (const float *src, int srcLen, float *dst,
int *dstLen, int factor, int *phase))

TMLAPI(void, tml_sc_DownSample, (const SCplx *src, int srcLen, SCplx *dst,
int *dstLen, int factor, int *phase))



/** \} */

#endif

