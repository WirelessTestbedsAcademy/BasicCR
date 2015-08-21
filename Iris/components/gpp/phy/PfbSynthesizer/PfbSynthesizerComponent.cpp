/**
 * \file components/gpp/phy/PfbSynthesizer/PfbSynthesizerComponent.cpp
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
 * Implementation of polyphase filterbank synthesizer.
 */

#include "PfbSynthesizerComponent.h"

#include <sstream>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, PfbSynthesizerComponent);

PfbSynthesizerComponent::PfbSynthesizerComponent(std::string name)
  : PhyComponent(name,                      // component name
                "pfbsynthesizer",           // component type
                "A polyphase filterbank synthesizer",  // description
                "Paul Sutton",              // author
                "0.1")                      // version
{
  taps.push_back(0.0f);
  channelizer = firpfbch_crcf_create(LIQUID_SYNTHESIZER, 1, 1, &taps[0]);

  registerParameter(
    "debug", "Running in debug mode?",
    "false", false, debug_x);

  registerParameter(
    "numchannels", "Number of channels",
    "8", false, nChans_x, Interval<int>(1,65536));
}

PfbSynthesizerComponent::~PfbSynthesizerComponent()
{
  firpfbch_crcf_destroy(channelizer);
}

void PfbSynthesizerComponent::registerPorts()
{
  for(int i=0; i<nChans_x; i++)
  {
    stringstream ss;
    ss << "input";
    ss << i;
    registerInputPort(ss.str(), TypeInfo< complex<float> >::identifier);
  }

  registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void PfbSynthesizerComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

void PfbSynthesizerComponent::initialize()
{
  // design custom filterbank channelizer
  unsigned int m  = 7;        // prototype filter delay
  float As        = 60.0f;    // stop-band attenuation
  channelizer = firpfbch_crcf_create_kaiser(LIQUID_SYNTHESIZER, nChans_x, m, As);

  // create NCO to center spectrum
  float offset = -0.5f*(float)(nChans_x-1) / (float)nChans_x * 2 * M_PI;
  nco = nco_crcf_create(LIQUID_VCO);
  nco_crcf_set_frequency(nco, offset);

  //Set up our input and output buffers
  buf.resize(nChans_x);
  bufIt = buf.begin();
  outBuf.resize(nChans_x);
}

void PfbSynthesizerComponent::process()
{

  //Get input DataSets
  std::size_t curSize = 0;
  vector< DataSet< complex<float> >* > inSets(nChans_x);
  for(int i=0;i<nChans_x;i++)
  {
    stringstream ss;
    ss << "input";
    ss << i;
    getInputDataSet(ss.str(), inSets[i]);
    std::size_t s = inSets[i]->data.size();
    if(s != curSize && curSize > 0)
      LOG(LWARNING) << "Input channel sizes do not match.";
    curSize = s;
  }

  //Get output DataSet
  DataSet< complex<float> >* writeDataSet = NULL;
  getOutputDataSet("output1", writeDataSet, curSize*nChans_x);
  writeDataSet->sampleRate = inSets[0]->sampleRate*nChans_x;
  writeDataSet->timeStamp = inSets[0]->timeStamp;

  // Execute the synthesizer
  for(int i=0;i<curSize;i++)
  {
    for (int j=0; j<nChans_x; j++)
      buf[j] = inSets[j]->data[i];

    firpfbch_crcf_synthesizer_execute(channelizer, &buf[0], &outBuf[0]);
    copy(outBuf.begin(), outBuf.end(), writeDataSet->data.begin()+i*nChans_x);
  }

  // mix signal down
  for(int i=0;i<writeDataSet->data.size();i++)
  {
    complex<float>* x = &writeDataSet->data[i];
    nco_crcf_mix_down(nco, *x, x);
    nco_crcf_step(nco);
  }

  //Release the DataSets
  for(int i=0;i<nChans_x;i++)
  {
    stringstream ss;
    ss << "input";
    ss << i;
    releaseInputDataSet(ss.str(), inSets[i]);
  }
  releaseOutputDataSet("output1", writeDataSet);

}

void PfbSynthesizerComponent::printTapsForMatlab()
{
  FILE*fid = fopen("PfbSynthesizerFilter.m","w");
  fprintf(fid,"%% %s : auto-generated file\n", "PfbSynthesizerFilter.m");
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
