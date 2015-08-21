/**
 * \file components/gpp/phy/OfdmDemodulator/benchmark/OfdmDemodulatorComponent_benchmark.cpp
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
 * Main benchmark file for OfdmDemodulator component.
 */

#include "../OfdmDemodulatorComponent.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "OfdmDemodulatorBenchmarkData.h"
#include "utility/DataBufferTrivial.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
namespace bp = boost::posix_time;

int main(int argc, char* argv[])
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

  // Create enough data for "numFrames" full frames
  int numFrames = 10000;
  int frameSize = OfdmDemodulatorBenchmarkData::testFrame1.size();
  DataSet< Cplx >* iSet = NULL;
  in.getWriteData(iSet, frameSize*numFrames);
  CplxVecIt it = iSet->data.begin();
  for(int i=0;i<numFrames;i++,it+=frameSize)
  {
    copy(OfdmDemodulatorBenchmarkData::testFrame1.begin(),
         OfdmDemodulatorBenchmarkData::testFrame1.end(),
         it);
  }
  in.releaseWriteData(iSet);

  mod.setBuffers(&in,&out);
  mod.initialize();

  bp::ptime t1(bp::microsec_clock::local_time());
  mod.process();
  bp::ptime t2(bp::microsec_clock::local_time());

  bp::time_duration time = t2-t1;
  float megSampsPerSec = (numFrames*frameSize/1.0e6)*(1.0e9/time.total_nanoseconds());
  cout << "Rate = " << megSampsPerSec << " MS/sec" << endl;
}
