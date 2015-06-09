/**
 * \file lib/generic/modulation/OfdmPreambleGenerator_test.cpp
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
 * Main test file for OfdmPreambleGenerator class.
 */

#define BOOST_TEST_MODULE OfdmPreambleGenerator_Test

#include <boost/test/unit_test.hpp>
#include <vector>
#include <complex>

#include "OfdmPreambleGenerator.h"

#include "irisapi/TypeInfo.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (OfdmPreambleGenerator_Test)

BOOST_AUTO_TEST_CASE(OfdmPreambleGenerator_Basic_Test)
{
  int numData=210;
  int numPilot=10;
  int numGuard=35;
  std::vector< std::complex<float> > preamble(256);

  OfdmPreambleGenerator g;
  g.generatePreamble(numData,numPilot,numGuard,
                     preamble.begin(),preamble.end());

  for(int i=0;i<128;i++)
    BOOST_CHECK(preamble[i] == preamble[i+128]);
}

BOOST_AUTO_TEST_CASE(OfdmPreambleGenerator_Long_Test)
{
  int numData=380;
  int numPilot=20;
  int numGuard=111;
  std::vector< std::complex<float> > preamble(512);

  OfdmPreambleGenerator g;
  g.generatePreamble(numData,numPilot,numGuard,
                     preamble.begin(),preamble.end());

  for(int i=0;i<256;i++)
    BOOST_CHECK(preamble[i] == preamble[i+256]);
}

BOOST_AUTO_TEST_SUITE_END()
