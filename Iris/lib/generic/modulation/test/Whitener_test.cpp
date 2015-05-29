/**
 * \file lib/generic/modulation/Whitener_test.cpp
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
 * Main test file for Whitener class.
 */

#define BOOST_TEST_MODULE Whitener_Test

#include "Whitener.h"

#include <boost/test/unit_test.hpp>

#include "irisapi/TypeInfo.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (Whitener_Test)

BOOST_AUTO_TEST_CASE(Whitener_Basic_Test)
{
  vector< uint8_t > data(4096);

  Whitener::whiten(data.begin(), data.end());

  for(int i=0; i<data.size(); i++)
    BOOST_CHECK(data[i] == whitenerdetail::whitenCode[i]);
}

BOOST_AUTO_TEST_CASE(Whitener_Test)
{
  vector< uint8_t > data(6144);
  for(int i=0; i<data.size(); ++i)
    data[i] = i%256;

  Whitener::whiten(data.begin(), data.end());
  Whitener::whiten(data.begin(), data.end());

  for(int i=0; i<data.size(); ++i)
    BOOST_CHECK(data[i] == i%256);
}

BOOST_AUTO_TEST_SUITE_END()
