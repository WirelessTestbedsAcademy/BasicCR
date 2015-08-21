/**
 * \file components/gpp/phy/PfbSynthesizer/test/PfbSynthesizerComponent_test.cpp
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
 * Main test file for PfbSynthesizer component.
 */

#define BOOST_TEST_MODULE PfbSynthesizerComponent_Test

#include <boost/test/unit_test.hpp>

#include "../PfbSynthesizerComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"
#include "math/MathDefines.h"

using namespace std;
using namespace iris;
using namespace iris::phy;

BOOST_AUTO_TEST_SUITE (PfbSynthesizerComponent_Test)

BOOST_AUTO_TEST_CASE(PfbSynthesizerComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(PfbSynthesizerComponent chan("test"));
}

BOOST_AUTO_TEST_CASE(PfbSynthesizerComponent_Parm_Test)
{
  PfbSynthesizerComponent chan("test");
  BOOST_CHECK(chan.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(chan.getParameterDefaultValue("numchannels") == "8");

  BOOST_CHECK(chan.getValue("debug") == "false");
  BOOST_CHECK(chan.getValue("numchannels") == "8");
}

BOOST_AUTO_TEST_CASE(PfbSynthesizerComponent_Ports_Test)
{
  PfbSynthesizerComponent chan("test");
  BOOST_REQUIRE_NO_THROW(chan.registerPorts());

  vector<Port> iPorts = chan.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 8);
  for( int i=0; i< iPorts.size(); i++)
  {
    stringstream ss;
    ss << "input" << i;
    BOOST_REQUIRE(iPorts[i].portName == ss.str());
    BOOST_REQUIRE(iPorts[i].supportedTypes.front() ==
        TypeInfo< complex<float> >::identifier);
  }

  vector<Port> oPorts = chan.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< complex<float> >::identifier);

  map<string, int> iTypes,oTypes;
  for( int i=0; i< iPorts.size(); i++)
  {
    stringstream ss;
    ss << "input" << i;
    iTypes[ss.str()] = TypeInfo< complex<float> >::identifier;
  }
  chan.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< complex<float> >::identifier);
}

BOOST_AUTO_TEST_CASE(PfbSynthesizerComponent_Init_Test)
{
  PfbSynthesizerComponent chan("test");
  chan.setValue("debug", "true");
  chan.registerPorts();
  vector<Port> iPorts = chan.getInputPorts();

  map<string, int> iTypes,oTypes;
  for( int i=0; i< iPorts.size(); i++)
  {
    stringstream ss;
    ss << "input" << i;
    iTypes[ss.str()] = TypeInfo< complex<float> >::identifier;
  }
  chan.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(chan.initialize());
}

BOOST_AUTO_TEST_CASE(PfbSynthesizerComponent_Process_Test)
{
  PfbSynthesizerComponent chan("test");
  chan.setValue("debug", "true");
  chan.registerPorts();
  vector<Port> iPorts = chan.getInputPorts();

  map<string, int> iTypes,oTypes;
  for( int i=0; i< iPorts.size(); i++)
  {
    stringstream ss;
    ss << "input" << i;
    iTypes[ss.str()] = TypeInfo< complex<float> >::identifier;
  }
  chan.calculateOutputTypes(iTypes,oTypes);

  vector< DataBufferTrivial< complex<float> >* > ins(iPorts.size());
  for(int i=0;i<iPorts.size();i++)
    ins[i] = new DataBufferTrivial< complex<float> >;
  DataBufferTrivial< complex<float> > out;

  vector<ReadBufferBase*> insBase;
  for(int i=0;i<ins.size();i++)
    insBase.push_back(ins[i]);
  vector<WriteBufferBase*> outsBase;
  outsBase.push_back(&out);


  // generate input signal(s)
  unsigned int num_signals = iPorts.size();
  float baseBw = 0.01f;
  unsigned int pulse_len = 137;
  DataSet< complex<float> >* iSet = NULL;
  for(int i=0;i<iPorts.size();i++)
  {
    ins[i]->getWriteData(iSet, pulse_len);
    float pulse[pulse_len];

    // create pulse
    liquid_firdes_kaiser(pulse_len, baseBw*(i+1), 50.0f, 0.0f, pulse);

    // add pulse to input signal with carrier offset
    for(int k=0; k<pulse_len; k++)
      iSet->data[k] = complex<float>(pulse[k],0);

    stringstream ss;
    ss << "in" << i;
    RawFileUtility::write(iSet->data.begin(), iSet->data.end(), ss.str());
    ins[i]->releaseWriteData(iSet);
  }

  chan.setBuffers(insBase,outsBase);
  chan.initialize();
  BOOST_REQUIRE_NO_THROW(chan.process());

  BOOST_REQUIRE(out.hasData());
  DataSet< complex<float> >* oSet = NULL;
  out.getReadData(oSet);
  BOOST_CHECK(oSet->data.size() == 1096);
  RawFileUtility::write(oSet->data.begin(), oSet->data.end(), "out");
  out.releaseReadData(oSet);

  for(int i=0;i<iPorts.size();i++)
    delete ins[i];
}

BOOST_AUTO_TEST_SUITE_END()
