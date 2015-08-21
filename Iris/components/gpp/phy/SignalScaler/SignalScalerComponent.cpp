/**
 * \file components/gpp/phy/SignalScaler/SignalScalerComponent.cpp
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
 * Implementation of the SignalScaler component.
 */

#include "SignalScalerComponent.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <complex>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

using namespace std;
using namespace boost::lambda;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, SignalScalerComponent);

SignalScalerComponent::SignalScalerComponent(string name)
  : PhyComponent(name,
                 "signalscaler",
                 "A signal scaler",
                 "Paul Sutton",
                 "0.1")
{
  registerParameter(
    "maximum", "The maximum value to scale to.",
    "16384", true, maximum_x);

  registerParameter(
    "factor", "Multiply the input with this value (0 means max is applied)",
    "0", true, factor_x, Interval<float>(0, 1e32f));

  registerParameter(
    "maxsamples", "How many samples to check for maxVal (0 means until end)",
    "0", true, maxSamples_x);

}

void SignalScalerComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< complex<float> >::identifier);
  registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void SignalScalerComponent::calculateOutputTypes(
  std::map<std::string,int>& inputTypes,
  std::map<std::string,int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

void SignalScalerComponent::initialize()
{
}

void SignalScalerComponent::process()
{
  DataSet<complex<float> >* readDataSet = NULL;
  DataSet<complex<float> >* writeDataSet = NULL;

  getInputDataSet("input1", readDataSet);
  size_t size = readDataSet->data.size();
  getOutputDataSet("output1", writeDataSet, size);

  writeDataSet->timeStamp = readDataSet->timeStamp;

  if (factor_x == 0)
  {
    // this is equalent to:
    // complex<float>* it = readDataSet->data.begin();
    // complex<float>* maxIt = it;
    // while (it != readDataSet->data.end())
    // {
    //   if (!(norm(*it) < norm(*maxIt))) maxIt = it;
    //   it++;
    // }
    // float maxVal = abs(*maxIt);
    //
    // * First, it finds the maximum element in the readDataSet using norm<float>(_1) < norm<float>(_2) as
    //   comparison predicate, where _1 and _2 are placeholders for the two predicate arguments (i.e., it compares
    //   the squared magnitudes of both complex numbers).
    //   - 'norm' is an STL template for computing the squared magnitude of complex numbers, parameterised with the complex
    //     element type (template <class T> norm(const complex<T>& x); )
    //   - 'bind' creates a functor given a function and an argument, here a placeholder, i.e., it creates a
    //     functor class with 'operator()(complex<float>)' which applies the norm<float> function to its argument.
    //     (see http://www.boost.org/doc/libs/1_40_0/doc/html/lambda/using_library.html#lambda.introductory_examples)
    // * Then, it computes the absolute value of that maximum (complex number)
    std::vector<complex<float> >::iterator until = readDataSet->data.end();
    if (maxSamples_x > 0) until = readDataSet->data.begin() + maxSamples_x;
    float maxVal = abs(*max_element(readDataSet->data.begin(), until,
            bind(norm<float>, _1) < bind(norm<float>, _2) ));

    // this is equivalent to:
    // complex<float>* outit = writeDataSet->data.begin();
    // for (complex<float>* it = readDataSet->data.begin(); it != readDataSet->data.end(); ++it)
    //     *outit++ = *it / maxVal * x_maximum;
    // it applies x / maxVal * x_maximum for each element x
    // in the readDataSet and writes the result into the writeDataSet (using boost::lambda)
    // (see http://www.boost.org/doc/libs/1_40_0/doc/html/lambda/using_library.html#lambda.introductory_examples)
    std::transform(readDataSet->data.begin(), readDataSet->data.end(),
            writeDataSet->data.begin(), _1 / maxVal * maximum_x);
  }
  else
  {
    // this applies x * x_factor for each element x in the readDataSet and writes the result to the
    // writeDataSet
    std::transform(readDataSet->data.begin(), readDataSet->data.end(), writeDataSet->data.begin(),
            _1 * factor_x);
  }

  releaseInputDataSet("input1", readDataSet);
  releaseOutputDataSet("output1", writeDataSet);
}

} // namespace phy
} // namespace iris
