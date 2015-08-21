/**
 * \file components/gpp/phy/SignalScaler/SignalScalerComponent.h
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
 * The SignalScalerComponent scales a signal by a given factor or
 * to a given maximum value.
 */

#ifndef PHY_SIGNALSCALERCOMPONENT_H_
#define PHY_SIGNALSCALERCOMPONENT_H_

#include <irisapi/PhyComponent.h>

namespace iris
{
namespace phy
{

/** The SignalScalerComponent scales a signal by a
 *  given factor or to a given maximum value.
 */
class SignalScalerComponent
  : public PhyComponent
{
 public:
  SignalScalerComponent(std::string name);
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();

 private:
  float maximum_x;  ///< Maximum value to scale to (only used if x_factor = 0)
  float factor_x;   ///< Scale input with this value (0 means max is applied)
  int maxSamples_x; ///< How many samples to check for maxVal (0 means until end)
};

} // namespace phy
} // namespace iris

#endif // PHY_SIGNALSCALERCOMPONENT_H_
