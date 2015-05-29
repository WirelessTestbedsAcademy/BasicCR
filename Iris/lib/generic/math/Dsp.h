/**
 * \file Dsp.h
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
 * Some useful DSP routines.
 */

#ifndef DSP_H_
#define DSP_H_

#include <cmath>
#include <complex>

namespace iris
{

/** Quick routine to estimate the magnitude of a complex number.
 * Adapted from Grant Griffin's DSP trick
 * (http://www.dspguru.com/dsp/tricks/magnitude-estimator)
 * Default alpha/beta values are for min RMS error.
 *
 * @param c       The complex number.
 * @param alpha   First multiplier
 * @param beta    Second multiplier
 */

template<class T>
T fastMag(std::complex<T> c,
          double alpha = 0.947543636291,
          double beta = 0.392485425092)
{
  // magnitude ~= alpha * max(|I|, |Q|) + beta * min(|I|, |Q|)
  T absI = fabs(c.real());
  T absQ = fabs(c.imag());
  if (absI > absQ) {
    return alpha * absI + beta * absQ;
  }
  return alpha * absQ + beta * absI;
}

} // namespace iris

#endif // DSP_H_
