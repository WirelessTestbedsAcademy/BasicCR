/**
 * \file FirFilter.h
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
 * FIR Filter classes.
 */

#ifndef _FIRFILTER_H_
#define _FIRFILTER_H_

#include <vector>
#include <deque>
#include <algorithm>

#include <boost/lambda/lambda.hpp>

namespace iris
{

//! Filters input data by applying an FIR filter with the 
//! given coefficients
template<class InT, class CoeffT = InT, class OutT = InT>
class FirFilter
{
public:
  //! default constructor - no coefficients are set
  FirFilter()
  {
  }

  //! constructor setting the filter coefficients
  template<class It>
  FirFilter(It coeff_start, It coeff_end) :
    coeffs_(coeff_start, coeff_end), sums_(coeffs_.size() + 1)
  {
  }

  //! set the filter coefficients
  template<class It>
  void setCoeffs(It coeff_start, It coeff_end)
  {
    coeffs_.clear();
    coeffs_.assign(coeff_start, coeff_end);
    sums_.clear();
    sums_.resize(coeffs_.size() + 1);
  }

  //! \brief Apply filter to given input sequence, writing output to output iterator.
  //! Make sure that output has enough capacity to hold input.size() values.
  //! \return Iterator pointing to one past the end of the output sequence.
  template<class InIt, class OutIt>
  OutIt filter(InIt istart, InIt iend, OutIt ostart)
  {
    while (istart != iend)
    {
      // multiply with each coefficient & add
      std::transform(coeffs_.begin(), coeffs_.end(), ++sums_.begin(), sums_.begin(),
          *istart++ * boost::lambda::_1 + boost::lambda::_2);
      *ostart++ = sums_.front();
    }
    return ostart;
  }
private:
  std::vector<CoeffT> coeffs_; //!< coefficient vector
  std::vector<OutT> sums_; //!< keeps sums in a delay line
};

//! \brief Upsampling & interpolating filter.
//! Upsamples input data and applies filter to the upsampled data
//! avoiding all the multiply by zero operations (filter works at the input
//! rate)
template<class InT, class CoeffT = InT, class OutT = InT>
class FirFilterUpsamp
{
public:
  //! Default constructor -- no coefficients are set
  FirFilterUpsamp()
  {
  }

  //! Constructor initialising the coefficients.
  //! Also sets the upsampling factor
  template<class It>
  FirFilterUpsamp(unsigned factor, It start, It end) :
    coeffs_(start, end), delayLine_(coeffs_.size()), factor_(factor)
  {
  }

  //! initialises the coefficients
  template<class It>
  void setCoeffs(It start, It end)
  {
    coeffs_.clear();
    coeffs_.assign(start, end);
    delayLine_.clear();
    delayLine_.resize(coeffs_.size());
  }

  std::vector<CoeffT> getCoeffs() { return coeffs_; }

  //! set the upsampling factor of the filter
  void setUpsamplingFactor(unsigned factor)
  {
    factor_ = factor;
  }

  //! \brief Apply filter to given input sequence, writing output to output iterator.
  //! Make sure that output has enough capacity to hold input.size() * factor values.
  //! \return Iterator pointing to one past the end of the output sequence.
  template<class InIt, class OutIt>
  OutIt filter(InIt istart, InIt iend, OutIt ostart)
  {
    while (istart != iend)
    {
      delayLine_.pop_back();
      delayLine_.push_front(*istart++);

      OutIt oend = ostart;
      advance(oend, factor_);
      coeff_iterator coeff_start = coeffs_.begin();
      coeff_iterator cend = coeffs_.end();
      while (ostart != oend)
      {
        *ostart = OutT();
        delay_iterator di = delayLine_.begin();
        coeff_iterator ci = coeff_start++;
        while (ci < cend)
        {
          *ostart += *ci * *di++;
          ci += factor_;
        }
        ostart++;
      }
    }
    return ostart;
  }

private:
  std::vector<CoeffT> coeffs_; //!< vector holding the coefficients
  std::deque<InT> delayLine_; //!< vector holding the delay line
  unsigned factor_; //!< upsampling factor

  //! iterator type for the coefficients vector
  typedef typename std::vector<CoeffT>::iterator coeff_iterator;
  //! iterator type for the delay line deque
  typedef typename std::deque<InT>::iterator delay_iterator;
};

} // end of iris namespace

#endif

