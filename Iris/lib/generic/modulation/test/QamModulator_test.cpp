/**
 * \file lib/generic/modulation/QamModulator_test.cpp
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
 * Main test file for QamModulator class.
 */

#define BOOST_TEST_MODULE QamModulator_Test

#include <boost/test/unit_test.hpp>

#include "QamModulator.h"

#include "irisapi/TypeInfo.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (QamModulator_Test)

/// Convenience function returns first element in array
template <typename T, size_t N>
T* begin(T(&arr)[N]) { return &arr[0]; }
/// Convenience function returns one past last element in array
template <typename T, size_t N>
T* end(T(&arr)[N]) { return &arr[0]+N; }

BOOST_AUTO_TEST_CASE(QamModulator_Bpsk_Fail_Test)
{
  uint8_t input[] = {0x00, 0x0F, 0xFF};
  vector< complex<float> > output(23);  // Should be length 24

  QamModulator q;
  BOOST_CHECK_THROW(q.modulate(begin(input), end(input),
                               output.begin(), output.end(),
                               BPSK), IrisException);
}

BOOST_AUTO_TEST_CASE(QamModulator_Qpsk_Fail_Test)
{
  uint8_t input[] = {0x00, 0x55, 0xAA, 0xFF};
  vector< complex<float> > output(15); // Should be length 16

  QamModulator q;
  BOOST_CHECK_THROW(q.modulate(begin(input), end(input),
                               output.begin(), output.end(),
                               QPSK), IrisException);
}

BOOST_AUTO_TEST_CASE(QamModulator_Qam16_Fail_Test)
{
  uint8_t input[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  vector< complex<float> > output(15);  // Should be length 16

  QamModulator q;
  BOOST_CHECK_THROW(q.modulate(begin(input), end(input),
                               output.begin(), output.end(),
                               QAM16), IrisException);
}

BOOST_AUTO_TEST_CASE(QamModulator_Bpsk_Test)
{
  uint8_t input[] = {0x00, 0x0F, 0xFF};
  vector< complex<float> > output(24);

  QamModulator q;
  BOOST_CHECK_NO_THROW(q.modulate(begin(input), end(input),
                                  output.begin(), output.end(),
                                  BPSK));

  complex<float> a(1,0);
  complex<float> b(-1,0);

  for(int i=0;i<12;i++)
    BOOST_CHECK(output[i] == a);

  for(int i=13;i<24;i++)
    BOOST_CHECK(output[i] == b);
}

BOOST_AUTO_TEST_CASE(QamModulator_Qpsk_Test)
{
  uint8_t input[] = {0x00, 0x55, 0xAA, 0xFF};
  vector< complex<float> > output(16);

  QamModulator q;
  BOOST_CHECK_NO_THROW(q.modulate(begin(input), end(input),
                                  output.begin(), output.end(),
                                  QPSK));

  complex<float> a(-1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f));
  complex<float> b(-1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f));
  complex<float> c( 1.0f/sqrtf(2.0f),-1.0f/sqrtf(2.0f));
  complex<float> d( 1.0f/sqrtf(2.0f), 1.0f/sqrtf(2.0f));

  for(int i=0;i<4;i++)
    BOOST_CHECK(output[i] == a);

  for(int i=5;i<8;i++)
    BOOST_CHECK(output[i] == b);

  for(int i=9;i<12;i++)
    BOOST_CHECK(output[i] == c);

  for(int i=13;i<16;i++)
    BOOST_CHECK(output[i] == d);
}

BOOST_AUTO_TEST_CASE(QamModulator_Qam16_Test)
{
  uint8_t input[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  vector< complex<float> > output(16);

  QamModulator q;
  BOOST_CHECK_NO_THROW(q.modulate(begin(input), end(input),
                                  output.begin(), output.end(),
                                  QAM16));

  vector< complex<float> > vec;
  vec.push_back(complex<float>(-1.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-1.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-3.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-3.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));

  vec.push_back(complex<float>(-1.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-1.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-3.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>(-3.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));

  vec.push_back(complex<float>( 1.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 1.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 3.0f/sqrtf(10.0f),-1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 3.0f/sqrtf(10.0f),-3.0f/sqrtf(10.0f)));

  vec.push_back(complex<float>( 1.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 1.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 3.0f/sqrtf(10.0f), 1.0f/sqrtf(10.0f)));
  vec.push_back(complex<float>( 3.0f/sqrtf(10.0f), 3.0f/sqrtf(10.0f)));

  for(int i=0;i<16;i++)
    BOOST_CHECK(output[i] == vec[i]);
}

BOOST_AUTO_TEST_SUITE_END()
