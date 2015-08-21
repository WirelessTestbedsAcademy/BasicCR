/**
 * \file components/gpp/phy/SignalScaler/SignalScalerComponent_test.cpp
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
 * Main test file for SignalScaler component.
 */

#define BOOST_TEST_MODULE SignalScalerComponent_Test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "../SignalScalerComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;

BOOST_AUTO_TEST_SUITE (SignalScalerComponent_Test)

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(SignalScalerComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Parm_Test)
{
  SignalScalerComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("maximum") == "16384");
  BOOST_CHECK(mod.getParameterDefaultValue("factor") == "0");
}

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Ports_Test)
{
  SignalScalerComponent mod("test");
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
      TypeInfo< complex<float> >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< complex<float> >::identifier);
}

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Init_Test)
{
  SignalScalerComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Process_Test)
{
  SignalScalerComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< complex<float> > in;
  DataBufferTrivial< complex<float> > out;

  DataSet< complex<float> >* iSet = NULL;
  in.getWriteData(iSet, 128);
  for(int i=0;i<128;i++)
    iSet->data[i] = complex<float>(i,i);
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 128);
  float maxVal = abs(*max_element(oSet->data.begin(), oSet->data.end(),
                                  bind(norm<float>, _1) < bind(norm<float>, _2) ));
  BOOST_CHECK(maxVal - 16384 < 0.1);
  out.releaseReadData(oSet);
}

BOOST_AUTO_TEST_CASE(SignalScalerComponent_Process_Test2)
{
  SignalScalerComponent mod("test");
  mod.setValue("factor", 0.5f);
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< complex<float> > in;
  DataBufferTrivial< complex<float> > out;

  DataSet< complex<float> >* iSet = NULL;
  in.getWriteData(iSet, 128);
  for(int i=0;i<128;i++)
    iSet->data[i] = complex<float>(i,i);
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();
  BOOST_REQUIRE_NO_THROW(mod.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 128);
  for(int i=0;i<128;i++)
    BOOST_CHECK(oSet->data[i] == complex<float>(i/2.0, i/2.0));
  out.releaseReadData(oSet);
}

BOOST_AUTO_TEST_SUITE_END()
