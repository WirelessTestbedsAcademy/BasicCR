/**
 * \file components/gpp/phy/UsrpTx/UsrpTxComponent.cpp
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
 * Implementation of a sink component which writes to a USRP transmitter
 * using the Universal Hardware Driver (UHD).
 * This component supports streaming data by default. For bursty data,
 * set "streaming" to false. For bursty data, packets must be contained
 * in a single DataSet. If a timestamp is specified in a DataSet, the
 * packet will be transmitted at that time, if supported by the USRP.
 */

#include "UsrpTxComponent.h"

#include <uhd/utils/thread_priority.hpp>
#include <boost/thread.hpp>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace uhd;


namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, UsrpTxComponent);

/*! Constructor
*
*  Call the constructor on PhyComponent and pass in all details about the component.
*  Register all parameters and events in the constructor.
*
* \param  name  The name assigned to this component when loaded
*/
UsrpTxComponent::UsrpTxComponent(std::string name)
  : PhyComponent(name,
                "usrptx",
                "A Usrp transmitter using the Universal Hardware Driver",
                "Paul Sutton",
                "0.1")
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
                    "The transmit rate",
                    "1000000",
                    true,
                    rate_x);
  registerParameter("frequency",
                    "The transmit frequency",
                    "2400000000",
                    true,
                    frequency_x);
  registerParameter("gain",
                    "The transmit gain",
                    "1",
                    true,
                    gain_x);
  registerParameter("streaming",
                    "Whether we're streaming data to tx",
                    "true",
                    true,
                    streaming_x);
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
                    "Daughterboard IF filter bandwidth (Hz)",
                    "0",
                    false,
                    bw_x);
  registerParameter("ref",
                    "Reference waveform (internal, external, mimo)",
                    "internal",
                    false,
                    ref_x);
  registerParameter("fmt",
                    "Data format (fc64, fc32 or sc16)",
                    "fc32",
                    false,
                    fmt_x);
}

/*! Destructor
*
*  Send an EOB packet to stop the Usrp
*/
UsrpTxComponent::~UsrpTxComponent()
{
  //Send a mini EOB packet
  uhd::tx_metadata_t md;
  md.start_of_burst = false;
  md.end_of_burst   = true;
  vector< complex<float> > v;
  if(txStream_ != NULL)
  {
    txStream_->send(&v.front(), 0, md);
  }
}

/*! Register the ports of this component
*
*  Ports are registered by name with a vector of valid data types permitted on those ports.
*  This example has one input port with a single valid data type - complex<float>.
*/
void UsrpTxComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes;
  validTypes.push_back(TypeInfo< complex<float> >::identifier);

  //format:        (name, vector of valid types)
  registerInputPort("input1", validTypes);
}

/*! Calculate output data types
*
*  Based on the input data types, tell the system what output data types will be provided.
*  \param  inputTypes  The data types of the inputs which will be passed to this component
*/
void UsrpTxComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //No output types
}

//! Do any initialization required
void UsrpTxComponent::initialize()
{
  uhd::set_thread_priority_safe();

  //Set up the input DataBuffer
  inBuf_ = castToType< complex<float> >(inputBuffers.at(0));

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
      usrp_->set_tx_subdev_spec(subDev_x);
    LOG(LINFO) << "Using Device: " << usrp_->get_pp_string();

    //Set rate
    LOG(LINFO) << "Setting TX Rate: " << (rate_x/1e6) << "Msps...";
    usrp_->set_tx_rate(rate_x);
    LOG(LINFO) << "Actual TX Rate: " << (usrp_->get_tx_rate()/1e6) << "Msps...";

    //Set frequency
    LOG(LINFO) << "Setting TX Frequency: " << (frequency_x/1e6) << "MHz...";
    double lo_offset = 2*rate_x;  //Set LO offset to twice signal rate by default
    if(fixLoOffset_x >= 0)
      lo_offset = fixLoOffset_x;
    usrp_->set_tx_freq(tune_request_t(frequency_x, lo_offset));
    LOG(LINFO) << "Actual TX Frequency: " << (usrp_->get_tx_freq()/1e6) << "MHz";
    LOG(LINFO) << "RX LO offset: " << (lo_offset/1e6) << "MHz...";

    //We can only set the time on usrp2 devices
    if(usrp_->get_mboard_name().find("usrp1") == string::npos)
    {
      LOG(LINFO) << "Setting device timestamp to 0...";
      usrp_->set_time_now(uhd::time_spec_t((double)0));
    }

    //set the rf gain
    gain_range_t range = usrp_->get_tx_gain_range();
    LOG(LINFO) << "Gain range: " << range.to_pp_string();
    LOG(LINFO) << "Setting TX Gain: " << gain_x << " dB...";
    usrp_->set_tx_gain(gain_x);
    LOG(LINFO) << "Actual TX Gain: " <<  usrp_->get_tx_gain() << " dB...";

    //set the IF filter bandwidth
    if(bw_x!=0)
    {
      LOG(LINFO) << "Setting TX Bandwidth: " << bw_x << " MHz...";
      usrp_->set_tx_bandwidth(bw_x);
      LOG(LINFO) << "Actual TX Bandwidth: " << usrp_->get_tx_bandwidth() << " MHz...";
    }

    //Set the antenna
    if(antenna_x!="")
      usrp_->set_tx_antenna(boost::to_upper_copy(antenna_x));
    LOG(LINFO) << "Using TX Antenna: " << usrp_->get_tx_antenna();

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time

    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = usrp_->get_tx_sensor_names(0);
    if (std::find(sensor_names.begin(),
                  sensor_names.end(),
                  "lo_locked") != sensor_names.end())
    {
      uhd::sensor_value_t lo_locked = usrp_->get_tx_sensor("lo_locked",0);
      LOG(LINFO) << "Checking TX: " << lo_locked.to_pp_string() << " ...";
      if(!lo_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }
    sensor_names = usrp_->get_mboard_sensor_names(0);
    if ((ref_x == "mimo") and (std::find(sensor_names.begin(),
                                         sensor_names.end(),
                                         "mimo_locked") != sensor_names.end()))
    {
      uhd::sensor_value_t mimo_locked = usrp_->get_mboard_sensor("mimo_locked",0);
      LOG(LINFO) << "Checking TX: " << mimo_locked.to_pp_string() << " ...";
      if(!mimo_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }
    if ((ref_x == "external") and (std::find(sensor_names.begin(),
                                             sensor_names.end(),
                                             "ref_locked") != sensor_names.end()))
    {
      uhd::sensor_value_t ref_locked = usrp_->get_mboard_sensor("ref_locked",0);
      LOG(LINFO) << "Checking TX: " << ref_locked.to_pp_string() << " ...";
      if(!ref_locked.to_bool())
        throw IrisException("Failed to lock LO");
    }

    //create a transmit streamer
    uhd::stream_args_t stream_args(fmt_x);
    txStream_ = usrp_->get_tx_stream(stream_args);
  }
  catch(std::exception& e)
  {
    throw IrisException(e.what());
  }
}

/*! The main work of the component is carried out here
*
*  Take a DataSet from the input buffer and send to the usrp
*/
void UsrpTxComponent::process()
{
  //Get a DataSet from the input DataBuffer
  DataSet< complex<float> >* readDataSet = NULL;
  inBuf_->getReadData(readDataSet);

  size_t size = readDataSet->data.size();

  //Set up metadata
  uhd::tx_metadata_t md;
  md.start_of_burst = true;
  if(streaming_x)
  {
    md.end_of_burst = false;
  }else{
    md.end_of_burst = true;
  }
  if(readDataSet->timeStamp > 0)
  {
    md.has_time_spec = true;
    md.time_spec = uhd::time_spec_t(readDataSet->timeStamp);
  }

  //Send the data
  size_t num_tx_samps = txStream_->send(
    &readDataSet->data.front(), size, md
  );

  //Release the DataSet
  inBuf_->releaseReadData(readDataSet);

}

//! This gets called whenever a parameter is reconfigured
void UsrpTxComponent::parameterHasChanged(std::string name)
{
  try
  {
    if(name == "frequency")
    {
      LOG(LINFO) << "Setting TX Frequency: " << (frequency_x/1e6) << "MHz...";
      double lo_offset = 2*rate_x;  //Set LO offset to twice signal rate by default
      if(fixLoOffset_x >= 0)
      {
        lo_offset = fixLoOffset_x;
      }
      usrp_->set_tx_freq(tune_request_t(frequency_x, lo_offset));
      LOG(LINFO) << "Actual TX Frequency: " << (usrp_->get_tx_freq()/1e6) << "MHz";
    }
    else if(name == "rate")
    {
      LOG(LINFO) << "Setting TX Rate: " << (rate_x/1e6) << "Msps...";
      usrp_->set_tx_rate(rate_x);
      LOG(LINFO) << "Actual TX Rate: " << (usrp_->get_tx_rate()/1e6) << "Msps...";
    }
    else if(name == "gain")
    {
      gain_range_t range = usrp_->get_tx_gain_range();
      LOG(LINFO) << "Gain range: " << range.to_pp_string();
      LOG(LINFO) << "Setting TX Gain: " << gain_x << " dB...";
      usrp_->set_tx_gain(gain_x);
      LOG(LINFO) << "Actual TX Gain: " <<  usrp_->get_tx_gain() << " dB...";
    }
  }
  catch(std::exception &e)
  {
    throw IrisException(e.what());
  }
}

} // namespace phy
} // namespace iris
