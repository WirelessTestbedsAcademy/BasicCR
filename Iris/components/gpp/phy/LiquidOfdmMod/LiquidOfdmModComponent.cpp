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
 * [1] http://liquidsdr.org/downloads/liquid.pdf
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "LiquidOfdmModComponent.h"
#include <algorithm>

using namespace std;

namespace iris
{
// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, LiquidOfdmModComponent);

LiquidOfdmModComponent::LiquidOfdmModComponent(string name):
    PhyComponent(name, "liquidofdmmodphycomponent", "A OFDM modulation component using libliquid.", "Andre Puschmann", "0.1")
  ,noSubcarriers_x(0)
  ,cyclicPrefixLength_x(0)
  ,taperLength_x(0)
  ,frameGenerator_(0)
{
    //Format: registerParameter(name, description, default, dynamic?, parameter)
    registerParameter("debug", "Whether to write debug information to file", 
        "false", true, debug_x);

    registerParameter("subcarriers", "Number of subcarriers", 
        "64", true, noSubcarriers_x);

    registerParameter("prefixlength", "Cyclic prefix length", 
        "16", true, cyclicPrefixLength_x);

    registerParameter("taperlength", "Taper length",
        "4", true, taperLength_x);

    // For a list of all availble modulation, FEC and CRC modes, please see the liquid-dsp manual [1]
    registerParameter("modulation", "Modulation scheme (bpsk, qpsk, qam16, qam32, ..)",
        "qpsk", true, modulationScheme_x);

    registerParameter("fec0", "Forward Error Correction for header",
        "none", true, fecZero_x);

    registerParameter("fec1", "Forward Error Correction for data (rep3, rep5, h74, h84, and h128, ..)",
        "h128", true, fecOne_x);

    registerParameter("crc", "Cyclic redundancy check (crc16, crc32, ..)",
        "crc32", true, crcScheme_x);

    registerParameter("frameheader", "Header content, maximum lenght is eight characters",
        "default", true, frameHeader_x);
}

LiquidOfdmModComponent::~LiquidOfdmModComponent()
{
    try {
        if (frameGenerator_)
            ofdmflexframegen_destroy(frameGenerator_);
    }
    catch(...)
    {
        LOG(LERROR) << "Unexpected exception caught in destructor";
    }
}

void LiquidOfdmModComponent::registerPorts()
{
    registerInputPort("input1", TypeInfo< uint8_t >::identifier);
    registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void LiquidOfdmModComponent::calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes)
{
    outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

void LiquidOfdmModComponent::initialize()
{
    // print capabilities of liquid
    if (debug_x) {
        liquid_print_modulation_schemes();
        liquid_print_fec_schemes();
        liquid_print_crc_schemes();
    }

    // initialize subcarrier allocation
    unsigned char p[noSubcarriers_x];
    ofdmframe_init_default_sctype(noSubcarriers_x, p);

    // create frame generator properties object and initialize to default
    ofdmflexframegenprops_s fgProps;
    ofdmflexframegenprops_init_default(&fgProps);
    modulation_scheme ms = liquid_getopt_str2mod(modulationScheme_x.c_str());
    fec_scheme fec0 = liquid_getopt_str2fec(fecZero_x.c_str());
    fec_scheme fec1 = liquid_getopt_str2fec(fecOne_x.c_str());
    crc_scheme check = liquid_getopt_str2crc(crcScheme_x.c_str());

    fgProps.mod_scheme = ms;
    fgProps.fec0 = fec0;
    fgProps.fec1 = fec1;
    fgProps.check = check;

    // create frame generator object
    try {
        frameGenerator_ = ofdmflexframegen_create(noSubcarriers_x, cyclicPrefixLength_x, taperLength_x, p, &fgProps);
        ofdmflexframegen_print(frameGenerator_);
    }
    catch(...)
    {
        LOG(LERROR) << "Unexpected exception caught during frame generator generation";
    }
}

void LiquidOfdmModComponent::parameterHasChanged(std::string name)
{
    ofdmflexframegenprops_s fgProps;
    ofdmflexframegen_getprops(frameGenerator_, &fgProps);
    if (name == "modulation") {
        modulation_scheme ms = liquid_getopt_str2mod(modulationScheme_x.c_str());
        fgProps.mod_scheme = ms;
    }
    if (name == "fec0") {
        fec_scheme fec0 = liquid_getopt_str2fec(fecZero_x.c_str());
        fgProps.fec0 = fec0;
    }
    if (name == "fec1") {
        fec_scheme fec1 = liquid_getopt_str2fec(fecOne_x.c_str());
        fgProps.fec1 = fec1;
    }
    if (name == "crc") {
        crc_scheme check = liquid_getopt_str2crc(crcScheme_x.c_str());
        fgProps.check = check;
    }
    ofdmflexframegen_setprops(frameGenerator_, &fgProps);

    if (name == "subcarriers" ||
        name == "prefixlength" ||
        name == "taperlength")
    {
      //Need to destroy and recreate the frame generator
      if (frameGenerator_)
        ofdmflexframegen_destroy(frameGenerator_);
      initialize();
    }
}

void LiquidOfdmModComponent::process()
{
    DataSet< uint8_t >* in = NULL;
    getInputDataSet("input1", in);

    unsigned int payloadSize = (unsigned int)in->data.size();
    unsigned char* payload = &in->data[0];

    unsigned char header[8];
    for (int i = 0; i < 8; i++)
        header[i] = frameHeader_x.c_str()[i] & 0xff;

    ofdmflexframegen_assemble(frameGenerator_, header, payload, payloadSize);
    unsigned int symbols = ofdmflexframegen_getframelen(frameGenerator_);
    unsigned int symbolSize = noSubcarriers_x + cyclicPrefixLength_x;
    unsigned int frameSize = symbols * symbolSize;

    //Allocate memory for single symbol and whole frame
    DataSet< complex<float> >* out = NULL;
    getOutputDataSet("output1", out, frameSize);
    std::complex<float> buffer[symbolSize];

    // generate frame, one OFDM symbol at a time
    unsigned int bytesWritten = 0;
    while (symbols--) {
        ofdmflexframegen_writesymbol(frameGenerator_, buffer);
        std::copy(buffer, buffer + symbolSize, out->data.begin() + bytesWritten);
        bytesWritten += symbolSize;
    }

    out->timeStamp = in->timeStamp; // copy meta data of the frame

    releaseInputDataSet("input1", in);
    releaseOutputDataSet("output1", out);
}
} /* namespace iris */
