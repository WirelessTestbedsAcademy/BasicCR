/**
 * \file components/gpp/phy/Spectrogram/SpectrogramComponent.h
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
 * Performs a spectral periodogram to estimate the power spectral
 * density in dB of a signal over time. Uses the spgram functions from
 * the liquid DSP library (see liquidsdr.org). FFT windows are shaped
 * with a Kaiser Bessel window. This component can act as a probe, simply
 * passing signal data through untouched and providing PSD data via events.
 * This component can also act as a data sink, having no output. The
 * default setting is for this component to provide PSD estimates on
 * an output port.
 */

#ifndef PHY_SPECTROGRAMCOMPONENT_H_
#define PHY_SPECTROGRAMCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include "liquid/liquid.h"

namespace iris
{
namespace phy
{

/** Performs a spectral periodogram to estimate the power spectral
 * density in dB.
 */
class SpectrogramComponent
  : public PhyComponent
{
 public:
  typedef std::complex<float>   Cplx;
  typedef std::vector<Cplx>     CplxVec;
  typedef CplxVec::iterator     CplxVecIt;
  typedef std::vector<float>    FloatVec;
  typedef FloatVec::iterator    FloatVecIt;

  SpectrogramComponent(std::string name);
  ~SpectrogramComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();

 private:
  void processWindow();
  void outputPsd();

  int nFft_x;             ///< FFT length.
  int windowLength_x;     ///< Length of windows used for spectrogram.
  int delay_x;            ///< Delay between windows.
  int nWindows_x;         ///< Number of windows to average over.
  float beta_x;           ///< Kaiser-Bessel window parameter (beta_ > 0).
  bool isProbe_x;         ///< Act as a probe? (Provide PSD estimates via events).
  bool isSink_x;          ///< Act as a sink? (Has no output).

  spgram sp_;       ///< Our spectrogram struct
  CplxVec window_;
  CplxVec spec_;
  FloatVec psd_;
  int n_;
};

} // namespace phy
} // namespace iris

#endif // PHY_SPECTROGRAMCOMPONENT_H_
