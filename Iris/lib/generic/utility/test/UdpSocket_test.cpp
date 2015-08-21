/**
 * \file lib/utility/UdpSocket_test.cpp
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
 * Main test file for UdpSocketReceiver and UdpSocketTransmitter classes.
 */

#define BOOST_TEST_MODULE UdpSocket_Test

#include "UdpSocketReceiver.h"
#include "UdpSocketTransmitter.h"
#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE (UdpSocket_Test)

BOOST_AUTO_TEST_CASE(UdpSocket_Test_Basic)
{
  int n = 100;
  vector<uint8_t> v(n);
  for(size_t i=0;i<n;i++)
    v[i] = i;

  UdpSocketTransmitter tx("127.0.0.1", 50005);
  UdpSocketReceiver rx(50005);

  tx.write(v.begin(), v.end());
  int av = rx.available();
  BOOST_REQUIRE(av == n*sizeof(uint8_t));

  vector<uint8_t> v2(n);
  rx.read(v2.begin(), v2.end());

  for(int i=0;i<n;i++)
    BOOST_REQUIRE(v[i] == v2[i]);
}

BOOST_AUTO_TEST_CASE(UdpSocket_Test_Float)
{
  int n = 100;
  vector<float> v(n);
  for(size_t i=0;i<n;i++)
    v[i] = i;

  UdpSocketTransmitter tx("127.0.0.1", 50005);
  UdpSocketReceiver rx(50005);

  tx.write(v.begin(), v.end());
  int av = rx.available();
  BOOST_REQUIRE(av == n*sizeof(float));

  vector<float> v2(n);
  rx.read(v2.begin(), v2.end());

  for(int i=0;i<n;i++)
    BOOST_REQUIRE(v[i] == v2[i]);
}

BOOST_AUTO_TEST_SUITE_END()
