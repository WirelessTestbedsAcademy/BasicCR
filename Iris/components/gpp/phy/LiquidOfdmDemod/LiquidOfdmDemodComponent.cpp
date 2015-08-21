/**
 * \file components/gpp/phy/LiquidOfdmDemod/LiquidOfdmDemodComponent.cpp
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

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "LiquidOfdmDemodComponent.h"
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>

using namespace std;

namespace iris
{

namespace liquidofdmdemoddetail
{
int gCallback( unsigned char *  _header,
               int              _header_valid,
               unsigned char *  _payload,
               unsigned int     _payload_len,
               int              _payload_valid,
               framesyncstats_s _stats,
               void *           _userdata)
{
  static_cast<LiquidOfdmDemodComponent*>(_userdata)->callback(_header,
     _header_valid,
     _payload,
     _payload_len,
     _payload_valid,
     _stats,
     _userdata);
}
} // namespace liquidofdmdemoddetail

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, LiquidOfdmDemodComponent);


LiquidOfdmDemodComponent::LiquidOfdmDemodComponent(string name):
    PhyComponent(name, "liquidofdmdemodphycomponent", "A OFDM demodulation component using liquid-dsp.", "Andre Puschmann", "0.1")
    ,frameSyncronizer_(0)
    ,totalFrames_(0)
    ,errorFrames_(0)
    ,timeStamp_(0)
    ,sampleRate_(0)
{
    //Format: registerParameter(name, description, default, dynamic?, parameter)
    registerParameter("debug", "Whether to output debug information",
        "false", true, debug_x);

    registerParameter("subcarriers", "Number of subcarriers",
        "64", true, noSubcarriers_x);

    registerParameter("prefixlength", "Cyclic prefix length",
        "16", true, cyclicPrefixLength_x);

    registerParameter("taperlength", "Taper length",
        "4", true, taperLength_x);
}


LiquidOfdmDemodComponent::~LiquidOfdmDemodComponent()
{
    try {
        if (frameSyncronizer_)
            ofdmflexframesync_destroy(frameSyncronizer_);
    }
    catch(...)
    {
        LOG(LERROR) << "Unexpected exception caught in destructor";
    }

    LOG(LINFO) << "Total frames received: " << totalFrames_;
    LOG(LINFO) << "Corrupted frames: " << errorFrames_;
    if (totalFrames_ > 0) {
        float errorRate = static_cast<float>(errorFrames_) / static_cast<float>(totalFrames_);
        LOG(LINFO) << boost::str(boost::format("Frame error rate: %.2f") % errorRate);
    }
}


void LiquidOfdmDemodComponent::registerPorts()
{
    registerInputPort("input1", TypeInfo< complex<float> >::identifier);
    registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}


void LiquidOfdmDemodComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
    outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}


void LiquidOfdmDemodComponent::initialize()
{
    // create frame synchronizer object, pass this pointer as callback parameter
    frameSyncronizer_ = ofdmflexframesync_create(noSubcarriers_x,
                                                  cyclicPrefixLength_x,
                                                  taperLength_x,
                                                  NULL,
                                                  liquidofdmdemoddetail::gCallback,
                                                  this);
    ofdmflexframesync_print(frameSyncronizer_);
}

void LiquidOfdmDemodComponent::parameterHasChanged(std::string name)
{
    if (name == "subcarriers" ||
        name == "prefixlength" ||
        name == "taperlength")
    {
      //Need to destroy and recreate the synchronizer object
      if (frameSyncronizer_)
        ofdmflexframesync_destroy(frameSyncronizer_);
      initialize();
    }
}


//! the callback function invoked by liquid upon frame reception
void LiquidOfdmDemodComponent::callback(unsigned char * _header,
                                        int _header_valid,
                                        unsigned char * _payload,
                                        unsigned int _payload_len,
                                        int _payload_valid,
                                        framesyncstats_s _stats,
                                        void * _userdata)
{
    totalFrames_++;
    if (_header_valid) {
        if (debug_x) {
            LOG(LDEBUG) << "Frame received: rssi = " << _stats.rssi
                        << " dB, evm = " << _stats.evm
                        << " dB, cfo = " << _stats.cfo;
            LOG(LDEBUG) << "Header: " << (_header_valid ? "valid" : "INVALID")
                        << ", Payload: " << (_payload_valid ? "valid" : "INVALID");
        }

        // create output data set if frame was received successfully
        if (_header_valid && _payload_valid) {
            DataSet< uint8_t>* out;
            getOutputDataSet("output1", out, _payload_len);
            out->sampleRate = sampleRate_;
            out->timeStamp = timeStamp_;
            std::copy(_payload, _payload + _payload_len, out->data.begin());
            releaseOutputDataSet("output1", out);
        }
        else {
            errorFrames_++;
            LOG(LDEBUG) << "Frame invalid";
        }
    }
}


void LiquidOfdmDemodComponent::process()
{
    DataSet< std::complex<float> >* in = NULL;
    getInputDataSet("input1", in);
    timeStamp_ = in->timeStamp;
    sampleRate_ = in->sampleRate;
    ofdmflexframesync_execute(frameSyncronizer_, &in->data[0], in->data.size());
    releaseInputDataSet("input1", in);
}

} /* namespace iris */
