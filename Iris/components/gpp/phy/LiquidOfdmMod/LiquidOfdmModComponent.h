/**
 * \file components/gpp/phy/LiquidOfdmMod/LiquidOfdmModComponent.cpp
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
 * An OFDM modulation component that uses liquid-dsp to do the hard
 * work.
 *
 */

#ifndef LIQUIDOFDMMODCOMPONENT_H_
#define LIQUIDOFDMMODCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <liquid/liquid.h>
#include <vector>

namespace iris
{
class LiquidOfdmModComponent: public PhyComponent
{
private:
    //Exposed parameters
    bool debug_x;
    unsigned int noSubcarriers_x;
    unsigned int cyclicPrefixLength_x;
    unsigned int taperLength_x;
    std::string modulationScheme_x;
    std::string fecZero_x;
    std::string fecOne_x;
    std::string crcScheme_x;
    unsigned int numSymbolsS0_x;
    std::string frameHeader_x;

    //Variables
    ofdmflexframegen frameGenerator_; // frame generator object

public:
    LiquidOfdmModComponent(std::string name);
    virtual ~LiquidOfdmModComponent();
    virtual void calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes);
    virtual void registerPorts();
    virtual void initialize();
    virtual void parameterHasChanged(std::string name);
    virtual void process();
};

} /* namespace iris */

#endif /* LIQUIDOFDMMODCOMPONENT_H_ */
