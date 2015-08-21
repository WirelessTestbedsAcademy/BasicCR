/**
 * \file components/gpp/phy/OfdmDemodulator/OfdmDemodulatorComponent.cpp
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
 * Implementation of the OfdmDemodulator component.
 */

#include "OfdmDemodulatorComponent.h"

#include <cmath>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <numeric>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "modulation/OfdmIndexGenerator.h"
#include "modulation/Crc.h"
#include "modulation/Whitener.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace boost::lambda;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, OfdmDemodulatorComponent);

OfdmDemodulatorComponent::OfdmDemodulatorComponent(std::string name)
  : PhyComponent(name,                            // component name
                "ofdmdemodulator",                // component type
                "An OFDM demodulation component", // description
                "Paul Sutton",                    // author
                "1.0")                            // version
    ,numHeaderBytes_(7)
    ,frameDetected_(false)
    ,haveHeader_(false)
    ,symbolLength_(0)
    ,rxNumBytes_(0)
    ,rxNumSymbols_(0)
    ,headerIndex_(0)
    ,frameIndex_(0)
    ,halfFft_(NULL)
    ,halfFftData_(NULL)
    ,fullFft_(NULL)
    ,fullFftData_(NULL)
    ,numRxFrames_(0)
    ,numRxFails_(0)
    ,symbolCount_(0)
{
  registerParameter(
    "debug", "Whether to write debug data to file.",
    "false", true, debug_x);

  registerParameter(
    "reportrate", "Report performance stats every reportrate frames.",
    "1000", true, reportRate_x);

  registerParameter(
    "numdatacarriers", "Number of data carriers (excluding pilots)",
    "192", true, numDataCarriers_x, Interval<int>(1,65536));

  registerParameter(
    "numpilotcarriers", "Number of pilot carriers",
    "8", true, numPilotCarriers_x, Interval<int>(1,65536));

  registerParameter(
    "numguardcarriers", "Number of guard carriers",
    "55", true, numGuardCarriers_x, Interval<int>(1,65536));

  registerParameter(
    "cyclicprefixlength", "Length of cyclic prefix",
    "16", true, cyclicPrefixLength_x, Interval<int>(1,65536));

  registerParameter(
    "threshold", "Frame detection threshold",
    "0.827", true, threshold_x, Interval<float>(0.0,1.0));

  // Create our pilot sequence
  typedef Cplx c;
  c seq[] = {c(1,0),c(1,0),c(-1,0),c(-1,0),c(-1,0),c(1,0),c(-1,0),c(1,0),};
  pilotSequence_.assign(begin(seq), end(seq));
}

OfdmDemodulatorComponent::~OfdmDemodulatorComponent()
{
  destroy();
}

void OfdmDemodulatorComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< complex<float> >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

void OfdmDemodulatorComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

void OfdmDemodulatorComponent::initialize()
{
  setup();
}

void OfdmDemodulatorComponent::process()
{
  getInputDataSet("input1", in_);
  timeStamp_ = in_->timeStamp;
  sampleRate_ = in_->sampleRate;
  CplxVecIt begin = in_->data.begin();
  CplxVecIt end = in_->data.end();

  try
  {
    while(begin != end)
    {
      if(!frameDetected_)
        begin = searchInput(begin, end);
      else
        begin = processFrame(begin, end);
    }
  }
  catch(IrisException& e)
  {
    LOG(LDEBUG) << e.what();
    headerIndex_ = 0;
    frameIndex_ = 0;
    frameDetected_ = false;
    haveHeader_ = false;
    numRxFails_++;
  }

  releaseInputDataSet("input1", in_);

  if(numRxFrames_ >= reportRate_x)
  {
    float successRate = 1-((float)numRxFails_/numRxFrames_);
    LOG(LINFO) << "Frame succcess rate: " << successRate*100 << "%";
    numRxFrames_ = 0;
    numRxFails_ = 0;
  }
}

void OfdmDemodulatorComponent::parameterHasChanged(std::string name)
{
  if(name == "numdatacarriers" || name == "numpilotcarriers" ||
     name == "numguardcarriers" || name == "cyclicprefixlength")
  {
    destroy();
    setup();
  }

  if(name == "threshold")
    detector_.reset(numBins_,cyclicPrefixLength_x,threshold_x);
}

void OfdmDemodulatorComponent::setup()
{
  // Set up index vectors
  pilotIndices_.clear();
  pilotIndices_.resize(numPilotCarriers_x);
  dataIndices_.clear();
  dataIndices_.resize(numDataCarriers_x);
  OfdmIndexGenerator::generateIndices(numDataCarriers_x,
                                      numPilotCarriers_x,
                                      numGuardCarriers_x,
                                      pilotIndices_.begin(), pilotIndices_.end(),
                                      dataIndices_.begin(), dataIndices_.end());

  numBins_ = numDataCarriers_x + numPilotCarriers_x + numGuardCarriers_x + 1;
  symbolLength_ = numBins_ + cyclicPrefixLength_x;
  numHeaderSymbols_ = (int)ceil(numHeaderBytes_/((float)numDataCarriers_x/8));

  preamble_.clear();
  preamble_.resize(numBins_);
  preambleBins_.resize(numBins_/2);

  preambleGen_.generatePreamble(numDataCarriers_x,
                                numPilotCarriers_x,
                                numGuardCarriers_x,
                                preamble_.begin(),
                                preamble_.end());

  if(debug_x)
    RawFileUtility::write(preamble_.begin(), preamble_.end(),
                          "OutputData/RxKnownPreamble");

  halfFftData_ = reinterpret_cast<Cplx*>(
      fftwf_malloc(sizeof(fftwf_complex) * numBins_/2));
  fill(&halfFftData_[0], &halfFftData_[numBins_/2], Cplx(0,0));
  fullFftData_ = reinterpret_cast<Cplx*>(
      fftwf_malloc(sizeof(fftwf_complex) * numBins_));
  fill(&fullFftData_[0], &fullFftData_[numBins_], Cplx(0,0));
  halfFft_ = fftwf_plan_dft_1d(numBins_/2,
                               (fftwf_complex*)halfFftData_,
                               (fftwf_complex*)halfFftData_,
                               FFTW_FORWARD,
                               FFTW_MEASURE);
  fullFft_ = fftwf_plan_dft_1d(numBins_,
                               (fftwf_complex*)fullFftData_,
                               (fftwf_complex*)fullFftData_,
                               FFTW_FORWARD,
                               FFTW_MEASURE);

  copy(preamble_.begin(), preamble_.begin()+numBins_/2, halfFftData_);
  fftwf_execute(halfFft_);
  copy(halfFftData_, halfFftData_+numBins_/2, preambleBins_.begin());
  transform(preambleBins_.begin(),
            preambleBins_.end(),
            preambleBins_.begin(),
            2.0f*_1);
  if(debug_x)
    RawFileUtility::write(preambleBins_.begin(), preambleBins_.end(),
                          "OutputData/RxKnownPreambleBins");

  rxPreamble_.resize(symbolLength_);
  corrector_.resize(symbolLength_);
  rxHeader_.resize(symbolLength_*numHeaderSymbols_);
  equalizer_.resize(numBins_);

  detector_.reset(numBins_,cyclicPrefixLength_x,threshold_x, debug_x);
}

void OfdmDemodulatorComponent::destroy()
{
  if(halfFft_ != NULL)
    fftwf_destroy_plan(halfFft_);
  if(fullFft_ != NULL)
    fftwf_destroy_plan(fullFft_);
  if(halfFftData_ != NULL)
    fftwf_free(halfFftData_);
  if(fullFftData_ != NULL)
    fftwf_free(fullFftData_);
}

OfdmDemodulatorComponent::CplxVecIt
OfdmDemodulatorComponent::searchInput(CplxVecIt begin, CplxVecIt end)
{
  float snr(0);
  CplxVecIt it = detector_.search(begin, end,
                                  rxPreamble_.begin(), rxPreamble_.end(),
                                  frameDetected_, fracFreqOffset_, snr);
  if(frameDetected_)
  {
    int idx = (it-in_->data.begin()) - (numBins_+cyclicPrefixLength_x);
    timeStamp_ = timeStamp_ + (idx/sampleRate_);
    extractPreamble();
  }
  return it;
}

OfdmDemodulatorComponent::CplxVecIt
OfdmDemodulatorComponent::processFrame(CplxVecIt begin, CplxVecIt end)
{
  for(; begin != end; begin++)
  {
    if(!haveHeader_)
    {
      rxHeader_[headerIndex_++] = *begin;
      if(headerIndex_ == symbolLength_*numHeaderSymbols_)
        extractHeader();
    }
    else
    {
      rxFrame_[frameIndex_++] = *begin;
      if(frameIndex_ == symbolLength_*rxNumSymbols_)
      {
        demodFrame();
        return ++begin;
      }
    }
  }
  return begin;
}

void OfdmDemodulatorComponent::extractPreamble()
{
  generateFractionalOffsetCorrector(fracFreqOffset_);
  correctFractionalOffset(rxPreamble_.begin(), rxPreamble_.end());

  int off = cyclicPrefixLength_x-4;
  CplxVecIt begin = rxPreamble_.begin() + off;
  CplxVecIt end = rxPreamble_.begin() + off + (numBins_/2);

  if(debug_x)
    RawFileUtility::write(begin, end, "OutputData/RxPreamble");

  int halfBins = numBins_/2;
  CplxVec bins(halfBins);
  copy(begin, end, halfFftData_);
  fftwf_execute(halfFft_);
  copy(halfFftData_, halfFftData_+halfBins, bins.begin());
  transform(bins.begin(), bins.end(), bins.begin(), _1*Cplx(2,0));

  if(debug_x)
    RawFileUtility::write(bins.begin(), bins.end(),
                          "OutputData/RxPreambleHalfBins");

  intFreqOffset_ = findIntegerOffset(bins.begin(), bins.end());
  int shift = (halfBins-intFreqOffset_)%halfBins;
  rotate(bins.begin(), bins.begin()+shift, bins.end());

  if(debug_x)
    RawFileUtility::write(bins.begin(), bins.end(),
                          "OutputData/RxPreambleHalfBinsRotated");

  generateEqualizer(bins.begin(), bins.end());
}

void OfdmDemodulatorComponent::extractHeader()
{
  symbolCount_ = 0;
  numRxFrames_++;
  int bytesPerHeader = numDataCarriers_x/8;
  ByteVec data(numHeaderSymbols_*bytesPerHeader);
  ByteVecIt dataIt = data.begin();
  CplxVecIt symIt = rxHeader_.begin();
  for(int i=0; i<numHeaderSymbols_; i++)
  {
    demodSymbol(symIt, symIt+symbolLength_,
                dataIt, dataIt+bytesPerHeader, BPSK);
    symIt += symbolLength_;
    dataIt += numDataCarriers_x/8;
    symbolCount_++;
  }

  Whitener::whiten(data.begin(), data.end());

  rxCrc_ = 0;
  rxCrc_ = data[3];
  rxCrc_ |= (data[2] << 8);
  rxCrc_ |= (data[1] << 16);
  rxCrc_ |= (data[0] << 24);

  rxModulation_ = data[6] & 0xFF;
  if(rxModulation_!=BPSK && rxModulation_!=QPSK && rxModulation_!=QAM16)
    throw IrisException("Invalid modulation depth - dropping frame.");

  rxNumBytes_ = ((data[4]<<8) | data[5]) & 0xFFFF;
  int bytesPerSymbol = (numDataCarriers_x*rxModulation_)/8;
  rxNumSymbols_ = ceil(rxNumBytes_/(float)bytesPerSymbol);
  if(rxNumSymbols_>32 || rxNumSymbols_<1)
    throw IrisException("Invalid frame length - dropping frame.");

  rxFrame_.resize(rxNumSymbols_*symbolLength_);
  haveHeader_ = true;
}

void OfdmDemodulatorComponent::demodFrame()
{
  int bytesPerSymbol = (numDataCarriers_x*rxModulation_)/8;
  int frameDataLen = (rxNumSymbols_*bytesPerSymbol);
  frameData_.resize(frameDataLen);

  CplxVecIt inIt = rxFrame_.begin();
  ByteVecIt outIt = frameData_.begin();
  for(int i=0;i<rxNumSymbols_;i++)
  {
    demodSymbol(inIt, inIt+symbolLength_,
                outIt, outIt+bytesPerSymbol,
                rxModulation_);
    inIt += symbolLength_;
    outIt += bytesPerSymbol;
    symbolCount_++;
  }

  outIt = frameData_.begin();
  Whitener::whiten(outIt, outIt+rxNumBytes_);
  uint32_t crc = Crc::generate(outIt, outIt+rxNumBytes_);
  if(crc != rxCrc_)
    throw IrisException("CRC mismatch - dropping frame.");

  DataSet< uint8_t>* out;
  getOutputDataSet("output1", out, rxNumBytes_);
  out->sampleRate = sampleRate_;
  out->timeStamp = timeStamp_;
  copy(outIt, outIt+rxNumBytes_, out->data.begin());
  releaseOutputDataSet("output1", out);

  headerIndex_ = 0;
  frameIndex_ = 0;
  frameDetected_ = false;
  haveHeader_ = false;
}

void OfdmDemodulatorComponent::demodSymbol(CplxVecIt inBegin, CplxVecIt inEnd,
                                           ByteVecIt outBegin, ByteVecIt outEnd,
                                           int modulationDepth)
{
  correctFractionalOffset(inBegin, inEnd);

  int off = cyclicPrefixLength_x-4;
  CplxVecIt begin = inBegin + off;
  CplxVecIt end = inBegin + off + numBins_;

  CplxVec bins(numBins_);
  copy(begin, end, fullFftData_);
  fftwf_execute(fullFft_);
  copy(fullFftData_, fullFftData_+numBins_, bins.begin());

  if(debug_x)
  {
    stringstream fileName;
    fileName << "OutputData//RxSymbolBins" << symbolCount_;
    RawFileUtility::write(bins.begin(), bins.end(),
                          fileName.str());
  }

  int shift = (numBins_-intFreqOffset_*2)%numBins_;
  rotate(bins.begin(), bins.begin()+shift, bins.end());

  if(debug_x)
  {
    stringstream fileName;
    fileName << "OutputData//RxSymbolBinsRotated" << symbolCount_;
    RawFileUtility::write(bins.begin(), bins.end(),
                          fileName.str());
  }

  equalizeSymbol(bins.begin(), bins.end());

  if(debug_x)
  {
    stringstream fileName;
    fileName << "OutputData//RxSymbolBinsEqualized" << symbolCount_;
    RawFileUtility::write(bins.begin(), bins.end(),
                          fileName.str());
  }

  CplxVec qamSymbols;
  for(int i=0; i<numDataCarriers_x; i++)
    qamSymbols.push_back(bins[dataIndices_[i]]);


  if(debug_x)
  {
    stringstream fileName;
    fileName << "OutputData//RxSymbolData" << symbolCount_;
    RawFileUtility::write(qamSymbols.begin(), qamSymbols.end(),
                          fileName.str());
  }

  qDemod_.demodulate(qamSymbols.begin(), qamSymbols.end(),
                     outBegin, outEnd, modulationDepth);
}

void OfdmDemodulatorComponent::generateFractionalOffsetCorrector(float offset)
{
  float relFreq = -offset/numBins_;
  toneGenerator_.generate(corrector_.begin(), corrector_.end(), relFreq);
  if(debug_x)
    RawFileUtility::write(corrector_.begin(), corrector_.end(),
                          "OutputData/RxFreqCorrector");
}

void OfdmDemodulatorComponent::correctFractionalOffset(CplxVecIt begin,
                                                       CplxVecIt end)
{
  transform(begin, end, corrector_.begin(), begin, _1*_2);
}

int OfdmDemodulatorComponent::findIntegerOffset(CplxVecIt begin, CplxVecIt end)
{
  FloatVec magRxBins(numBins_/2);
  transform(begin, end, magRxBins.begin(), opAbs());

  FloatVec magTxBins(numBins_);
  transform(preambleBins_.begin(), preambleBins_.end(),
            magTxBins.begin(), opAbs());
  FloatVecIt it = magTxBins.begin();
  copy(it, it+(numBins_/2), it+(numBins_/2));

  FloatVec correlations;
  //Calculate negative offset correlations
  FloatVecIt txIt = magTxBins.begin()+(numBins_/2);
  for(int i=-16; i<0; i++)
  {
    float res = inner_product(txIt+i, txIt+i+(numBins_/2),
                              magRxBins.begin(), 0.0f);
    correlations.push_back(res);
  }
  //Calculate positive offset correlations
  txIt = magTxBins.begin();
  for(int i=0; i<17; i++)
  {
    float res = inner_product(txIt+i, txIt+i+(numBins_/2),
                              magRxBins.begin(), 0.0f);
    correlations.push_back(res);
  }

  if(debug_x)
    RawFileUtility::write(correlations.begin(), correlations.end(),
                          "OutputData/RxFreqOffsetCorrelations");

  FloatVecIt result = max_element(correlations.begin(), correlations.end());
  int off =  (int)distance(correlations.begin(), result) - 16;
  return off;
}

void OfdmDemodulatorComponent::generateEqualizer(CplxVecIt begin, CplxVecIt end)
{
  CplxVec shortEq(numBins_/2);
  transform(begin, end, preambleBins_.begin(), shortEq.begin(), _2/_1);

  if(debug_x)
    RawFileUtility::write(shortEq.begin(), shortEq.end(),
                          "OutputData/RxShortEqualizer");

  shortEq[0] = (shortEq[(numBins_/2)-1] + shortEq[1])/Cplx(2,0);
  for(int i=0; i<numBins_/2; i++)
    equalizer_[i*2] = shortEq[i];
  for(int i=1; i<numBins_; i+=2)
    equalizer_[i] = (equalizer_[i-1] + equalizer_[(i+1)%numBins_])/Cplx(2,0);
  equalizer_[0] = Cplx(0,0);

  if(debug_x)
    RawFileUtility::write(equalizer_.begin(), equalizer_.end(),
                          "OutputData/RxEqualizer");
}

void OfdmDemodulatorComponent::equalizeSymbol(CplxVecIt begin, CplxVecIt end)
{
  transform(begin, end, equalizer_.begin(), begin, _1*_2);

  CplxVec pilots;
  for(int i=0; i<numPilotCarriers_x; i++)
    pilots.push_back(*(begin+pilotIndices_[i]));

  CplxVec diffs;
  for(int i=0; i<numPilotCarriers_x; i++)
    diffs.push_back(pilotSequence_[i%numPilotCarriers_x]/pilots[i]);

  Cplx sum = accumulate(diffs.begin(), diffs.end(), Cplx(0,0));
  float ave = arg(sum/(float)numPilotCarriers_x);

  Cplx corrector = Cplx(cos(ave), sin(ave));
  transform(begin, end, begin, _1*corrector);
}

} // namesapce phy
} // namespace iris
