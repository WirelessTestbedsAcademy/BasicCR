/**
 * \file components/gpp/phy/OfdmModulator/OfdmModulatorComponent.h
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
 * An OFDM modulation component. Takes a block of data bytes in uint8_t
 * format and outputs a full OFDM frame of complex<float> symbols with
 * the following structure:                                                <br>
 *             -----------------------------------                         <br>
 *             | Preamble | Header | Data ...... |                         <br>
 *             -----------------------------------                         <br>
 *
 * The default parameter values give the following spectral shape:
 *
 *  ________________||||||||||||||||____||||||||||||||||________________   <br>
 *  <  156 guards  >< 100 carriers ><DC>< 100 carriers ><  155 guards  >   <br>
 *
 * This default shape is designed with a large number of guard subcarriers
 * to account for the roll-off of the half-band filters in the USRP front-ends.
 * When transmitted with bandwidth X, this default waveform can be demodulated
 * by receiving bandwidth X/2 and using the OfdmDemodulator component with its
 * default parameter values.
 */

#ifndef PHY_OFDMMODULATORCOMPONENT_H_
#define PHY_OFDMMODULATORCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "fftw3.h"
#include "modulation/QamModulator.h"
#include "modulation/OfdmPreambleGenerator.h"
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** An OFDM modulation component. Takes a block of data bytes in uint8_t
 * format and outputs a full OFDM frame of complex<float> symbols with
 * the following structure:                                                <br>
 *             -----------------------------------                         <br>
 *             | Preamble | Header | Data ...... |                         <br>
 *             -----------------------------------                         <br>
 */
class OfdmModulatorComponent
  : public PhyComponent
{
 public:
  typedef std::complex<float>   Cplx;
  typedef std::vector<Cplx>     CplxVec;
  typedef CplxVec::iterator     CplxVecIt;
  typedef std::vector<int>      IntVec;
  typedef IntVec::iterator      IntVecIt;
  typedef std::vector<uint8_t>  ByteVec;
  typedef ByteVec::iterator     ByteVecIt;

  OfdmModulatorComponent(std::string name);
  ~OfdmModulatorComponent();
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
  void createHeader(ByteVecIt begin, ByteVecIt end);
  void createFrame(ByteVecIt begin, ByteVecIt end);
  void createSymbol(CplxVecIt inBegin, CplxVecIt inEnd,
                    CplxVecIt outBegin, CplxVecIt outEnd);
  CplxVecIt copyWithCp(CplxVecIt inBegin, CplxVecIt inEnd,
                       CplxVecIt outBegin, CplxVecIt outEnd);

  bool debug_x;               ///< Debug flag
  int numDataCarriers_x;      ///< Data subcarriers (default = 192)
  int numPilotCarriers_x;     ///< Pilot subcarriers (default = 8)
  int numGuardCarriers_x;     ///< Guard subcarriers (default = 55+256)
  int modulationDepth_x;      ///< 1=BPSK, 2=QPSK, 4=QAM16 (default = 1)
  int cyclicPrefixLength_x;   ///< Length of cyclic prefix (default = 32)
  int maxSymbolsPerFrame_x;   ///< Max OFDM data symbols per frame (default = 32)

  int numBins_;               ///< Number of bins for our FFT.
  int bytesPerSymbol_;        ///< Bytes per OFDM symbol.
  const int numHeaderBytes_;  ///< Number of bytes in our frame header (7).
  int numHeaderSymbols_;
  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame

  IntVec pilotIndices_;       ///< Indices for our pilot carriers.
  IntVec dataIndices_;        ///< Indices for our data carriers.
  ByteVec header_;            ///< Contains the header data for each frame.
  Cplx* fftBins_;             ///< Allocated using fftwf_malloc (SIMD aligned)
  CplxVec preamble_;          ///< Contains our frame preamble.
  CplxVec pilotSequence_;     ///< Contains our pilot symbols.
  CplxVec modHeader_;         ///< Contains our modulated header data.
  CplxVec modData_;           ///< Contains our modulated data.
  ByteVec pad_;               ///< Padding data.
  CplxVec modPad_;            ///< Used to pad out the last symbol, if required.
  CplxVec symbol_;            ///< Contains a single OFDM symbol.

  fftwf_plan fft_;                      ///< Our FFT object pointer.
  QamModulator qMod_;                   ///< Our QAM modulator.
  OfdmPreambleGenerator preambleGen_;   ///< Our preamble generator.

  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_OFDMMODULATORCOMPONENT_H_
