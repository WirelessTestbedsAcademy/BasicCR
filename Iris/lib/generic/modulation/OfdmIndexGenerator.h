/**
 * \file lib/generic/modulation/OfdmIndexGenerator.h
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
 * An OFDM preamble generator. Objects of this class can be used to
 * generate preambles with half-symbol repetitions.
 */

#ifndef MOD_OFDMINDEXGENERATOR_H_
#define MOD_OFDMINDEXGENERATOR_H_

#include <complex>
#include <vector>
#include <deque>
#include <algorithm>
#include <boost/math/special_functions/round.hpp>

#include "irisapi/Exceptions.h"
#include "irisapi/TypeInfo.h"
#include "irisapi/Logging.h"

namespace iris
{

namespace bm = boost::math;

/// Generate pilot and data carrier indices for OFDM symbols.
class OfdmIndexGenerator
{
 public:
  typedef std::vector<int>     IntVec;
  typedef IntVec::iterator     IntVecIt;

  /** Generate subcarrier indices for pilot and data carriers in an OFDM symbol.
   *
   * This algorithm attempts to place pilots in an OFDM symbol such that
   * they're evenly spaced across the carriers. Index 0 is the DC carrier
   * and it never carries data.
   *
   * @param numData   Number of data carriers (not including pilots).
   * @param numPilot  Number of pilot carriers.
   * @param numGuard  Number of guard carriers (not including DC carrier).
   * @param outBegin  Iterator to first element of output pilot carrier indices.
   * @param outEnd    Iterator to one past last element of output pilot indices.
   * @param dataBegin Iterator to first element of output data carrier indices.
   * @param dataEnd   Iterator to one past last element of output data indices.
   */
  static void generateIndices(int numData,
                              int numPilot,
                              int numGuard,
                              IntVecIt pilotsBegin,
                              IntVecIt pilotsEnd,
                              IntVecIt dataBegin,
                              IntVecIt dataEnd)
  {
    if(pilotsEnd-pilotsBegin < numPilot)
      throw IrisException("Insufficient storage provided for generateIndices pilot output.");
    if(dataEnd-dataBegin < numData)
      throw IrisException("Insufficient storage provided for generateIndices data output.");

    int numBins = numData+numPilot+numGuard+1;
    int numActive = numData+numPilot;
    IntVec scratch,scratch2;

    if(numPilot > 0)
    {
      float space = numActive/(float)numPilot;
      for(int i=1;i<=numPilot;i++)
        scratch.push_back((int)bm::round(space*i)-(int)bm::round(space/2)+1);

      for(int i=numPilot/2;i<numPilot;i++)
        scratch2.push_back(scratch[i]-numActive/2);
      for(int i=0;i<numPilot/2;i++)
        scratch2.push_back(scratch[i]+numBins-1-numActive/2);

      IntVecIt it = scratch2.begin();
      if(*it==0)
      {
        scratch2.erase(it);
        scratch2.push_back(numBins-1);
      }

      std::copy(scratch2.begin(),scratch2.end(),pilotsBegin);
    }

    for(int i=1;i<=numActive/2;i++)
      if(find(scratch2.begin(),scratch2.end(),i)==scratch2.end())
        *dataBegin++ = i;
    for(int i=numBins-(int)ceil(numActive/2.0);i<numBins;i++)
      if(find(scratch2.begin(),scratch2.end(),i)==scratch2.end())
        *dataBegin++ = i;
  }

  /// Convenience function for logging.
  static std::string getName(){ return "OfdmIndexGenerator"; }

 private:

  OfdmIndexGenerator(); // Disabled constructor
};

} // namespace iris

#endif // MOD_OFDMINDEXGENERATOR_H_
