/**
 * \file components/gpp/phy/OfdmModulator/test/OfdmModulatorComponent_test.cpp
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
 * Main test file for OfdmModulator component.
 */

#define BOOST_TEST_MODULE OfdmModulatorComponent_Test

#include <boost/test/unit_test.hpp>

#include "../OfdmModulatorComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;

BOOST_AUTO_TEST_SUITE (OfdmModulatorComponent_Test)

BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(OfdmModulatorComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Parm_Test)
{
  OfdmModulatorComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("numdatacarriers") == "192");
  BOOST_CHECK(mod.getParameterDefaultValue("numpilotcarriers") == "8");
  BOOST_CHECK(mod.getParameterDefaultValue("numguardcarriers") == "311");
  BOOST_CHECK(mod.getParameterDefaultValue("modulationdepth") == "1");
  BOOST_CHECK(mod.getParameterDefaultValue("cyclicprefixlength") == "32");
  BOOST_CHECK(mod.getParameterDefaultValue("maxsymbolsperframe") == "32");
}

BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Ports_Test)
{
  OfdmModulatorComponent mod("test");
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

BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Init_Test)
{
  OfdmModulatorComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Process_Test)
{
  OfdmModulatorComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial<uint8_t> in;
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
/*
BOOST_AUTO_TEST_CASE(OfdmModulatorComponent_Generate_Data)
{
  OfdmModulatorComponent mod("test");
  mod.setValue("numdatacarriers", 40);
  mod.setValue("numpilotcarriers", 8);
  mod.setValue("numguardcarriers", 15);
  mod.setValue("cyclicprefixlength", 8);
  mod.setValue("maxsymbolsperframe", 4);

  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial<uint8_t> in;
  DataBufferTrivial< complex<float> > out;

  // Create enough data for one full frame
  DataSet<uint8_t>* iSet = NULL;
  in.getWriteData(iSet, 4*5); // #dataSymbols * #bytesPerSymbol
  for(int i=0;i<4*5;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 8*72); // #symbols * #samplesPerSymbol

  RawFileUtility::write(oSet->data.begin(), oSet->data.end(), "Frame");
}
*/
BOOST_AUTO_TEST_SUITE_END()
