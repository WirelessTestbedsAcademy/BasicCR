/**
 * \file lib/generic/modulation/ToneGenerator.h
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
 * A complex tone generator.
 */

#ifndef MOD_TONEGENERATOR_H_
#define MOD_TONEGENERATOR_H_

#include <complex>
#include <vector>

#include "irisapi/TypeInfo.h"
#include "irisapi/Logging.h"
#include "math/MathDefines.h"

namespace iris
{

/// Generate tones with a given frequency
class ToneGenerator
{
 public:
  typedef std::vector<float>      FloatVec;
  typedef std::complex<float>     Cplx;

  ToneGenerator()
  {
    double angle;
    for(int i=0; i<65536; i++)
    {
      angle = (2.0 * IRIS_PI * (double)i) / (double)65536.0;
      lookup_.push_back((float)cos(angle));
    }
  }

  /** Generate a complex tone with a given frequency
   *
   * @param outBegin    Iterator to first element in output container.
   * @param outEnd      Iterator to one past last element in output.
   * @param frequency   Required tone frequency.
   */
  template <class Iterator>
  void generate(Iterator outBegin, Iterator outEnd, float frequency)
  {
    int neg = 1;
    frequency >= 0 ? neg = 1 : neg = -1;
    uint16_t delta = (uint16_t)abs(65536.0 * frequency);
    uint16_t deltaAcc = 0;
    uint16_t deltaAccSin = (65536/4 * 3);

    for(; outBegin!=outEnd; outBegin++)
    {
      *outBegin = Cplx(lookup_[deltaAcc], neg*lookup_[deltaAccSin]);
      deltaAcc += delta;
      deltaAccSin += delta;
    }
  }

  /// Convenience function for logging.
  std::string getName(){ return "ToneGenerator"; }

 private:
  FloatVec lookup_;  ///< An oversampled cosine
};

} // namespace iris

#endif // MOD_TONEGENERATOR_H_
