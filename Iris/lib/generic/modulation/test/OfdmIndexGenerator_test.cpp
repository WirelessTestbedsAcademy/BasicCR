/**
 * \file lib/generic/modulation/OfdmIndexGenerator_test.cpp
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
 * Main test file for OfdmIndexGenerator class.
 */

#define BOOST_TEST_MODULE OfdmIndexGenerator_Test

#include <boost/test/unit_test.hpp>
#include <vector>
#include <complex>

#include "OfdmIndexGenerator.h"

#include "irisapi/TypeInfo.h"
#include "utility/RawFileUtility.h"

using namespace iris;

template <typename T, size_t N>
static T* begin(T(&arr)[N]) { return &arr[0]; }
template <typename T, size_t N>
static T* end(T(&arr)[N]) { return &arr[0]+N; }

BOOST_AUTO_TEST_SUITE (OfdmIndexGenerator_Test)

BOOST_AUTO_TEST_CASE(OfdmIndexGenerator_Default_Test)
{
  int numData=192;
  int numPilot=8;
  int numGuard=55;
  std::vector< int > pIndices(numPilot);
  std::vector< int > dIndices(numData);

  OfdmIndexGenerator::generateIndices(numData,numPilot,numGuard,
                                      pIndices.begin(),pIndices.end(),
                                      dIndices.begin(),dIndices.end());

  int p[] = {13,38,63,88,168,193,218,243};
  std::vector< int > pilots(begin(p),end(p));
  BOOST_CHECK(pilots==pIndices);

  std::vector< int >::iterator it;
  for(it=pIndices.begin();it!=pIndices.end();it++)
    BOOST_CHECK(find(dIndices.begin(),dIndices.end(),*it)==dIndices.end());

  BOOST_CHECK(find(dIndices.begin(), dIndices.end(),0)==dIndices.end());
  BOOST_CHECK(find(pIndices.begin(), pIndices.end(),0)==pIndices.end());
}

BOOST_AUTO_TEST_CASE(OfdmIndexGenerator_OddBins_Test)
{
  int numData=7;
  int numPilot=3;
  int numGuard=0;
  std::vector< int > pIndices(numPilot);
  std::vector< int > dIndices(numData);

  OfdmIndexGenerator::generateIndices(numData,numPilot,numGuard,
                                      pIndices.begin(),pIndices.end(),
                                      dIndices.begin(),dIndices.end());

  int p[] = {1,4,7};
  std::vector< int > pilots(begin(p),end(p));
  BOOST_CHECK(pilots==pIndices);

  std::vector< int >::iterator it;
  for(it=pIndices.begin();it!=pIndices.end();it++)
    BOOST_CHECK(find(dIndices.begin(),dIndices.end(),*it)==dIndices.end());

  BOOST_CHECK(find(dIndices.begin(), dIndices.end(),0)==dIndices.end());
  BOOST_CHECK(find(pIndices.begin(), pIndices.end(),0)==pIndices.end());
}

BOOST_AUTO_TEST_CASE(OfdmIndexGenerator_OddActive_Test)
{
  int numData=6;
  int numPilot=3;
  int numGuard=0;
  std::vector< int > pIndices(numPilot);
  std::vector< int > dIndices(numData);

  OfdmIndexGenerator::generateIndices(numData,numPilot,numGuard,
                                      pIndices.begin(),pIndices.end(),
                                      dIndices.begin(),dIndices.end());

  int p[] = {1,4,7};
  std::vector< int > pilots(begin(p),end(p));
  BOOST_CHECK(pilots==pIndices);

  std::vector< int >::iterator it;
  for(it=pIndices.begin();it!=pIndices.end();it++)
    BOOST_CHECK(find(dIndices.begin(),dIndices.end(),*it)==dIndices.end());

  BOOST_CHECK(find(dIndices.begin(), dIndices.end(),0)==dIndices.end());
  BOOST_CHECK(find(pIndices.begin(), pIndices.end(),0)==pIndices.end());
}

BOOST_AUTO_TEST_CASE(OfdmIndexGenerator_AllPilot_Test)
{
  int numData=0;
  int numPilot=10;
  int numGuard=0;
  std::vector< int > pIndices(numPilot);
  std::vector< int > dIndices(numData);

  OfdmIndexGenerator::generateIndices(numData,numPilot,numGuard,
                                      pIndices.begin(),pIndices.end(),
                                      dIndices.begin(),dIndices.end());

  int p[] = {1,2,3,4,5,6,7,8,9,10};
  std::vector< int > pilots(begin(p),end(p));
  BOOST_CHECK(pilots==pIndices);

  BOOST_CHECK(dIndices.empty());
}

BOOST_AUTO_TEST_CASE(OfdmIndexGenerator_AllGuard_Test)
{
  int numData=0;
  int numPilot=0;
  int numGuard=11;
  std::vector< int > pIndices(numPilot);
  std::vector< int > dIndices(numData);

  OfdmIndexGenerator::generateIndices(numData,numPilot,numGuard,
                                      pIndices.begin(),pIndices.end(),
                                      dIndices.begin(),dIndices.end());

  BOOST_CHECK(pIndices.empty());
  BOOST_CHECK(dIndices.empty());
}

BOOST_AUTO_TEST_SUITE_END()
