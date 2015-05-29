/**
 * \file components/gpp/phy/LiquidOfdmDemod/LiquidOfdmDemodComponent.h
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
 * An OFDM demodulation component that uses liquid-dsp to do the hard
 * work.
 *
 */

#ifndef LIQUIDOFDMDEMODCOMPONENT_H_
#define LIQUIDOFDMDEMODCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <liquid/liquid.h>
#include <vector>

namespace iris
{
class LiquidOfdmDemodComponent: public PhyComponent
{
private:
    //Exposed parameters
    unsigned int noSubcarriers_x;
    unsigned int cyclicPrefixLength_x;
    unsigned int taperLength_x;
    bool debug_x;

    //local variables
    ofdmflexframesync frameSyncronizer_;
    uint32_t totalFrames_;
    uint32_t errorFrames_;
    double timeStamp_;
    double sampleRate_;

public:
    LiquidOfdmDemodComponent(std::string name);
    virtual ~LiquidOfdmDemodComponent();
    virtual void calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes);
    virtual void registerPorts();
    virtual void initialize();
    virtual void parameterHasChanged(std::string name);
    virtual void process();
    void callback( unsigned char *  _header,
                   int              _header_valid,
                   unsigned char *  _payload,
                   unsigned int     _payload_len,
                   int              _payload_valid,
                   framesyncstats_s _stats,
                   void *           _userdata);
};

namespace liquidofdmdemoddetail
{
int gCallback( unsigned char *  _header,
               int              _header_valid,
               unsigned char *  _payload,
               unsigned int     _payload_len,
               int              _payload_valid,
               framesyncstats_s _stats,
               void *           _userdata);
} // namespace liquidofdmdemoddetail

} /* namespace iris */

#endif /* LIQUIDOFDMDEMODCOMPONENT_H_ */
