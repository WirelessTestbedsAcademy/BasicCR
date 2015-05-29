/**
 * \file components/gpp/phy/Spectrogram/SpectrogramComponent_test.cpp
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
 * Main test file for Spectrogram component.
 */

#define BOOST_TEST_MODULE SpectrogramComponent_Test

#include <boost/test/unit_test.hpp>

#include "../SpectrogramComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;

BOOST_AUTO_TEST_SUITE (SpectrogramComponent_Test)

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(SpectrogramComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Parm_Test)
{
  SpectrogramComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("nfft") == "512");
  BOOST_CHECK(mod.getParameterDefaultValue("windowlength") == "256");
  BOOST_CHECK(mod.getParameterDefaultValue("delay") == "128");
  BOOST_CHECK(mod.getParameterDefaultValue("nwindows") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("beta") == "8.6");
  BOOST_CHECK(mod.getParameterDefaultValue("isprobe") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("issink") == "false");
}

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Ports_Test0)
{
  SpectrogramComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(
    find(oPorts.front().supportedTypes.begin(),
         oPorts.front().supportedTypes.end(),
         TypeInfo< float >::identifier)
    != oPorts.front().supportedTypes.end());
  BOOST_REQUIRE(
    find(oPorts.front().supportedTypes.begin(),
         oPorts.front().supportedTypes.end(),
         TypeInfo< complex<float> >::identifier)
    != oPorts.front().supportedTypes.end());

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< float >::identifier);
}

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Ports_Test1)
{
  SpectrogramComponent mod("test");
  mod.setValue("issink", "true");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes.size() == 0);
}

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Ports_Test2)
{
  SpectrogramComponent mod("test");
  mod.setValue("isprobe", "true");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(
    find(oPorts.front().supportedTypes.begin(),
         oPorts.front().supportedTypes.end(),
         TypeInfo< float >::identifier)
    != oPorts.front().supportedTypes.end());
  BOOST_REQUIRE(
    find(oPorts.front().supportedTypes.begin(),
         oPorts.front().supportedTypes.end(),
         TypeInfo< complex<float> >::identifier)
    != oPorts.front().supportedTypes.end());

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< complex<float> >::identifier);
}

BOOST_AUTO_TEST_CASE(SpectrogramComponent_Init_Test)
{
  SpectrogramComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}
/*
BOOST_AUTO_TEST_CASE(SpectrogramComponent_Process_Test)
{
  SpectrogramComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< complex<float> > in;
  DataBufferTrivial< complex<float> > out;

  // Create enough data for one full frame
  DataSet<uint8_t>* iSet = NULL;
  in.getWriteData(iSet, 32*24); // #dataSymbols * #bytesPerSymbol
  for(int i=0;i<32*24;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 35*544); // #symbols * #samplesPerSymbol
  out.releaseReadData(oSet);
}
*/
BOOST_AUTO_TEST_SUITE_END()
