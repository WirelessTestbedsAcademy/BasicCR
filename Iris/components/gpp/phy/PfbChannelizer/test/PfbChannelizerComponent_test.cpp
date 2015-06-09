/**
 * \file components/gpp/phy/PfbChannelizer/test/PfbChannelizerComponent_test.cpp
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
 * Main test file for PfbChannelizer component.
 */

#define BOOST_TEST_MODULE PfbChannelizerComponent_Test

#include <boost/test/unit_test.hpp>

#include "../PfbChannelizerComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"
#include "math/MathDefines.h"

using namespace std;
using namespace iris;
using namespace iris::phy;

BOOST_AUTO_TEST_SUITE (PfbChannelizerComponent_Test)

BOOST_AUTO_TEST_CASE(PfbChannelizerComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(PfbChannelizerComponent chan("test"));
}

BOOST_AUTO_TEST_CASE(PfbChannelizerComponent_Parm_Test)
{
  PfbChannelizerComponent chan("test");
  BOOST_CHECK(chan.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(chan.getParameterDefaultValue("numchannels") == "8");

  BOOST_CHECK(chan.getValue("debug") == "false");
  BOOST_CHECK(chan.getValue("numchannels") == "8");
}

BOOST_AUTO_TEST_CASE(PfbChannelizerComponent_Ports_Test)
{
  PfbChannelizerComponent chan("test");
  BOOST_REQUIRE_NO_THROW(chan.registerPorts());

  vector<Port> iPorts = chan.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  vector<Port> oPorts = chan.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 8);
  for( int i=0; i< oPorts.size(); i++)
  {
    stringstream ss;
    ss << "output" << i;
    BOOST_REQUIRE(oPorts[i].portName == ss.str());
    BOOST_REQUIRE(oPorts[i].supportedTypes.front() ==
        TypeInfo< complex<float> >::identifier);
  }

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  chan.calculateOutputTypes(iTypes,oTypes);
  for( int i=0; i< oPorts.size(); i++)
  {
    stringstream ss;
    ss << "output" << i;
    BOOST_REQUIRE(oTypes[ss.str()] == TypeInfo< complex<float> >::identifier);
  }
}

BOOST_AUTO_TEST_CASE(PfbChannelizerComponent_Init_Test)
{
  PfbChannelizerComponent chan("test");
  chan.setValue("debug", "true");
  chan.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  chan.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(chan.initialize());
}

BOOST_AUTO_TEST_CASE(PfbChannelizerComponent_Process_Test)
{
  PfbChannelizerComponent chan("test");
  chan.setValue("debug", "true");
  chan.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< complex<float> >::identifier;
  chan.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial< complex<float> > in;
  DataBufferTrivial< complex<float> > out0;
  DataBufferTrivial< complex<float> > out1;
  DataBufferTrivial< complex<float> > out2;
  DataBufferTrivial< complex<float> > out3;
  DataBufferTrivial< complex<float> > out4;
  DataBufferTrivial< complex<float> > out5;
  DataBufferTrivial< complex<float> > out6;
  DataBufferTrivial< complex<float> > out7;

  // generate input signal(s)
  unsigned int num_signals = 4;
  float fc[4] = {0.0f,   0.25f,  0.375f, -0.375f}; // center frequencies
  float bw[4] = {0.015f, 0.025f, 0.035f,  0.045f}; // bandwidths
  unsigned int pulse_len = 137;
  DataSet< complex<float> >* iSet = NULL;
  in.getWriteData(iSet, pulse_len);
  float pulse[pulse_len];
  for (int i=0; i<num_signals; i++)
  {
    // create pulse
    liquid_firdes_kaiser(pulse_len, bw[i], 50.0f, 0.0f, pulse);

    // add pulse to input signal with carrier offset
    for(int k=0; k<pulse_len; k++)
      iSet->data[k] += pulse[k] * exp(complex<float>(0,2*IRIS_PI*fc[i]*k)) * bw[i];
  }
  RawFileUtility::write(iSet->data.begin(), iSet->data.end(), "input");
  in.releaseWriteData(iSet);


  vector<ReadBufferBase*> ins;
  vector<WriteBufferBase*> outs;
  ins.push_back(&in);
  outs.push_back(&out0);
  outs.push_back(&out1);
  outs.push_back(&out2);
  outs.push_back(&out3);
  outs.push_back(&out4);
  outs.push_back(&out5);
  outs.push_back(&out6);
  outs.push_back(&out7);

  chan.setBuffers(ins,outs);
  chan.initialize();
  BOOST_REQUIRE_NO_THROW(chan.process());
  int x=0;

  vector< ReadBuffer< complex<float> >* > v;
  v.push_back(&out0);
  v.push_back(&out1);
  v.push_back(&out2);
  v.push_back(&out3);
  v.push_back(&out4);
  v.push_back(&out5);
  v.push_back(&out6);
  v.push_back(&out7);

  for(int i=0;i<8;i++)
  {
    stringstream ss;
    ss << "out" << i;
    ReadBuffer< complex<float> >* r = v[i];
    BOOST_REQUIRE(r->hasData());
    DataSet< complex<float> >* oSet = NULL;
    r->getReadData(oSet);
    BOOST_CHECK(oSet->data.size() == 17);
    string file = ss.str();
    RawFileUtility::write(oSet->data.begin(), oSet->data.end(), file);
    r->releaseReadData(oSet);
  }

}
BOOST_AUTO_TEST_SUITE_END()
