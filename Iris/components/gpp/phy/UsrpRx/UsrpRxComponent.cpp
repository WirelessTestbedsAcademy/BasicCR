/**
 * \file components/gpp/phy/UsrpRx/UsrpRxComponent.cpp
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
 * Implementation of a source component which reads from a USRP receiver
 * using the Universal Hardware Driver (UHD).
 * This component streams data from the USRP and sets the timestamp and
 * sampleRate on the generated DataSet if supported.
 */

#include "UsrpRxComponent.h"

#include <uhd/utils/thread_priority.hpp>
#include <uhd/exception.hpp>
#include <boost/system/system_error.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/thread.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace uhd;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, UsrpRxComponent);

/*! Constructor
*
*  Call the constructor on PhyComponent and pass in all details about the component.
*  Register all parameters and events in the constructor.
*
*   \param  name  The name assigned to this component when loaded
*/
UsrpRxComponent::UsrpRxComponent(std::string name)
  : PhyComponent(name,
                "usrprx",
                "A Usrp receiver using the UHD",
                "Paul Sutton",
                "0.2")
  ,isUsrp1_(true)
  ,currentTimestamp_(0.0)
  ,gotFirstPacket_(false)
{
  /*
   * format:
   * registerParameter(name,
   *                   description,
   *                   default value,
   *                   dynamic?,
   *                   parameter,
   *                   allowed values);
   */
  registerParameter("args",
                    "A delimited string which may be used to specify a particular usrp",
                    "",
                    false,
                    args_x);
  registerParameter("rate",
                    "The receive rate",
                    "1000000",
                    true,
                    rate_x);
  registerParameter("frequency",
                    "The receive frequency",
                    "2400000000",
                    true,
                    frequency_x);
  registerParameter("gain",
                    "The transmit gain",
                    "1",
                    true,
                    gain_x);
  registerParameter("outputblocksize",
                    "How many samples to output in each block",
                    "1024",
                    true,
                    outputBlockSize_x);
  registerParameter("fixlooffset",
                    "Value to fix LO offset to in Hz - defaults to 2*rate",
                    "-1",
                    false,
                    fixLoOffset_x);
  registerParameter("antenna",
                    "Daughterboard antenna selection",
                    "",
                    false,
                    antenna_x);
  registerParameter("subdev",
                    "Daughterboard subdevice specification",
                    "",
                    false,
                    subDev_x);
  registerParameter("bw",
                    "Daughterboard IF filter bandwidth in Hz",
                    "0",
                    false,
                    bw_x);
  registerParameter("ref",
                    "Reference clock(internal, external, mimo)",
                    "internal",
                    false,
                    ref_x);
  registerParameter("wirefmt",
                    "Wire format (sc8 or sc16)",
                    "sc16",
                    false,
                    wireFmt_x);
}

UsrpRxComponent::~UsrpRxComponent()
{
  setStreaming(false);
}

/*! Register the ports of this component
*
*  Ports are registered by name with a vector of valid data types permitted on those ports.
*  This receiver has one output port
*/
void UsrpRxComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes;
  validTypes.push_back(TypeInfo< complex<float> >::identifier);

  //format:        (name, vector of valid types)
  registerOutputPort("output1", validTypes);
}

/*! Calculate output data types
*
*  Based on the input data types, tell the system what output data types will be provided.
*  \param  inputTypes  The data types of the inputs which will be passed to this component
*/
void UsrpRxComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //One output type - complex<float>
  outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

//! Do any initialization required
void UsrpRxComponent::initialize()
{
  // Set up the output DataBuffers
  outBuf_ = castToType< complex<float> >(outputBuffers.at(0));

  //Set up the usrp
  try
  {
    //Create the device
    LOG(LINFO) << "Creating the usrp device with args: " << args_x;
    usrp_ = uhd::usrp::multi_usrp::make(args_x);
    //Lock mboard clocks
    usrp_->set_clock_source(ref_x);
    //always select the subdevice first, the channel mapping affects the other settings
    if (subDev_x!="")
      usrp_->set_rx_subdev_spec(subDev_x);
    LOG(LINFO) << "Using Device: " << usrp_->get_pp_string();

    setStreaming(false);

    //Set properties on device
    LOG(LINFO) << "Setting RX Rate: " << (rate_x/1e6) << "Msps...";
    usrp_->set_rx_rate(rate_x);
    LOG(LINFO) << "Actual RX Rate: " << (usrp_->get_rx_rate()/1e6) << "Msps...";

    LOG(LINFO) << "Setting RX Frequency: " << (frequency_x/1e6) << "MHz...";
    double lo_offset = 2*rate_x;  //Set LO offset to twice signal rate by default
    if(fixLoOffset_x >= 0)
      lo_offset = fixLoOffset_x;
    usrp_->set_rx_freq(tune_request_t(frequency_x, lo_offset));
    LOG(LINFO) << "Actual RX Frequency: " << (usrp_->get_rx_freq()/1e6) << "MHz...";
    LOG(LINFO) << "RX LO offset: " << (lo_offset/1e6) << "MHz...";

    //We can only set the time on usrp2 devices
    if(usrp_->get_mboard_name().find("usrp1") == string::npos)
    {
      isUsrp1_ = false;
      LOG(LINFO) << "Setting device timestamp to 0...";
      usrp_->set_time_now(uhd::time_spec_t((double)0));
    }

    //Set the antenna
    if(antenna_x != "")
      usrp_->set_rx_antenna(boost::to_upper_copy(antenna_x));
    LOG(LINFO) << "Using RX Antenna: " << usrp_->get_rx_antenna();

    //Set gain
    gain_range_t range = usrp_->get_rx_gain_range();
    LOG(LINFO) << "Gain range: " << range.to_pp_string();
    LOG(LINFO) << "Setting Gain: " << gain_x;
    usrp_->set_rx_gain(gain_x);
    LOG(LINFO) << "Actual gain: " << usrp_->get_rx_gain();

    //set the IF filter bandwidth
    if (bw_x > 0)
    {
      LOG(LINFO) << "Setting RX Bandwidth: " << bw_x/1e6 << " MHz...";
      usrp_->set_rx_bandwidth(bw_x);
      LOG(LINFO) << "Actual RX Bandwidth: " << usrp_->get_rx_bandwidth()/1e6 << " MHz...";
    }

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time

    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = usrp_->get_rx_sensor_names(0);
    if(std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end())
    {
      uhd::sensor_value_t lo_locked = usrp_->get_rx_sensor("lo_locked",0);
      LOG(LINFO) << "Checking RX: " << lo_locked.to_pp_string() <<  "...";
      if(!lo_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }
    sensor_names = usrp_->get_mboard_sensor_names(0);
    if((ref_x == "mimo") and (std::find(sensor_names.begin(), sensor_names.end(), "mimo_locked") != sensor_names.end()))
    {
      uhd::sensor_value_t mimo_locked = usrp_->get_mboard_sensor("mimo_locked",0);
      LOG(LINFO) << "Checking RX: " << mimo_locked.to_pp_string() << " ...";
      if(!mimo_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }
    if ((ref_x == "external") and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end()))
    {
      uhd::sensor_value_t ref_locked = usrp_->get_mboard_sensor("ref_locked",0);
      LOG(LINFO) << "Checking RX: " << ref_locked.to_pp_string() <<  " ...";
      if(!ref_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }

    //allocate recv buffer and metadata
    uhd::rx_metadata_t md;
    std::vector<std::complex<float> > buff(usrp_->get_device()->get_max_recv_samps_per_packet());

    //flush the buffers in the recv path
    while(usrp_->get_device()->recv(
      &buff.front(), buff.size(), md,
      uhd::io_type_t::COMPLEX_FLOAT32,
      uhd::device::RECV_MODE_ONE_PACKET
    )){/* NOP */};

    //create a receive streamer
    uhd::stream_args_t stream_args("fc32",wireFmt_x);
    rxStream_ = usrp_->get_rx_stream(stream_args);
  }
  catch(const boost::exception &e)
  {
    throw IrisException(boost::diagnostic_information(e));
  }
  catch(std::exception& e)
  {
    throw IrisException(e.what());
  }
}

/*! The main work of the component is carried out here
*
*  This UsrpRx receives data from the usrp and writes it to an output buffer.
*/
void UsrpRxComponent::process()
{
  if(not isStreaming_)
    setStreaming(true);

  //Get a DataSet from the output DataBuffer
  DataSet< complex<float> >* writeDataSet = NULL;
  outBuf_->getWriteData(writeDataSet, outputBlockSize_x);

  rx_metadata_t md;
  int num_rx_samps;
  try
  {
    num_rx_samps = rxStream_->recv(&(writeDataSet->data.front()),
                                   writeDataSet->data.size(),
                                   md,
                                   5.0);
  }
  catch(...)
  {
    throw IrisException("Unexpected exception caught");
  }

  if(md.error_code != rx_metadata_t::ERROR_CODE_NONE)
  {
    LOG(LERROR) << "Usrp error code: " << md.error_code;
    string err;
    switch(md.error_code)
    {
      case rx_metadata_t::ERROR_CODE_LATE_COMMAND:
        err = "Late command";
        break;
      case rx_metadata_t::ERROR_CODE_BROKEN_CHAIN:
        err = "Broken chain";
        break;
      case rx_metadata_t::ERROR_CODE_OVERFLOW:
        err = "Overflow";
        break;
      default:
        err = "Unknown error";
        break;
    }
    LOG(LERROR) << "Usrp error: " << err;
  }

  //Set the metadata
  double rate = usrp_->get_rx_rate();
  if(md.has_time_spec && !isUsrp1_)
  {
    time_spec_t expectedTimestamp = currentTimestamp_ + time_spec_t(0, num_rx_samps, rate);
    currentTimestamp_ = md.time_spec;
    int lostSamples = boost::math::iround(rate*(md.time_spec - expectedTimestamp).get_real_secs());
    if(lostSamples > 0 && gotFirstPacket_)
    {
      LOG(LERROR) << "Overflow detected - lost " << lostSamples << " samples";
    }
  }
  else
  {
    currentTimestamp_ = currentTimestamp_ + time_spec_t(0, num_rx_samps, rate);
  }
  writeDataSet->sampleRate = rate;
  writeDataSet->timeStamp = currentTimestamp_.get_real_secs();

  gotFirstPacket_ = true;

  //Release the DataSet
  outBuf_->releaseWriteData(writeDataSet);

}

//! This gets called whenever a parameter is reconfigured
void UsrpRxComponent::parameterHasChanged(std::string name)
{
  try
  {
    if(name == "frequency")
    {
      LOG(LINFO) << "Setting RX Frequency: " << (frequency_x/1e6) << "MHz...";
      double lo_offset = 2*rate_x;  //Set LO offset to twice signal rate by default
      if(fixLoOffset_x >= 0)
        lo_offset = fixLoOffset_x;
      usrp_->set_rx_freq(tune_request_t(frequency_x, lo_offset));
    }
    else if(name == "rate")
    {
      LOG(LINFO) << "Setting RX Rate: " << (rate_x/1e6) << "Msps...";
      usrp_->set_rx_rate(rate_x);
      LOG(LINFO) << "Actual RX Rate: " << (usrp_->get_rx_rate()/1e6) << "Msps...";
    }
    else if(name == "gain")
    {
      gain_range_t range = usrp_->get_rx_gain_range();
      LOG(LINFO) << "Gain range: " << range.to_pp_string();
      LOG(LINFO) << "Setting Gain: " << gain_x;
      usrp_->set_rx_gain(gain_x);
    }
  }
  catch(std::exception &e)
  {
    throw IrisException(e.what());
  }
}

void UsrpRxComponent::setStreaming(bool s)
{
  //setup streaming
  if(s)
  {
    usrp_->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
  }
  else
  {
    usrp_->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
  }
  isStreaming_ = s;
}

} // namespace phy
} // namespace iris
