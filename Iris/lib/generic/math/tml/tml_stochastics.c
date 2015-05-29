/**
 * \file tml_stochastics.c
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
 * Implementation of the functions for random numbers and stochastic
 * computations.
 */

#define TML_W32DLL

#include <math.h>
#include <stdlib.h>
#include "tml.h"

/* this function is taken from http://c-faq.com/lib/gaussian.html */
/* FIXXX ME: probably a better version around - but it works well */
      double gaussrand()
      {
        static double V1, V2, S;
        static int phase = 0;
        double X;

        if(phase == 0) {
          do {
            double U1 = (double)rand() / RAND_MAX;
            double U2 = (double)rand() / RAND_MAX;

            V1 = 2 * U1 - 1;
            V2 = 2 * U2 - 1;
            S = V1 * V1 + V2 * V2;
            } while(S >= 1 || S == 0);

          X = V1 * sqrt(-2 * log(S) / S);
        } else
          X = V2 * sqrt(-2 * log(S) / S);

        phase = 1 - phase;

        return X;
      }
      
      /** generates a standard normally distributed random number (double) */
      #define RNOR (gaussrand())

/* ------------------------------------------------------------ */


TMLIMP(void, tml_sr_RandGausInit, (unsigned long seed, float mean, float stdDev,
    TML_sr_RandGausState *statePtr))
{
  statePtr->mean = mean;
  statePtr->stDev = stdDev;
  srand(seed); 
}
    
    
TMLIMP(void, tml_sc_RandGausInit, (unsigned long seed, float mean, float stdDev,
    TML_sc_RandGausState *statePtr))
{
  statePtr->mean = mean;
  statePtr->stDev = stdDev;
  srand(seed);  
}

TMLIMP(void, tml_dr_RandGausInit, (unsigned long seed, double mean, double stdDev,
    TML_dr_RandGausState *statePtr))
{
  statePtr->mean = mean;
  statePtr->stDev = stdDev;
  srand(seed);  
}

TMLIMP(void, tml_dc_RandGausInit, (unsigned long seed, double mean, double stdDev,
    TML_dc_RandGausState *statePtr))
{
  statePtr->mean = mean;
  statePtr->stDev = stdDev;
  srand(seed);  
}

TMLIMP(float, tml_sr_RandGaus, (TML_sr_RandGausState *statePtr))
{
  return ((float)RNOR) * statePtr->stDev + statePtr->mean;
}

TMLIMP(SCplx, tml_sc_RandGaus, (TML_sc_RandGausState *statePtr))
{
  SCplx tmp;
  tmp.re = ((float)RNOR * statePtr->stDev) + statePtr->mean;
  tmp.im = ((float)RNOR * statePtr->stDev) + statePtr->mean;
  return tmp;
}

TMLIMP(double, tml_dr_RandGaus, (TML_dr_RandGausState *statePtr))
{
  return ((double)RNOR) * statePtr->stDev + statePtr->mean;
}

TMLIMP(DCplx, tml_dc_RandGaus, (TML_dc_RandGausState *statePtr))
{

  DCplx tmp;
  tmp.re = ((double)RNOR * statePtr->stDev) + statePtr->mean;
  tmp.im = ((double)RNOR * statePtr->stDev) + statePtr->mean;
  return tmp;
}

TMLIMP(void, tml_sr_vRandGaus, (TML_sr_RandGausState *statePtr, float *samps, int sampsLen))
{
  int i;
  for (i = 0; i < sampsLen; i++)
    samps[i] = ((float)RNOR) * statePtr->stDev + statePtr->mean; 
}

TMLIMP(void, tml_sc_vRandGaus, (TML_sc_RandGausState *statePtr, SCplx *samps, int sampsLen))
{
  int i;
  for (i = 0; i < sampsLen; i++)
  {
    samps[i].re = ((float)RNOR) * statePtr->stDev + statePtr->mean;
    samps[i].im = ((float)RNOR) * statePtr->stDev + statePtr->mean;
  } 
}

TMLIMP(void, tml_dr_vRandGaus, (TML_dr_RandGausState *statePtr, double *samps, int sampsLen))
{
  int i;
  for (i = 0; i < sampsLen; i++)
    samps[i] = ((double)RNOR) * statePtr->stDev + statePtr->mean; 
}

TMLIMP(void, tml_dc_vRandGaus, (TML_dc_RandGausState *statePtr, DCplx *samps, int sampsLen))
{
  int i;
  for (i = 0; i < sampsLen; i++)
  {
    samps[i].re = ((double)RNOR) * statePtr->stDev + statePtr->mean;
    samps[i].im = ((double)RNOR) * statePtr->stDev + statePtr->mean;
  } 
}
