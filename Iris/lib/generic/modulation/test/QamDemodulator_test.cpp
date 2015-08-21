/**
 * \file lib/generic/modulation/QamDemodulator_test.cpp
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
 * Main test file for QamDemodulator class.
 */

#define BOOST_TEST_MODULE QamDemodulator_Test

#include <boost/test/unit_test.hpp>

#include "QamDemodulator.h"

#include "irisapi/TypeInfo.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (QamDemodulator_Test)

/// Convenience function returns pointer to first element in array
template <typename T, size_t N>
T* begin(T(&arr)[N]) { return &arr[0]; }
/// Convenience function returns pointer to one past last element in array
template <typename T, size_t N>
T* end(T(&arr)[N]) { return &arr[0]+N; }

BOOST_AUTO_TEST_CASE(QamDemodulator_Bpsk_Fail_Test)
{
  vector< complex<float> > input;
  for(int i=0;i<16;i++)
    input.push_back(complex<float>(1,0));

  uint8_t output[1];  // Should be length 2

  QamDemodulator q;
  BOOST_CHECK_THROW(q.demodulate(input.begin(), input.end(),
                                 begin(output), end(output),
                                 BPSK), IrisException);
}

BOOST_AUTO_TEST_CASE(QamDemodulator_Qpsk_Fail_Test)
{
  vector< complex<float> > input;
  for(int i=0;i<16;i++)
    input.push_back(complex<float>(-1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f)));

  uint8_t output[3]; // Should be length 4

  QamDemodulator q;
  BOOST_CHECK_THROW(q.demodulate(input.begin(), input.end(),
                                 begin(output), end(output),
                                 QPSK), IrisException);
}

BOOST_AUTO_TEST_CASE(QamDemodulator_Qam16_Fail_Test)
{
  vector< complex<float> > input;
  for(int i=0;i<16;i++)
    input.push_back(complex<float>(-1.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));

  uint8_t output[7]; // Should be length 8

  QamDemodulator q;
  BOOST_CHECK_THROW(q.demodulate(input.begin(), input.end(),
                                 begin(output), end(output),
                                 QAM16), IrisException);
}

BOOST_AUTO_TEST_CASE(QamDemodulator_Bpsk_Test)
{
  vector< complex<float> > input;
  input.push_back(complex<float>(1,0));
  input.push_back(complex<float>(-1,0));
  input.push_back(complex<float>(1,0));
  input.push_back(complex<float>(-1,0));
  input.push_back(complex<float>(1,0));
  input.push_back(complex<float>(-1,0));
  input.push_back(complex<float>(1,0));
  input.push_back(complex<float>(-1,0));

  uint8_t output[1];

  QamDemodulator q;
  BOOST_CHECK_NO_THROW(q.demodulate(input.begin(), input.end(),
                                    begin(output), end(output),
                                    BPSK));

  uint8_t a = 0x55;

  BOOST_CHECK(output[0] == a);
}

BOOST_AUTO_TEST_CASE(QamDemodulator_Qpsk_Test)
{
  vector< complex<float> > input;
  input.push_back(complex<float>(-1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>(-1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>(-1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>(-1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f)));

  uint8_t output[2];

  QamDemodulator q;
  BOOST_CHECK_NO_THROW(q.demodulate(input.begin(), input.end(),
                                    begin(output), end(output),
                                    QPSK));

  uint8_t a = 0x1B;
  BOOST_CHECK(output[0] == a);
  BOOST_CHECK(output[1] == a);
}

BOOST_AUTO_TEST_CASE(QamDemodulator_Qam16_Test)
{
  vector< complex<float> > input;
  input.push_back(complex<float>(-1.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-1.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-3.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-3.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));

  input.push_back(complex<float>(-1.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-1.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-3.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>(-3.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));

  input.push_back(complex<float>( 1.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 3.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 3.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));

  input.push_back(complex<float>( 1.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 1.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 3.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  input.push_back(complex<float>( 3.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));

  uint8_t output[8];

  QamDemodulator q;
  BOOST_CHECK_NO_THROW(q.demodulate(input.begin(), input.end(),
                                    begin(output), end(output),
                                    QAM16));

  uint8_t expected[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  for(int i=0;i<8;i++)
    BOOST_CHECK(output[i] == expected[i]);
}

BOOST_AUTO_TEST_SUITE_END()
