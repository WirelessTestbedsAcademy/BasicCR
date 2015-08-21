/**
 * \file components/gpp/phy/PfbChannelizer/PfbChannelizerComponent.cpp
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
 * Implementation of polyphase filterbank channelizer.
 */

#include "PfbChannelizerComponent.h"

#include <sstream>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, PfbChannelizerComponent);

PfbChannelizerComponent::PfbChannelizerComponent(std::string name)
  : PhyComponent(name,                      // component name
                "pfbchannelizer",           // component type
                "A polyphase filterbank channelizer",  // description
                "Paul Sutton",              // author
                "0.1")                      // version
{
  taps.push_back(0.0f);
  channelizer = firpfbch_crcf_create(LIQUID_ANALYZER, 1, 1, &taps[0]);

  registerParameter(
    "debug", "Running in debug mode?",
    "false", false, debug_x);

  registerParameter(
    "numchannels", "Number of channels",
    "8", false, nChans_x, Interval<int>(1,65536));
}

PfbChannelizerComponent::~PfbChannelizerComponent()
{
  firpfbch_crcf_destroy(channelizer);
}

void PfbChannelizerComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< complex<float> >::identifier);

  for(int i=0; i<nChans_x; i++)
  {
    stringstream ss;
    ss << "output";
    ss << i;
    registerOutputPort(ss.str(), TypeInfo< complex<float> >::identifier);
  }
}

void PfbChannelizerComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  for(int i=0; i<nChans_x; i++)
  {
    stringstream ss;
    ss << "output";
    ss << i;
    outputTypes[ss.str()] = TypeInfo< complex<float> >::identifier;
  }
}

void PfbChannelizerComponent::initialize()
{
  // design custom filterbank channelizer
  unsigned int m  = 7;        // prototype filter delay
  float As        = 60.0f;    // stop-band attenuation
  channelizer = firpfbch_crcf_create_kaiser(LIQUID_ANALYZER, nChans_x, m, As);

  // create NCO to center spectrum
  float offset = -0.5f*(float)(nChans_x-1) / (float)nChans_x * 2 * M_PI;
  nco = nco_crcf_create(LIQUID_VCO);
  nco_crcf_set_frequency(nco, offset);

  //Set up our input and output buffers
  buf.resize(nChans_x);
  bufIt = buf.begin();
  outBuf.resize(nChans_x);
}

void PfbChannelizerComponent::process()
{

  //Get a DataSet from the input DataBuffer
  DataSet< complex<float> >* readDataSet = NULL;
  getInputDataSet("input1", readDataSet);
  std::size_t size = readDataSet->data.size();

  //Get output DataSets
  int numRuns = size/nChans_x;
  vector< DataSet< complex<float> >* > outSets(nChans_x);
  for(int i=0;i<nChans_x;i++)
  {
    stringstream ss;
    ss << "output";
    ss << i;
    getOutputDataSet(ss.str(), outSets[i], numRuns);
    outSets[i]->sampleRate = readDataSet->sampleRate/(double)nChans_x;
    outSets[i]->timeStamp = readDataSet->timeStamp;
  }

  // mix signal down
  for(int i=0;i<readDataSet->data.size();i++)
  {
    complex<float>* x = &readDataSet->data[i];
    nco_crcf_mix_down(nco, *x, x);
    nco_crcf_step(nco);
  }

  //Execute the channelizer
  int run=0;
  CplxVecIt it=readDataSet->data.begin();
  for(;it!=readDataSet->data.end();it++)
  {
    *bufIt++ = *it;
    if(bufIt == buf.end())
    {
      firpfbch_crcf_analyzer_execute(channelizer, &buf[0], &outBuf[0]);
      for(int i=0;i<nChans_x;i++)
      {
        outSets[i]->data[run] = outBuf[i];
      }
      bufIt = buf.begin();
      run++;
    }
  }

  //Release the DataSets
  releaseInputDataSet("input1", readDataSet);
  for(int i=0;i<nChans_x;i++)
  {
    stringstream ss;
    ss << "output";
    ss << i;
    releaseOutputDataSet(ss.str(), outSets[i]);
  }

}

void PfbChannelizerComponent::printTapsForMatlab()
{
  FILE*fid = fopen("PfbChannelizerFilter.m","w");
  fprintf(fid,"%% %s : auto-generated file\n", "PfbChannelizerFilter.m");
  fprintf(fid,"clear all;\n");
  fprintf(fid,"close all;\n\n");
  fprintf(fid,"h_len=%u;\n", (unsigned)taps.size());
  fprintf(fid,"cutoff=%12.4e;\n",0.5f/nChans_x);
  fprintf(fid,"channels=%u;\n",nChans_x);
  //fprintf(fid,"stopattenuation=%12.4e;\n",stopAtt_x);

  for (int i=0; i<taps.size(); i++)
    fprintf(fid,"h(%4u) = %20.8e;\n", i+1, taps[i]);

  fprintf(fid,"nfft=1024;\n");
  fprintf(fid,"H=20*log10(abs(fftshift(fft(h/channels,nfft))));\n");
  fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
  fprintf(fid,"figure; plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
  fprintf(fid,"grid on;\n");
  fprintf(fid,"xlabel('normalized frequency');\n");
  fprintf(fid,"ylabel('PSD [dB]');\n");
  //fprintf(fid,"title(['Prototype Kaiser filter design f_c: %f, S_L: %f, h: %u']);\n",
  //        0.5f/nChans_x, -stopAtt_x, (unsigned)taps.size());
  fprintf(fid,"axis([-0.5 0.5 -stopattenuation-40 10]);\n");
  fprintf(fid,"line([cutoff, cutoff] , [-stopattenuation-40 10], 'Color',[1 0 0]);\n");
  fprintf(fid,"line([-cutoff, -cutoff] , [-stopattenuation-40 10], 'Color',[1 0 0]);\n");

  fclose(fid);
}

} // namesapce phy
} // namespace iris
