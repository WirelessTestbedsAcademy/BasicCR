/**
 * \file components/gpp/phy/PfbSynthesizer/PfbSynthesizerComponent.h
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
 * A polyphase filterbank synthesizer using liquid-dsp. For more
 * info see https://github.com/jgaeddert/liquid-dsp.
 */

#ifndef PHY_PFBSYNTHESIZERCOMPONENT_H_
#define PHY_PFBSYNTHESIZERCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include "liquid/liquid.h"

namespace iris
{
namespace phy
{

/** A polyphase filterbank synthesizer using liquid-dsp. For more
 * info see https://github.com/jgaeddert/liquid-dsp.
 */
class PfbSynthesizerComponent
  : public PhyComponent
{
 public:
  typedef std::vector<float>    FloatVec;
  typedef FloatVec::iterator    FloatVecIt;
  typedef std::complex<float>   Cplx;
  typedef std::vector<Cplx>     CplxVec;
  typedef CplxVec::iterator     CplxVecIt;

  PfbSynthesizerComponent(std::string name);
  ~PfbSynthesizerComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();

 private:
  bool debug_x;                 ///< Running in debug mode?
  int nChans_x;                 ///< Number of channels

  FloatVec taps;                ///< Our prototype filter taps
  CplxVec buf;                  ///< Our input buffer
  CplxVecIt bufIt;              ///< Iterator into our input buffer
  CplxVec outBuf;               ///< Our output buffer
  firpfbch_crcf channelizer;    ///< Ptr to our channelizer struct
  nco_crcf nco;                 ///< frequency-centering NCO

  void printTapsForMatlab();
};

} // namespace phy
} // namespace iris

#endif // PHY_PFBSYNTHESIZERCOMPONENT_H_
