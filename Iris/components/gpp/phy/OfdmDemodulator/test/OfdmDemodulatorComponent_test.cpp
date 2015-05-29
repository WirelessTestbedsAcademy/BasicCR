/**
 * \file components/gpp/phy/OfdmDemodulator/test/OfdmDemodulatorComponent_test.cpp
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
 * Main test file for OfdmDemodulator component.
 */

#define BOOST_TEST_MODULE OfdmDemodulatorComponent_Test

#include <boost/test/unit_test.hpp>

#include "../OfdmDemodulatorComponent.h"
#include "OfdmDemodulatorTestData.h"
#include "utility/DataBufferTrivial.h"

using namespace std;
using namespace iris;
using namespace iris::phy;

BOOST_AUTO_TEST_SUITE (OfdmDemodulatorComponent_Test)

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(OfdmDemodulatorComponent demod("test"));
}

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Parm_Test)
{
  OfdmDemodulatorComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("numdatacarriers") == "192");
  BOOST_CHECK(mod.getParameterDefaultValue("numpilotcarriers") == "8");
  BOOST_CHECK(mod.getParameterDefaultValue("numguardcarriers") == "55");
  BOOST_CHECK(mod.getParameterDefaultValue("cyclicprefixlength") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("threshold") == "0.827");
}

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Ports_Test)
{
  OfdmDemodulatorComponent mod("test");
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

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Init_Test)
{
  OfdmDemodulatorComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Demod_Test1)
{
  typedef complex<float>    Cplx;
  typedef vector<Cplx>      CplxVec;
  typedef CplxVec::iterator CplxVecIt;

  OfdmDemodulatorComponent mod("test");
  mod.setValue("numdatacarriers", 40);
  mod.setValue("numpilotcarriers", 8);
  mod.setValue("numguardcarriers", 15);
  mod.setValue("cyclicprefixlength", 8);
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< Cplx > in;
  DataBufferTrivial< uint8_t > out;

  // Create enough data for one full frame
  DataSet< Cplx >* iSet = NULL;
  in.getWriteData(iSet, OfdmDemodulatorTestData::testFrame1.size());
  copy(OfdmDemodulatorTestData::testFrame1.begin(),
       OfdmDemodulatorTestData::testFrame1.end(),
       iSet->data.begin());
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< uint8_t >* oSet = NULL;
  out.getReadData(oSet);
  for(int i=0; i<oSet->data.size(); i++)
    BOOST_CHECK(oSet->data[i]==i);
  out.releaseReadData(oSet);
}

BOOST_AUTO_TEST_CASE(OfdmDemodulatorComponent_Demod_Test2)
{
  typedef complex<float>    Cplx;
  typedef vector<Cplx>      CplxVec;
  typedef CplxVec::iterator CplxVecIt;

  OfdmDemodulatorComponent mod("test");
  mod.setValue("numdatacarriers", 40);
  mod.setValue("numpilotcarriers", 8);
  mod.setValue("numguardcarriers", 15);
  mod.setValue("cyclicprefixlength", 8);
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< Cplx > in;
  DataBufferTrivial< uint8_t > out;

  mod.setBuffers(&in,&out);
  mod.initialize();

  // Provide data in blocks of 4 samples
  DataSet< Cplx >* iSet = NULL;
  int numBlocks = OfdmDemodulatorTestData::testFrame1.size()/4;
  CplxVecIt begin = OfdmDemodulatorTestData::testFrame1.begin();
  CplxVecIt end = OfdmDemodulatorTestData::testFrame1.end();

  for(int i=0; i<numBlocks; i++)
  {
    in.getWriteData(iSet, 4);
    copy(begin,
         begin+4,
         iSet->data.begin());
    in.releaseWriteData(iSet);
    BOOST_REQUIRE_NO_THROW(mod.process());
    begin += 4;
  }

  BOOST_REQUIRE(out.hasData());
  DataSet< uint8_t >* oSet = NULL;
  out.getReadData(oSet);
  for(int i=0; i<oSet->data.size(); i++)
    BOOST_CHECK(oSet->data[i]==i);
  out.releaseReadData(oSet);
}

BOOST_AUTO_TEST_SUITE_END()
