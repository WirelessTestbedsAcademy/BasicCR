/**
 * \file components/gpp/phy/OfdmDemodulator/OfdmDemodulatorComponent.h
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
 * An OFDM demodulation component. Takes blocks of received complex<float>
 * data and detects and demodulates OFDM frames. Received data is output
 * in blocks of uint8_t bytes. There is one output block for each received
 * frame. The demodulator expects frames with the following structure:     <br>                                           <br>
 *             -----------------------------------                         <br>
 *             | Preamble | Header | Data ...... |                         <br>
 *             -----------------------------------                         <br>
 *
 * The default parameter values expect the following spectral shape:
 *
 *  _______________||||||||||||||||____||||||||||||||||________________    <br>
 *  <  23 guards  >< 100 carriers ><DC>< 100 carriers ><  22 guards  >     <br>
 *
 * This corresponds to the default shape generated with the OfdmModulator
 * component when receiving with bandwidth X/2 where X is the transmit
 * bandwidth. See OfdmModulatorComponent.h for more information.
 */

#ifndef PHY_OFDMDEMODULATORCOMPONENT_H_
#define PHY_OFDMDEMODULATORCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "fftw3.h"

#include "irisapi/PhyComponent.h"
#include "modulation/OfdmPreambleDetector.h"
#include "modulation/ToneGenerator.h"
#include "modulation/QamDemodulator.h"
#include "modulation/OfdmPreambleGenerator.h"
#include "math/MathDefines.h"

namespace iris
{
namespace phy
{

/** An OFDM demodulation component. Takes a block of samples in
 * complex<float> format and outputs a block of uint8_t bytes each
 * time an OFDM frame is demodulated. The demodulator expects frames
 * with the following structure:                                           <br>
 *             -----------------------------------                         <br>
 *             | Preamble | Header | Data ...... |                         <br>
 *             -----------------------------------                         <br>
 */
class OfdmDemodulatorComponent
  : public PhyComponent
{
public:
  typedef std::vector<uint8_t>  ByteVec;
  typedef ByteVec::iterator     ByteVecIt;
  typedef std::vector<int>      IntVec;
  typedef IntVec::iterator      IntVecIt;
  typedef std::vector<float>    FloatVec;
  typedef FloatVec::iterator    FloatVecIt;
  typedef std::complex<float>   Cplx;
  typedef std::vector<Cplx>     CplxVec;
  typedef CplxVec::iterator     CplxVecIt;

  OfdmDemodulatorComponent(std::string name);
  ~OfdmDemodulatorComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

private:
  void setup();
  void destroy();
  CplxVecIt searchInput(CplxVecIt begin, CplxVecIt end);
  CplxVecIt processFrame(CplxVecIt begin, CplxVecIt end);
  void extractPreamble();
  void extractHeader();
  void demodFrame();
  void demodSymbol(CplxVecIt inBegin, CplxVecIt inEnd,
                   ByteVecIt outBegin, ByteVecIt outEnd,
                   int modulationDepth);
  void generateFractionalOffsetCorrector(float offset);
  void correctFractionalOffset(CplxVecIt begin, CplxVecIt end);
  int findIntegerOffset(CplxVecIt begin, CplxVecIt end);
  void generateEqualizer(CplxVecIt begin, CplxVecIt end);
  void equalizeSymbol(CplxVecIt begin, CplxVecIt end);

  struct opAbs{float operator()(Cplx i) const{return abs(i);};};

  bool debug_x;               ///< Debug flag
  int reportRate_x;           ///< Report performance every reportRate_x frames
  int numDataCarriers_x;      ///< Data subcarriers (default = 192)
  int numPilotCarriers_x;     ///< Pilot subcarriers (default = 8)
  int numGuardCarriers_x;     ///< Guard subcarriers (default = 55)
  int cyclicPrefixLength_x;   ///< Length of cyclic prefix (default = 16)
  float threshold_x;          ///< Frame detection threshold (default = 0.827)

  int symbolLength_;          ///< Length of each OFDM symbol including prefix.
  int numBins_;               ///< Number of bins for our FFT.
  const int numHeaderBytes_;  ///< Number of bytes used for header.
  int numHeaderSymbols_;      ///< Number of header symbols in this frame.
  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  bool frameDetected_;        ///< Have we detected a frame?
  bool haveHeader_;           ///< Have we extracted the header?
  int headerIndex_;           ///< Index into container for header symbols.
  int frameIndex_;            ///< Index into container for frame symbols.
  float fracFreqOffset_;      ///< Fractional frequency offset of current frame.
  int intFreqOffset_;         ///< Integer frequency offset of current frame.
  uint32_t rxCrc_;            ///< Received framecheck.
  uint16_t rxNumBytes_;       ///< Number of bytes of data in received frame.
  uint8_t rxModulation_;      ///< Modulation depth of received frame.
  int rxNumSymbols_;          ///< Number of OFDM symbols in received frame.
  int numRxFrames_;           ///< Count of total detected frames.
  int numRxFails_;            ///< Count of frames we failed to demod.
  int symbolCount_;           ///< Index of symbol in current frame.

  DataSet< Cplx >* in_;       ///< Pointer to an input DataSet.
  IntVec pilotIndices_;       ///< Indices for our pilot carriers.
  IntVec dataIndices_;        ///< Indices for our data carriers.
  CplxVec preamble_;          ///< Contains our known frame preamble.
  CplxVec preambleBins_;      ///< Contains bins of our known preamble.
  CplxVec pilotSequence_;     ///< Contains our known pilot symbols.
  CplxVec rxPreamble_;        ///< Container for received preamble.
  CplxVec rxHeader_;          ///< Container for received header.
  CplxVec rxFrame_;           ///< Container for received frame.
  CplxVec equalizer_;         ///< The equalizer for the current frame.
  CplxVec corrector_;         ///< Fractional frequency offset corrector.
  ByteVec frameData_;         ///< Container for received frame data.

  Cplx* halfFftData_;         ///< Input/output array for half-length fft
  fftwf_plan halfFft_;        ///< Half-length fft plan
  Cplx* fullFftData_;         ///< Input/output array for full-length fft
  fftwf_plan fullFft_;        ///< Full-length fft plan

  OfdmPreambleDetector detector_;       ///< Our preamble detector.
  ToneGenerator toneGenerator_;         ///< Our tone generator.
  QamDemodulator qDemod_;               ///< Our QAM demodulator.
  OfdmPreambleGenerator preambleGen_;   ///< Our preamble generator.

  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }
};

} // namespace phy
} // namespace iris

#endif // PHY_OFDMDEMODULATORCOMPONENT_H_
