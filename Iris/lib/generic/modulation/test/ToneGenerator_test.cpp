/**
 * \file lib/generic/modulation/ToneGenerator_test.cpp
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
 * Main test file for ToneGenerator class.
 */

#define BOOST_TEST_MODULE ToneGenerator_Test

#include <boost/test/unit_test.hpp>
#include <vector>
#include <complex>

#include "ToneGenerator.h"

#include "irisapi/TypeInfo.h"
#include "utility/RawFileUtility.h"
#include "math/kissfft/kissfft.hh"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (ToneGenerator_Test)

BOOST_AUTO_TEST_CASE(ToneGenerator_Test1)
{
  typedef complex<float>    Cplx;
  typedef vector<Cplx>      CplxVec;

  CplxVec tone(64);
  ToneGenerator t;
  t.generate(tone.begin(), tone.end(), 1.0/64);

  CplxVec bins(64),fftTone(64);
  bins[1] = Cplx(1,0);
  kissfft<float> fft(64, true);
  fft.transform(&bins[0], &fftTone[0]);

  float errorTolerance = 1e-6;
  for(int i=0; i<64; i++)
  {
    BOOST_CHECK((tone[i].real() - fftTone[i].real()) < errorTolerance);
    BOOST_CHECK((tone[i].imag() - fftTone[i].imag()) < errorTolerance);
  }
}

BOOST_AUTO_TEST_CASE(ToneGenerator_Test2)
{
  typedef complex<float>    Cplx;
  typedef vector<Cplx>      CplxVec;

  CplxVec tone(64);
  ToneGenerator t;
  t.generate(tone.begin(), tone.end(), 3.0/64);

  CplxVec bins(64),fftTone(64);
  bins[3] = Cplx(1,0);
  kissfft<float> fft(64, true);
  fft.transform(&bins[0], &fftTone[0]);

  float errorTolerance = 1e-6;
  for(int i=0; i<64; i++)
  {
    BOOST_CHECK((tone[i].real() - fftTone[i].real()) < errorTolerance);
    BOOST_CHECK((tone[i].imag() - fftTone[i].imag()) < errorTolerance);
  }
}

BOOST_AUTO_TEST_SUITE_END()
