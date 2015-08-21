/**
 * \file lib/generic/modulation/OfdmPreambleDetector_test.cpp
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
 * Main test file for OfdmPreambleDetector class.
 */

#define BOOST_TEST_MODULE OfdmPreambleDetector_Test

#include <boost/test/unit_test.hpp>
#include <vector>
#include <complex>

#include "OfdmPreambleDetector.h"
#include "OfdmPreambleDetectorTestData.h"

#include "irisapi/TypeInfo.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (OfdmPreambleDetector_Test)

BOOST_AUTO_TEST_CASE(OfdmPreambleDetector_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(OfdmPreambleDetector detector;)
}

/** Test using a clean signal with preamble
 *
 * Signal has following structure:
 * | 16 |  256   | 16 |    256   | 16 |  256   |
 * | CP | Symbol | CP | Preamble | CP | Symbol |
 */
BOOST_AUTO_TEST_CASE(OfdmPreambleDetector_Detect1_Test)
{
  typedef OfdmPreambleDetectorTestData  D;
  typedef std::complex<float>           Cplx;
  typedef std::vector<Cplx>             CplxVec;
  typedef CplxVec::iterator             CplxVecIt;

  CplxVec preamble(272);
  bool detected = false;
  float freqOffset;
  float snr;

  OfdmPreambleDetector detector;
  CplxVecIt it;
  BOOST_REQUIRE_NO_THROW(
    it = detector.search(D::preambleTestData1_.begin(),
                    D::preambleTestData1_.end(),
                    preamble.begin(),
                    preamble.end(),
                    detected,
                    freqOffset,
                    snr);
  )
  int index = it - D::preambleTestData1_.begin();
  BOOST_CHECK( detected );
  BOOST_CHECK( abs(preamble.front()) > 0);
  BOOST_CHECK( index == 545);
  BOOST_CHECK( snr > 25 );
  BOOST_CHECK( freqOffset < 0.001 );

  // Run preamble through detector again to verify
  CplxVec scratch(272);
  detected = false;
  it = detector.search(preamble.begin(),
                       preamble.end(),
                       scratch.begin(),
                       scratch.end(),
                       detected,
                       freqOffset,
                       snr);
  BOOST_CHECK( detected );
  BOOST_CHECK( it == preamble.end() );
}

/** Test using a noisy signal with preamble and frequency offset
 *
 * Signal has following structure:
 *         ---------------------------------------------
 * Size:   | 16 |  256   | 16 |    256   | 16 |  256   |
 * Data:   | CP | Symbol | CP | Preamble | CP | Symbol |
 *         ---------------------------------------------
 *
 * SNR is ~15dB and frequency offset is 1/4 subcarrier spacing.
 */
BOOST_AUTO_TEST_CASE(OfdmPreambleDetector_Detect2_Test)
{
  typedef OfdmPreambleDetectorTestData  D;
  typedef std::complex<float>           Cplx;
  typedef std::vector<Cplx>             CplxVec;
  typedef CplxVec::iterator             CplxVecIt;

  CplxVec preamble(272);
  bool detected = false;
  float freqOffset;
  float snr;

  OfdmPreambleDetector detector;
  CplxVecIt it;
  BOOST_REQUIRE_NO_THROW(
    it = detector.search(D::preambleTestData2_.begin(),
                    D::preambleTestData2_.end(),
                    preamble.begin(),
                    preamble.end(),
                    detected,
                    freqOffset,
                    snr);
  )
  int index = it - D::preambleTestData2_.begin();
  BOOST_CHECK( detected );
  BOOST_CHECK( abs(preamble.front()) > 0);
  BOOST_CHECK( index == 545);
  BOOST_CHECK( snr > 13 );
  BOOST_CHECK( snr < 17 );
  BOOST_CHECK( freqOffset > 0.225 );
  BOOST_CHECK( freqOffset < 0.275 );

  // Run preamble through detector again to verify
  CplxVec scratch(272);
  detected = false;
  it = detector.search(preamble.begin(),
                       preamble.end(),
                       scratch.begin(),
                       scratch.end(),
                       detected,
                       freqOffset,
                       snr);
  BOOST_CHECK( detected );
  BOOST_CHECK( it == preamble.end());
}

/** Test using a noisy signal without preamble
 *
 * Signal has following structure:
 *         -----------------------------
 * Size:   | 16 |  256   | 16 |  256   |
 * Data:   | CP | Symbol | CP | Symbol |
 *         -----------------------------
 *
 * SNR is ~15dB and frequency offset is 1/4 subcarrier spacing.
 */
BOOST_AUTO_TEST_CASE(OfdmPreambleDetector_Detect3_Test)
{
  typedef OfdmPreambleDetectorTestData  D;
  typedef std::complex<float>           Cplx;
  typedef std::vector<Cplx>             CplxVec;
  typedef CplxVec::iterator             CplxVecIt;

  CplxVec preamble(272);
  bool detected = false;
  float freqOffset;
  float snr;

  OfdmPreambleDetector detector;
  CplxVecIt it;
  BOOST_REQUIRE_NO_THROW(
    it = detector.search(D::preambleTestData3_.begin(),
                    D::preambleTestData3_.end(),
                    preamble.begin(),
                    preamble.end(),
                    detected,
                    freqOffset,
                    snr);
  )
  BOOST_CHECK( !detected );
  BOOST_CHECK( abs(preamble.front()) == 0);
  BOOST_CHECK( it == D::preambleTestData3_.end());
}

BOOST_AUTO_TEST_SUITE_END()
