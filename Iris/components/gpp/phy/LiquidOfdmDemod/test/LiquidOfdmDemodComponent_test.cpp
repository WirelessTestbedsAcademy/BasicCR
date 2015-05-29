/**
 * \file components/gpp/phy/LiquidOfdmDemod/test/LiquidOfdmDemodComponent_Test.cpp
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
 * Main test file for LiquidOfdmDemod component.
 */

#define BOOST_TEST_MODULE LiquidOfdmDemodComponent_Test

#include <boost/test/unit_test.hpp>

#include "../LiquidOfdmDemodComponent.h"
#include "../../LiquidOfdmMod/LiquidOfdmModComponent.h"
#include "utility/DataBufferTrivial.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (LiquidOfdmDemodComponent_Test)

BOOST_AUTO_TEST_CASE(LiquidOfdmDemodComponent_Test)
{
  BOOST_REQUIRE_NO_THROW(LiquidOfdmDemodComponent demod("test"));
}

BOOST_AUTO_TEST_CASE(LiquidOfdmDemodComponent_Parm_Test)
{
  LiquidOfdmDemodComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("subcarriers") == "64");
  BOOST_CHECK(mod.getParameterDefaultValue("prefixlength") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("taperlength") == "4");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
}

BOOST_AUTO_TEST_CASE(LiquidOfdmDemodComponent_Ports_Test)
{
  LiquidOfdmDemodComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< uint8_t >::identifier);
}

BOOST_AUTO_TEST_CASE(LiquidOfdmDemodComponent_Init_Test)
{
  LiquidOfdmDemodComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_SUITE_END()
