/**
 * \file components/gpp/phy/OfdmModulator/OfdmModulatorComponent.cpp
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
 * Implementation of the OfdmModulator component.
 */

#include "OfdmModulatorComponent.h"

#include <cmath>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

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
IRIS_COMPONENT_EXPORTS(PhyComponent, OfdmModulatorComponent);

OfdmModulatorComponent::OfdmModulatorComponent(std::string name)
  : PhyComponent(name,                          // component name
                "ofdmmodulator",                // component type
                "An OFDM modulation component", // description
                "Paul Sutton",                  // author
                "1.0")                          // version
    ,numHeaderBytes_(7)
    ,numHeaderSymbols_(0)
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,fft_(NULL)
    ,fftBins_(NULL)
{
  registerParameter(
    "debug", "Whether to output debug data.",
    "false", true, debug_x);

  registerParameter(
    "numdatacarriers", "Number of data carriers (excluding pilots)",
    "192", true, numDataCarriers_x, Interval<int>(1,65536));

  registerParameter(
    "numpilotcarriers", "Number of pilot carriers",
    "8", true, numPilotCarriers_x, Interval<int>(1,65536));

  registerParameter(
    "numguardcarriers", "Number of guard carriers",
    "311", true, numGuardCarriers_x, Interval<int>(1,65536));

  int vals[] = {1,2,4};
  registerParameter(
    "modulationdepth", "Modulation depth (1=BPSK, 2=QPSK, 4=QAM16)",
    "1", true, modulationDepth_x, list<int>(begin(vals),end(vals)));

  registerParameter(
    "cyclicprefixlength", "Length of cyclic prefix",
    "32", true, cyclicPrefixLength_x, Interval<int>(1,65536));

  registerParameter(
    "maxsymbolsperframe", "Maximum number of data symbols per frame",
    "32", true, maxSymbolsPerFrame_x, Interval<int>(1,128));

  // Create our pilot sequence
  typedef Cplx c;
  c seq[] = {c(1,0),c(1,0),c(-1,0),c(-1,0),c(-1,0),c(1,0),c(-1,0),c(1,0),};
  pilotSequence_.assign(begin(seq), end(seq));
}

OfdmModulatorComponent::~OfdmModulatorComponent()
{
  destroy();
}

void OfdmModulatorComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void OfdmModulatorComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

void OfdmModulatorComponent::initialize()
{
  setup();
}

void OfdmModulatorComponent::process()
{
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);
  timeStamp_ = in->timeStamp;
  sampleRate_ = in->sampleRate;
  int size = (int)in->data.size();
  int numSymbols = ceil(size/(float)bytesPerSymbol_);

  ByteVecIt it = in->data.begin();
  do
  {
    int sizeThisFrame;
    if(numSymbols >= maxSymbolsPerFrame_x)
      sizeThisFrame = maxSymbolsPerFrame_x * bytesPerSymbol_;
    else
      sizeThisFrame = size;

    createHeader(it, it+sizeThisFrame);
    createFrame(it, it+sizeThisFrame);

    numSymbols -= maxSymbolsPerFrame_x;
    size -= (maxSymbolsPerFrame_x*bytesPerSymbol_);
    it += sizeThisFrame;

  }while(numSymbols > 0);

  releaseInputDataSet("input1", in);
}

void OfdmModulatorComponent::parameterHasChanged(std::string name)
{
  if(name == "numdatacarriers" || name == "numpilotcarriers" ||
     name == "numguardcarriers" || name == "cyclicprefixlength" ||
     name == "modulationdepth")
  {
    destroy();
    setup();
  }
}

/// Set up all our index vectors and containers.
void OfdmModulatorComponent::setup()
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

  // Create preamble
  numBins_ = numDataCarriers_x + numPilotCarriers_x + numGuardCarriers_x + 1;
  preamble_.clear();
  preamble_.resize(numBins_);
  preambleGen_.generatePreamble(numDataCarriers_x,
                                numPilotCarriers_x,
                                numGuardCarriers_x,
                                preamble_.begin(), preamble_.end());

  if(debug_x)
    RawFileUtility::write(preamble_.begin(), preamble_.end(),
                          "OutputData/TxPreamble");

  // Set up containers
  fftBins_ = reinterpret_cast<Cplx*>(
      fftwf_malloc(sizeof(fftwf_complex) * numBins_));
  fill(&fftBins_[0], &fftBins_[numBins_], Cplx(0,0));
  fft_ = fftwf_plan_dft_1d(numBins_,
                           (fftwf_complex*)fftBins_,
                           (fftwf_complex*)fftBins_,
                           FFTW_BACKWARD,
                           FFTW_MEASURE);
  symbol_.clear();
  symbol_.resize(numBins_);
  int bytesPerSymbol = numDataCarriers_x/8;
  numHeaderSymbols_ = (int)ceil(numHeaderBytes_/(float)bytesPerSymbol);
  header_.resize(numHeaderSymbols_*bytesPerSymbol);
  modHeader_.resize(numHeaderSymbols_*numDataCarriers_x);

  // Set up padding
  bytesPerSymbol_ = (numDataCarriers_x * modulationDepth_x)/8;
  pad_.resize(bytesPerSymbol_);
  Whitener::whiten(pad_.begin(), pad_.end());
  modPad_.resize(numDataCarriers_x);
  qMod_.modulate(pad_.begin(), pad_.end(),
                 modPad_.begin(), modPad_.end(),
                 modulationDepth_x);

}

void OfdmModulatorComponent::destroy()
{
  if(fftBins_ != NULL)
    fftwf_free(fftBins_);
  if(fft_ != NULL)
    fftwf_destroy_plan(fft_);
}

/** Create a header for the current frame.
 *
 * The header will occupy a single OFDM symbol and will be BPSK modulated.
 * The header structure is as follows:                                     <br>
 *       ---------------------------------------------------------         <br>
 * bits  |    32|                16|            8| bits/symbol-56|         <br>
 * data  |   CRC| Frame size(bytes)| QAM encoding|        padding|         <br>
 *       ---------------------------------------------------------         <br>
 *
 * @param begin Iterator to first byte of tx data.
 * @param end   Iterator to one past last byte of tx data.
 */
void OfdmModulatorComponent::createHeader(ByteVecIt begin, ByteVecIt end)
{
  //Add the CRC
  uint32_t crc = Crc::generate(begin,end);
  header_[0] = (crc>>24) & 0xFF;
  header_[1] = (crc>>16) & 0xFF;
  header_[2] = (crc>>8) & 0xFF;
  header_[3] = crc & 0xFF;

  //Add frame size
  uint16_t size = end-begin;
  header_[4] = (size>>8) & 0xFF;
  header_[5] = size & 0xFF;

  //Add the QAM encoding
  header_[6] = modulationDepth_x & 0xFF;

  //Pad the header with dummy data
  for(int i=7; i<header_.size(); i++)
    header_[i] = i;
}

/** Create an OFDM frame and write it to the output.
 *
 * The frame structure is as follows:                                     <br>
 *          --------------------------------------------------------      <br>
 * symbols  |        1 |     >=1 |           variable |           1 |     <br>
 * data     | Preamble |  Header |       Data Symbols | Frame Guard |     <br>
 *          --------------------------------------------------------      <br>
 *
 * @param begin   Iterator to first input data byte.
 * @param end     Iterator to one past last input data byte.
 */
void OfdmModulatorComponent::createFrame(ByteVecIt begin, ByteVecIt end)
{
  int numOfdmSymbols = ceil((end-begin)/(float)bytesPerSymbol_);
  int ofdmSymLength = numBins_+cyclicPrefixLength_x;

  // Whiten
  Whitener::whiten(header_.begin(), header_.end());
  Whitener::whiten(begin, end);

  // Modulate and pad
  qMod_.modulate(header_.begin(), header_.end(),
                 modHeader_.begin(), modHeader_.end(), BPSK);
  modData_.resize(numOfdmSymbols*numDataCarriers_x);
  CplxVecIt modIt = qMod_.modulate(begin, end,
                                   modData_.begin(), modData_.end(),
                                   modulationDepth_x);
  CplxVecIt padIt = modPad_.begin();
  for(; modIt!=modData_.end(); modIt++,padIt++)
    *modIt = *padIt;

  // Get a DataSet
  int frameLength = (1+numHeaderSymbols_+numOfdmSymbols+1) * (ofdmSymLength);
  DataSet< complex<float> >* out = NULL;
  getOutputDataSet("output1", out, frameLength);
  out->sampleRate = sampleRate_;
  out->timeStamp = timeStamp_;
  CplxVecIt it = out->data.begin();

  // Copy preamble
  it = copyWithCp(preamble_.begin(), preamble_.end(), it, it+ofdmSymLength);

  // Create and copy header symbol(s)
  CplxVecIt headIt = modHeader_.begin();
  for(; headIt != modHeader_.end(); headIt += numDataCarriers_x)
  {
    createSymbol(headIt, headIt+numDataCarriers_x, symbol_.begin(),symbol_.end());
    it = copyWithCp(symbol_.begin(), symbol_.end(), it, it+ofdmSymLength);
  }

  // Create and copy data symbols
  CplxVecIt inIt = modData_.begin();
  for(int i=0; i<numOfdmSymbols; i++, inIt += numDataCarriers_x)
  {
    createSymbol(inIt, inIt+numDataCarriers_x, symbol_.begin(), symbol_.end());
    it = copyWithCp(symbol_.begin(), symbol_.end(), it, it+ofdmSymLength);
  }

  if(debug_x)
    RawFileUtility::write(out->data.begin(), out->data.end(),
                          "OutputData/TxFrame");

  releaseOutputDataSet("output1", out);
}

/** Create a single OFDM symbol.
 *
 * Our pilot and data index vectors are used to map QAM symbols onto carriers
 * and an FFT is used to create the time-domain OFDM symbol.
 *
 * @param inBegin   Iterator to first input QAM symbol.
 * @param inEnd     Iterator to one past last input QAM symbol.
 * @param outBegin  Iterator to first sample of the output OFDM symbol.
 * @param outEnd    Iterator to one past last sample of the output symbol.
 */
void OfdmModulatorComponent::createSymbol(CplxVecIt inBegin, CplxVecIt inEnd,
                                          CplxVecIt outBegin, CplxVecIt outEnd)
{
  if(outEnd-outBegin < numBins_)
    throw IrisException("Insufficient storage provided for createSymbol output.");

  fill(&fftBins_[0], &fftBins_[numBins_], Cplx(0,0));

  int i = 0;
  IntVecIt it = pilotIndices_.begin();
  for(; it!=pilotIndices_.end(); it++, i++)
    fftBins_[*it] = pilotSequence_[i%pilotSequence_.size()];
  for(it=dataIndices_.begin(); it!= dataIndices_.end(); it++)
    fftBins_[*it] = *inBegin++;

  if(debug_x)
    RawFileUtility::write(&fftBins_[0], &fftBins_[numBins_],
                          "OutputData/TxSymbolBins");

  fftwf_execute(fft_);
  copy(&fftBins_[0], &fftBins_[numBins_], outBegin);
  float scaleFactor = numPilotCarriers_x + numDataCarriers_x;
  transform(outBegin, outEnd, outBegin, _1/scaleFactor);

  if(debug_x)
    RawFileUtility::write(outBegin, outEnd,
                          "OutputData/TxSymbol");
}

OfdmModulatorComponent::CplxVecIt
OfdmModulatorComponent::copyWithCp(CplxVecIt inBegin, CplxVecIt inEnd,
                                   CplxVecIt outBegin, CplxVecIt outEnd)
{
  if(outEnd-outBegin < (inEnd-inBegin)+cyclicPrefixLength_x)
    throw IrisException("Insufficient storage provided for copyWithCp output.");

  CplxVecIt it = copy(inEnd-cyclicPrefixLength_x, inEnd, outBegin);
  copy(inBegin, inEnd, it);
  return outEnd;
}

} // namesapce phy
} // namespace iris
