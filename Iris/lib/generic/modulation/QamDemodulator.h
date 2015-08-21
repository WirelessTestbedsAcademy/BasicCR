/**
 * \file lib/generic/modulation/QamDemodulator.h
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
 * A Quadrature Amplitude Demodulation class. Objects of this class
 * can be used to demodulate using M-ary QAM with a constellation on
 * a rectangular lattice. Expects constellations which are Gray coded
 * with average unit energy.
 */

#ifndef MOD_QAMDEMODULATOR_H_
#define MOD_QAMDEMODULATOR_H_

#include <complex>
#include <vector>

#include "irisapi/Exceptions.h"
#include "irisapi/TypeInfo.h"
#include "irisapi/Logging.h"
#include "math/MathDefines.h"

namespace iris
{

/** A Quadrature Amplitude Demodulation class.
 *
 * Objects of this class provide M-ary QAM demodulation. Expects constellations
 * which are Gray coded with average unit energy.
 */
class QamDemodulator
{
 public:
  typedef std::vector<uint8_t>  Uint8Vec;
  typedef std::complex<float>   Cplx;
  typedef std::vector<Cplx>     CplxVec;

  QamDemodulator()
  {
    createQam16Lut();
  }

  /** Demodulate a set of QAM complex<float> symbols to uint8_t bytes.
   * Defaults to BPSK.
   *
   * @param inBegin   Iterator to first input QAM symbol.
   * @param inEnd     Iterator to one past last input QAM symbol.
   * @param outBegin  Iterator to first output byte.
   * @param outEnd    Iterator to one past last output byte.
   * @param M         Modulation depth (1=BPSK, 2=QPSK, 4=QAM16)
   */
  template <class InputInterator, class OutputIterator>
  OutputIterator demodulate(InputInterator inBegin,
                            InputInterator inEnd,
                            OutputIterator outBegin,
                            OutputIterator outEnd,
                            unsigned int M)
  {
    // Check for sufficient output size
    if((outEnd-outBegin)*8/M < inEnd-inBegin)
      throw IrisException("Insufficient storage provided for demodulate output.");

    if((outEnd-outBegin)*8/M > inEnd-inBegin)
      LOG(LWARNING) << "Output size larger than required for demodulate.";

    int count=0;
    int symIndex,pointsPerQuadrant;
    Cplx tempV;
    float biasValues[2]={2.0f/sqrtf(10.0f),0.0f};

    switch (M)
    {
      case QPSK: //QPSK
        for(;inBegin!=inEnd;inBegin++)
        {
          if(count%4 == 0 && count!=0)
            outBegin++;
          //slice to demodulate
          if((*inBegin).real() > 0)
            if((*inBegin).imag() > 0)
              *outBegin = *outBegin<<2 | 0x3;
            else
              *outBegin = *outBegin<<2 | 0x2;
          else
            if((*inBegin).imag() > 0)
              *outBegin = *outBegin<<2 | 0x1;
            else
              *outBegin = *outBegin<<2 | 0x0;
          count++;
        }
        break;
      case QAM16: //16 QAM
        for(;inBegin!=inEnd;inBegin++)
        {
          if(count%2 == 0 && count!=0)
            outBegin++;

          symIndex = 0;
          pointsPerQuadrant = 4;
          tempV = *inBegin;
          for(int j=0;j<2;j++)
          {
            if (tempV.real() > 0)
            {
              if (tempV.imag() > 0)
              {
                tempV = tempV + Cplx(-biasValues[j],-biasValues[j]);
              }
              else
              {
                tempV = tempV + Cplx(-biasValues[j],biasValues[j]);
                symIndex += 3*pointsPerQuadrant;
              }
            }
            else
            {
              if (tempV.imag() > 0)
              {
                tempV = tempV + Cplx(biasValues[j],-biasValues[j]);
                symIndex += pointsPerQuadrant;
              }
              else
              {
                tempV = tempV + Cplx(biasValues[j],biasValues[j]);
                symIndex += 2*pointsPerQuadrant;
              }
            }
            pointsPerQuadrant = pointsPerQuadrant >> 2;
          }
          *outBegin = *outBegin<<4 | Qam16Lut_[symIndex];
          count++;
        }
        break;
      default : //BPSK
        for(;inBegin!=inEnd;inBegin++)
        {
          if(count%8 == 0 && count!=0)
            outBegin++;
          (*inBegin).real() > 0 ? *outBegin = *outBegin<<1 | 0: *outBegin = *outBegin<<1 | 1;
          count++;
        }
        break;
    }

    return outBegin;
  }

  /// Convenience function for logging.
  std::string getName(){ return "QamDemodulator"; }


 private:

  void createQam16Lut()
  {
    using namespace std;
    Qam16Lut_.push_back(15);
    Qam16Lut_.push_back(13);
    Qam16Lut_.push_back(12);
    Qam16Lut_.push_back(14);

    Qam16Lut_.push_back(5);
    Qam16Lut_.push_back(7);
    Qam16Lut_.push_back(6);
    Qam16Lut_.push_back(4);

    Qam16Lut_.push_back(0);
    Qam16Lut_.push_back(2);
    Qam16Lut_.push_back(3);
    Qam16Lut_.push_back(1);

    Qam16Lut_.push_back(10);
    Qam16Lut_.push_back(8);
    Qam16Lut_.push_back(9);
    Qam16Lut_.push_back(11);
  }

  Uint8Vec Qam16Lut_;
};

} // namespace iris

#endif // MOD_QAMDEMODULATOR_H_
