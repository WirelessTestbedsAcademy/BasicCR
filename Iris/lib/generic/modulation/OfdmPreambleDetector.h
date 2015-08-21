/**
 * \file lib/generic/modulation/OfdmPreambleDetector.h
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
 * An OFDM preamble detector. Detects preambles with half-symbol repetitions
 * using Schmidl & Cox algorithm (Schmidl, Timothy M., and Donald C. Cox.
 * "Robust frequency and timing synchronization for OFDM."
 * IEEE Transactions on communications 45.12 (1997): 1613-1621.).
 */

#ifndef MOD_OFDMPREAMBLEDETECTOR_H_
#define MOD_OFDMPREAMBLEDETECTOR_H_

#include <complex>
#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>
#include <algorithm>

#include "irisapi/Exceptions.h"
#include "irisapi/TypeInfo.h"
#include "irisapi/Logging.h"
#include "math/MathDefines.h"
#include "math/Dsp.h"
#include "utility/RawFileUtility.h"

namespace iris
{

/// Detect OFDM preambles using Schmidl & Cox algorithm.
class OfdmPreambleDetector
  : boost::noncopyable
{
public:
  typedef std::complex<float>             Cplx;
  typedef boost::circular_buffer<Cplx>    CplxBuf;
  typedef CplxBuf::iterator               CplxBufIt;
  typedef boost::circular_buffer<float>   FloatBuf;
  typedef FloatBuf::iterator              FloatBufIt;

  /** Create an OFDM preamble detector.
   *
   * @param symbolLen         The OFDM symbol length.
   * @param cyclicPrefixLen   The cyclic prefix length.
   * @param threshold         The detector threshold (in range [0,1]).
   */
  OfdmPreambleDetector(int symbolLen = 256,
                       int cyclicPrefixLen = 16,
                       float threshold = 0.827,
                       bool debug = false)
    :sLen_(symbolLen)
    ,cpLen_(cyclicPrefixLen)
    ,thresh_(threshold*cyclicPrefixLen)
    ,debug_(debug)
    ,symbolBuffer_(symbolLen+cyclicPrefixLen)
    ,halfBuffer_(symbolLen/2, Cplx(0,0))
    ,eBuffer_(symbolLen, Cplx(0,0))
    ,pBuffer_(symbolLen/2, Cplx(0,0))
    ,vBuffer_(cyclicPrefixLen, 0.0)
    ,vDebugBuffer_(symbolLen, 0.0)
    ,currentP_(0,0)
    ,currentE_(0,0)
    ,vMovingAve_(0.0)
    ,lastVma_(0.0)
  {}

  /** Search for a preamble in the range [inBegin, inEnd).
   *
   * A detected preamble will be copied into the range
   * [preambleBegin, preambleEnd). If no preamble is detected,
   * the returned iterator == inEnd.
   *
   * @param inBegin         Iterator to first input signal sample.
   * @param inEnd           Iterator to one past last input sample.
   * @param preambleBegin   Iterator to first sample preamble container.
   * @param preambleEnd     Iterator to one past last sample of container.
   * @param freqOffset      Estimated frequency offset.
   * @param snr             Estimated SNR.
   * \return                Iterator to start of detected preamble or last
   *                          searched input sample.
   */
  template <class Iterator>
  Iterator search(Iterator inBegin, Iterator inEnd,
                  Iterator preambleBegin, Iterator preambleEnd,
                  bool &detected, float &freqOffset, float &snr);

  /// Reset the detector (keep current parameters).
  void reset()
  {reset(sLen_,cpLen_,thresh_/cpLen_, debug_);}

  /// Reset the detector.
  void reset(int symbolLen, int cyclicPrefixLen, float threshold, bool debug=false)
  {
    sLen_ = symbolLen;
    cpLen_ = cyclicPrefixLen;
    thresh_ = threshold*cyclicPrefixLen;
    debug_ = debug;
    symbolBuffer_.assign(sLen_+cpLen_, Cplx(0,0));
    halfBuffer_.assign(sLen_/2, Cplx(0,0));
    eBuffer_.assign(sLen_, Cplx(0,0));
    pBuffer_.assign(sLen_/2, Cplx(0,0));
    vBuffer_.assign(cpLen_, 0);
    vDebugBuffer_.assign(sLen_, 0);
    currentP_ = Cplx(0,0);
    currentE_ = Cplx(0,0);
    vMovingAve_ = 0;
  }

  /// Convenience function for logging.
  static std::string getName(){ return "OfdmPreambleDetector"; }

private:
  CplxBuf symbolBuffer_;    ///< Holds last full symbol (including CP).
  CplxBuf halfBuffer_;      ///< Holds last half-symbol.
  CplxBuf eBuffer_;         ///< Holds last symbol length of e values.
  CplxBuf pBuffer_;         ///< Holds last half-symbol length of p values.
  FloatBuf vBuffer_;        ///< Holds last CP length of v values.
  FloatBuf vDebugBuffer_;   ///< Holds last symbol length of v values for debug.

  Cplx currentP_;               ///< Current P (correlation value)
  Cplx currentE_;               ///< Current E (power value)
  float vMovingAve_, lastVma_;  ///< Moving averages of V (normalized correlation)
  float maxVal_, maxValThresh_; ///< Used for peak detection.
  int sLen_, cpLen_;            ///< Symbol length, CP length, extension length
  float thresh_;                ///< Detection threshold.
  bool debug_;                  ///< Is debugging on?

};

template <class Iterator>
Iterator OfdmPreambleDetector::search(Iterator inBegin,
                                      Iterator inEnd,
                                      Iterator preambleBegin,
                                      Iterator preambleEnd,
                                      bool &detected,
                                      float &freqOffset,
                                      float &snr)
{
  int counter=-1;

  for(; inBegin != inEnd; ++inBegin)
  {
    counter++;

    //Add current sample to symbolBuffer_
    symbolBuffer_.push_back(*inBegin);

    Cplx  nextSymbol = *inBegin;
    Cplx  midSymbol = halfBuffer_.front();

    //Add current sample to the halfBuffer_
    halfBuffer_.push_back(*inBegin);

    //Iterate for P
    Cplx  nextP = conj(nextSymbol)*midSymbol;
    Cplx  tailP = pBuffer_.front();
    currentP_ += (nextP - tailP);
    pBuffer_.push_back(nextP);

    //Iterate for E
    Cplx  nextE = conj(nextSymbol)*nextSymbol;
    Cplx  tailE = eBuffer_.front();
    currentE_ += (nextE - tailE);
    eBuffer_.push_back(nextE);

    //Use P and E values to calculate V
    //float magP = abs(currentP_);
    //float magE = abs(currentE_);
    float magP = fastMag(currentP_);
    float magE = fastMag(currentE_);
    float den = magE*magE;
    float v = (den == 0) ? 0 : ((2*magP)*(2*magP))/den;

    //Estimate the SNR
    snr = 10*log10(sqrtf(v)/(1-sqrtf(v)));

    //Iterate for the moving average value of V
    lastVma_ = vMovingAve_;
    vMovingAve_ += (v - vBuffer_.front());
    vBuffer_.push_back(v);
    vDebugBuffer_.push_back(v);

    //Check the moving average V value for threshold and peak
    if(vMovingAve_ > thresh_ && vMovingAve_ < lastVma_)
    {
      detected = true;

      if(debug_)
          RawFileUtility::write(vDebugBuffer_.begin(), vDebugBuffer_.end(),
                                "OutputData/RxFrameDetectVArray");

      //We've detected the peak - copy the preamble into output vector
      if((preambleEnd-preambleBegin) < sLen_+cpLen_)
        throw IrisException("Insufficient storage provided for preamble output");
      std::copy(symbolBuffer_.begin(), symbolBuffer_.end(), preambleBegin);

      //Phase of P gives the fractional frequency offset
      freqOffset = arg(currentP_);
      freqOffset = -(freqOffset)/(float)IRIS_PI;

      reset();
      return ++inBegin;
    }
  }

  return inBegin;
}

} // namespace iris

#endif // MOD_OFDMPREAMBLEDETECTOR_H_
