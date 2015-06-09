/**
 * \file components/gpp/phy/OfdmModulator/benchmark/OfdmModulatorComponent_benchmark.cpp
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
 * Main benchmark file for OfdmModulator component.
 */

#include "../OfdmModulatorComponent.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
namespace bp = boost::posix_time;

int main(int argc, char* argv[])
{
  OfdmModulatorComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  DataBufferTrivial<uint8_t> in;
  DataBufferTrivial< complex<float> > out;

  // Create enough data for "numFrames" full frames
  int numFrames = 100;
  int numBytes = numFrames*32*24; // #dataSymbols * #bytesPerSymbol
  DataSet<uint8_t>* iSet = NULL;
  in.getWriteData(iSet, numBytes);
  for(int i=0;i<numBytes;i++)
    iSet->data[i] = i%255;
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();

  bp::ptime t1(bp::microsec_clock::local_time());
  mod.process();
  bp::ptime t2(bp::microsec_clock::local_time());

  bp::time_duration time = t2-t1;
  float megBytesPerSec = (numBytes/1.0e6)*(1.0e9/time.total_nanoseconds());
  cout << "Rate = " << megBytesPerSec << " MB/sec" << endl;
}
