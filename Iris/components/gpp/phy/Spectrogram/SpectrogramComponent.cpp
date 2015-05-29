/**
 * \file components/gpp/phy/Spectrogram/SpectrogramComponent.cpp
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
 * Implementation of a PSD estimator using a spectral periodogram.
 */

#include "SpectrogramComponent.h"
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace boost::lambda;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, SpectrogramComponent);

SpectrogramComponent::SpectrogramComponent(std::string name)
  : PhyComponent(name,
                "spectrogram",
                "A PSD estimator using a spectral periodogram",
                "Paul Sutton",
                "0.1")
  ,sp_(0)
{
  registerParameter("nfft", "FFT length", "512",
      false, nFft_x, Interval<int>(2,65536));
  registerParameter("windowlength", "Spectrogram window length", "256",
      false, windowLength_x, Interval<int>(2,65536));
  registerParameter("delay", "Delay between windows", "128",
      false, delay_x, Interval<int>(2,65536));
  registerParameter("nwindows", "Number of windows to average over", "16",
      false, nWindows_x, Interval<int>(1,65536));
  registerParameter("beta", "Kaiser-Bessel window parameter", "8.6",
      false, beta_x, Interval<float>(0,64));
  registerParameter("isprobe", "Act as a probe (provide PSDs via events)", "false",
      false, isProbe_x);
  registerParameter("issink", "Act as a sink (do not provide output)", "false",
      false, isSink_x);

  registerEvent(
      "psdevent",
      "An event providing the current estimated PSD (in dB)",
      TypeInfo< float >::identifier);
}

SpectrogramComponent::~SpectrogramComponent()
{
  if(sp_)
    spgram_destroy(sp_);
}

void SpectrogramComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< complex<float> >::identifier);
  if(!isSink_x)
  {
    std::vector<int> types;
    types.push_back(TypeInfo< float >::identifier);
    types.push_back(TypeInfo< complex<float> >::identifier);
    registerOutputPort("output1", types);
  }
}

void SpectrogramComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  if(!isSink_x)
  {
    if(isProbe_x)
      outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
    else
      outputTypes["output1"] = TypeInfo< float >::identifier;
  }
}

void SpectrogramComponent::initialize()
{
  if(delay_x > windowLength_x)
  {
    LOG(LERROR) << "Delay cannot exceed window length. "
                << "Setting delay to:" << windowLength_x;
    delay_x = windowLength_x;
  }
  n_ = 0;
  window_.reserve(windowLength_x);
  spec_.resize(nFft_x);
  psd_.resize(nFft_x);
  sp_ = spgram_create_kaiser(nFft_x, windowLength_x, beta_x);
}

void SpectrogramComponent::process()
{
  //Get a DataSet from the input DataBuffer
  DataSet< complex<float> >* readDataSet = NULL;
  getInputDataSet("input1", readDataSet);
  std::size_t size = readDataSet->data.size();

  //Fill windows and push to spectrogram
  CplxVecIt it = readDataSet->data.begin();
  for(;it != readDataSet->data.end(); ++it)
  {
    window_.push_back(*it);
    if(window_.size() == windowLength_x)
      processWindow();
  }

  if(!isSink_x && isProbe_x)
  {
    //Pass data through
    DataSet< complex<float> >* writeDataSet = NULL;
    getOutputDataSet("output1", writeDataSet, size);
    writeDataSet->data = readDataSet->data;
    writeDataSet->sampleRate = readDataSet->sampleRate;
    writeDataSet->timeStamp = readDataSet->timeStamp;
    releaseOutputDataSet("output1", writeDataSet);
  }

  releaseInputDataSet("input1", readDataSet);
}

void SpectrogramComponent::processWindow()
{
  //Calculate spectrogram, fftshift and accumulate
  spgram_push(sp_, &window_[0], windowLength_x);
  spgram_execute(sp_, &spec_[0]);
  for(int i=0;i<nFft_x;i++)
  {
    Cplx c = spec_[(i+nFft_x/2)%nFft_x];
    psd_[i] += real(c*conj(c));
  }

  //Clear containers
  CplxVec temp = window_;
  window_.assign(temp.begin()+delay_x,temp.end());
  spec_.assign(nFft_x, Cplx(0,0));

  //We've accumulated enough windows, normalize, convert to dB and output
  if(++n_ >= nWindows_x)
  {
    FloatVecIt it = psd_.begin();
    for(;it!=psd_.end();++it)
      *it = (10*log10(*it/nWindows_x));

    outputPsd();
    psd_.assign(nFft_x, 0.0);
    n_ = 0;
  }
}

void SpectrogramComponent::outputPsd()
{
  if(isProbe_x)
    activateEvent("psdevent", psd_);
  else
    if(!isSink_x)
    {
      DataSet< float >* writeDataSet = NULL;
      getOutputDataSet("output1", writeDataSet, nFft_x);
      writeDataSet->data = psd_;
      releaseOutputDataSet("output1", writeDataSet);
    }
}

} // namesapce phy
} // namespace iris
