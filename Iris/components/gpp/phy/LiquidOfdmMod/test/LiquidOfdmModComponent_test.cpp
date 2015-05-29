/**
 * \file components/gpp/phy/LiquidOfdmMod/LiquidOfdmModComponent_test.cpp
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
 * Main test file for LiquidOfdmMod component.
 */

#define BOOST_TEST_MODULE LiquidOfdmModComponent_test

#include <boost/test/unit_test.hpp>

#include "../LiquidOfdmModComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;

BOOST_AUTO_TEST_SUITE (LiquidOfdmModComponent_test)

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(LiquidOfdmModComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Parm_Test)
{
  LiquidOfdmModComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("subcarriers") == "64");
  BOOST_CHECK(mod.getParameterDefaultValue("prefixlength") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("taperlength") == "4");
  BOOST_CHECK(mod.getParameterDefaultValue("modulation") == "qpsk");
  BOOST_CHECK(mod.getParameterDefaultValue("fec0") == "none");
  BOOST_CHECK(mod.getParameterDefaultValue("fec1") == "h128");
  BOOST_CHECK(mod.getParameterDefaultValue("crc") == "crc32");
  BOOST_CHECK(mod.getParameterDefaultValue("frameheader") == "default");
}

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Ports_Test)
{
  LiquidOfdmModComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< complex<float> >::identifier);
}

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Init_Test)
{
  LiquidOfdmModComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Process_Test)
{
  LiquidOfdmModComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial<uint8_t> in;
  DataBufferTrivial< complex<float> > out;

  DataSet<uint8_t>* iSet = NULL;
  in.getWriteData(iSet, 32*24);

  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 9280);
  out.releaseReadData(oSet);
}

BOOST_AUTO_TEST_CASE(LiquidOfdmModComponent_Reconfigure_Test)
{
  LiquidOfdmModComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial<uint8_t> in;
  DataBufferTrivial< complex<float> > out;

  DataSet<uint8_t>* iSet = NULL;
  in.getWriteData(iSet, 32*24);

  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 9280);
  out.releaseReadData(oSet);

  mod.setValue("subcarriers", "128");
  mod.parameterHasChanged("subcarriers");

  in.getWriteData(iSet, 32*24);
  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 8496);
  out.releaseReadData(oSet);

  mod.setValue("prefixlength", "32");
  mod.parameterHasChanged("prefixlength");

  in.getWriteData(iSet, 32*24);
  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 9440);
  out.releaseReadData(oSet);

  mod.setValue("taperlength", "8");
  mod.parameterHasChanged("taperlength");

  in.getWriteData(iSet, 32*24);
  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 9440);
  out.releaseReadData(oSet);


}
BOOST_AUTO_TEST_SUITE_END()
