/**
 * \file tml_stochastics.h
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
 * The functions for random numbers and stochastic computations.
 */

#ifndef TML_STOCHASTICS_H_
#define TML_STOCHASTICS_H_

#include "tml.h"



   typedef struct TML_sr_RandGausState {
    float    mean,stDev;      /* Mean and standard deviation values */
    /*float    extraValue;  */      /* Value computed early         */
    /*int      useExtra;  */      /* Flag to use extra value      */
    /*unsigned int seed2,carry; */      /* Variables of the algorithm     */
    /*unsigned int seed1[3];  */      /* Variables of the algorithm     */
    /*int      quickSeed;*/
   } TML_sr_RandGausState;

   typedef struct TML_dr_RandGausState {
    double     mean,stDev;
    /*double     extraValue;
    int      useExtra;
    unsigned int seed2,carry;
    unsigned int seed1[3];
    int      quickSeed;*/
   } TML_dr_RandGausState;

/*   typedef struct TMLRandGausState {
    int      useExtra;
    unsigned int seed2,carry;
    unsigned int seed1[3];
    short    mean,stDev;
    short    extraValue;
    int      quickSeed;
   } NSPWRandGausState;
*/
   typedef struct TML_sr_RandGausState TML_sc_RandGausState;
   typedef struct TML_dr_RandGausState TML_dc_RandGausState;
 /*  typedef struct NSPWRandGausState NSPVRandGausState;*/

   

/** Initializes the random number generator for gaussian distributed 
 * random numbers.
 * \param seed A seed for the pseudo random numbers, can be any number.
 *       The same number will produce the some series of values.
 * \param mean The mean of the distribution.
 * \param stdDev The standard deviation of the distribution.
 * \param statePtr Pointer to the internal state which should be used
 * with any subsequent call to generate random numbers.
 */ 
TMLAPI(void, tml_sr_RandGausInit, (unsigned long seed, float mean, float stdDev,
  TML_sr_RandGausState *statePtr))
TMLAPI(void, tml_sc_RandGausInit, (unsigned long seed, float mean, float stdDev,
  TML_sc_RandGausState *statePtr))
TMLAPI(void, tml_dr_RandGausInit, (unsigned long seed, double mean, double stdDev,
  TML_dr_RandGausState *statePtr))
TMLAPI(void, tml_dc_RandGausInit, (unsigned long seed, double mean, double stdDev,
  TML_dc_RandGausState *statePtr))

/** Generates Gaussian distributed random numbers.
 * \param statePtr Pointer to the state previously initialized with the appropriate
 * RandGausInit function.
 */
TMLAPI(float, tml_sr_RandGaus, (TML_sr_RandGausState *statePtr))
TMLAPI(SCplx, tml_sc_RandGaus, (TML_sc_RandGausState *statePtr))
TMLAPI(double, tml_dr_RandGaus, (TML_dr_RandGausState *statePtr))
TMLAPI(DCplx, tml_dc_RandGaus, (TML_dc_RandGausState *statePtr))

/** Generates Gaussian distributed vectors (all elements are drawn from 
 * a Gaussian distribution).
 * \param statePtr Pointer to the state previously initialized with the appropriate
 * RandGausInit function.
 * \param samps Pointer to the vector where the data should be stored.
 * \param sampsLen Length of the output vector.
 */
TMLAPI(void, tml_sr_vRandGaus, (TML_sr_RandGausState *statePtr, float *samps, int sampsLen))
TMLAPI(void, tml_sc_vRandGaus, (TML_sc_RandGausState *statePtr, SCplx *samps, int sampsLen))
TMLAPI(void, tml_dr_vRandGaus, (TML_dr_RandGausState *statePtr, double *samps, int sampsLen))
TMLAPI(void, tml_dc_vRandGaus, (TML_dc_RandGausState *statePtr, DCplx *samps, int sampsLen))

#endif /*TML_STOCHASTICS_H_*/
