/**
 * \file components/gpp/phy/RtlRx/RtlRxComponent.cpp
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
 * This component streams data from the RTL Dongle and sets a timestamp and
 * sampleRate on the generated DataSet.
 */



#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "RtlRxComponent.h"

#include <boost/system/system_error.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/assign.hpp>				// Needed for operator overloading

#include <stdio.h>
#include <map>

using namespace std;
using namespace boost::assign;

namespace iris
{
namespace phy
{

#define BUF_SIZE  (16 * 32 * 512)
#define BUF_NUM   32
#define BUF_SKIP  1 		//Number of buffers to skip for initiallization

#define BYTES_PER_SAMPLE  2 //The Rtl device delivers 8 bit unsigned IQ data

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, RtlRxComponent);

RtlRxComponent::RtlRxComponent(std::string name)
  : PhyComponent(name,                      // component name
                "rtlrxcomponent",           // component type
                "Rtl receiver",             // description
                "Jonathan van de Belt",     // author
                "0.1")                      // version
{
  registerParameter("args", "A delimited string which may be used to specify particular settings", "", false, args_x);
  registerParameter("rate", "The receive rate", "2400000", true, rate_x, Interval<double>(25000,2400000));
  registerParameter("frequency", "The receive frequency", "100000000", true, frequency_x, Interval<double>(52000000,2190000000));
  registerParameter("frequencycorrection", "frequencycorrection", "0", true, frequency_correction_x);
  registerParameter("gain", "The receive gain", "0", true, gain_x), Interval<int>(-1,42);
  registerParameter("ifgain", "The IF gain", "0", true, if_gain_x);
  registerParameter("outputblocksize", "How many samples to output in each block", "1024", false, outputBlockSize_x);
  registerParameter("deviceindex", "Device Index","0",false,device_index_x, Interval<int>(0,4));

  running_d = false;
  currentTimestamp_d = 0.0;
  skipped_d = 0;
  rtl_clock_freq_d = 0;
  tuner_clock_freq_d = 0;
}

RtlRxComponent::~RtlRxComponent()
{
  if (rtl_dev) {
    running_d = false;
    rtlsdr_cancel_async(rtl_dev);
    thread_d.join();
    rtlsdr_close(rtl_dev);
    rtl_dev = NULL;
  }

  if (buf_d) {
    for(unsigned int i = 0; i < buf_num_d; ++i) {
      if (buf_d[i])
        free(buf_d[i]);
    }

    free(buf_d);
    buf_d = NULL;
  }
}

void RtlRxComponent::registerPorts()
{
  registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void RtlRxComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //One output type - always complex<float>
  outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

void RtlRxComponent::initialize()
{

  //Set up the output DataBuffer

  //Set up the rtl device
  try
  {
    //Check if there is an rtl connected
    if(device_index_x < rtlsdr_get_device_count())
    {
      //Create the device
      LOG(LINFO) << "Creating the rtl device with args: " << args_x;

      LOG(LINFO) << "Using device " << device_index_x << ": "
             << rtlsdr_get_device_name(device_index_x);

      rtl_dev = NULL;
      int ret = rtlsdr_open(&rtl_dev, device_index_x);
      if (ret < 0)
          throw IrisException("Failed to open rtlsdr device.");

      //ARGS -- not implemented yet
      /*
      if (rtl_clock_freq_d > 0 || tuner_clock_freq_d > 0) {
          if (rtl_clock_freq_d)
            LOG(LINFO) << "Setting rtl clock to " << rtl_clock_freq_d << " Hz.";
          if (tuner_clock_freq_d)
            LOG(LINFO) << "Setting tuner clock to " << tuner_clock_freq_d << " Hz.";

          ret = rtlsdr_set_xtal_freq( rtl_dev, rtl_clock_freq_d, tuner_clock_freq_d );
          if (ret < 0)
            throw IrisException("Failed to set clock frequencies.");
        }
      */


      //Set up lut for tranforming raw data to complex<float>
      //See http://sdr.osmocom.org/trac/attachment/wiki/rtl-sdr/rtl2832-cfile.png

      //First deinterleave 8-bit I and Q samples, depending on endian convention. Then add a constant (-127) and multiply by 0.008.
      for (unsigned int i = 0; i <= 0xffff; i++)
      {
      #ifdef BOOST_LITTLE_ENDIAN
        lut_d.push_back(std::complex<float> ( (float(i & 0xff) - 127.5f) * (1.0f/128.0f),
                           (float(i >> 8) - 127.5f) * (1.0f/128.0f) ) );
      #else // BOOST_BIG_ENDIAN
        lut_d.push_back(std::complex<float> ( (float(i >> 8) - 127.5f) * (1.0f/128.0f),
                           (float(i & 0xff) - 127.5f) * (1.0f/128.0f) ) );
      #endif
      }

      //Set properties on device
      LOG(LINFO) << "Setting RX Rate: " << (rate_x/1e6) << "Msps...";
      rtlsdr_set_sample_rate(rtl_dev, (uint32_t)rate_x);
      LOG(LINFO) << "Actual RX Rate: " << (rtlsdr_get_sample_rate(rtl_dev)/1e6) << "Msps...";

      LOG(LINFO) << "Setting RX Frequency: " << (frequency_x/1e6) << "MHz...";
      rtlsdr_set_center_freq(rtl_dev, (uint32_t)frequency_x);
      rtlsdr_set_freq_correction( rtl_dev, (int)frequency_correction_x );
      LOG(LINFO) << "Actual RX Frequency: " << (rtlsdr_get_center_freq(rtl_dev)/1e6) << "MHz...";

      //Set gain mode
      LOG(LINFO) << "Setting Gain mode to " << ((gain_x)? "Manual" : "Automatic" );
      ret = rtlsdr_set_tuner_gain_mode(rtl_dev, gain_x);
      if (ret < 0)
        throw IrisException("Failed to set tuner gain mode.");
      ret = rtlsdr_set_agc_mode(rtl_dev, gain_x);
      if (ret < 0)
        throw IrisException("Failed to set agc mode.");
      ret = rtlsdr_reset_buffer( rtl_dev );
      if (ret < 0)
        throw IrisException("Failed to reset usb buffers.");

      //Find Gain range
      int count = rtlsdr_get_tuner_gains(rtl_dev, NULL);
      if (count > 0)
      {
        int* gains = new int[ count ];
        count = rtlsdr_get_tuner_gains(rtl_dev, gains);
        for(int i=0;i<count;i++)
          gains_d.push_back(gains[i]);
      }

      //Manual mode on
      if(gain_x)
      {
        //Set gain
        LOG(LINFO) << "Setting Gain: " << gain_x;
        rtlsdr_set_tuner_gain(rtl_dev, int(clipGain(gain_x) * 10.0) );
        LOG(LINFO) << "Actual Gain: " << ((double)rtlsdr_get_tuner_gain( rtl_dev ))/10.0;
        //Gain range values for the E4000 tuner
        //LOG(LINFO) << "Gain Range: -1, 1.5, 4, 6.5, 9, 11.5, 14, 16.5, 19, 21.5, 24, 29, 34, 42";

      }

      //Set the IF gain
      setIfGain( if_gain_x );

      //Allocate recv buffer and metadata
      buf_num_d = BUF_NUM;
      buf_head_d = buf_used_d = buf_offset_d = 0;
      samp_avail_d = BUF_SIZE / BYTES_PER_SAMPLE;

      buf_d = (unsigned short **) malloc(buf_num_d * sizeof(unsigned short *));

      if (buf_d)
      {
         for(unsigned int i = 0; i < buf_num_d; ++i)
            buf_d[i] = (unsigned short *) malloc(BUF_SIZE);
      }

      //Create a new thread for the wait & read function
      thread_d = boost::thread(rtlsdrWait, this);

    }
    else
      throw IrisException("No Rtl devices found");

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

 void RtlRxComponent::process()
{
  running_d = true;
  std::vector<std::complex<float> > out;

  //Get a DataSet from the output DataBuffer
  DataSet< complex<float> >* writeDataSet = NULL;
  getOutputDataSet("output1", writeDataSet, outputBlockSize_x);

  try
  {
    {	//Get lock on mutex
      boost::mutex::scoped_lock lock( buf_mutex_d );

      while (buf_used_d < 3 && running_d) //Collect at least 3 buffers on other thread
        buf_cond_d.wait( lock );		//Hold lock
    } //Release lock

    unsigned short *buf = buf_d[buf_head_d] + buf_offset_d;

    //ASSUMPTION: outputBlockSize_x < BUF_SIZE / BYTES_PER_SAMPLE

    //If we can only proccess a section of the buffer, read up to this point
    if (writeDataSet->data.size() <= samp_avail_d)
    {
      for (int i = 0; i < writeDataSet->data.size(); ++i)
      {
        //Vector of std::complex<float>
        out.push_back(lut_d[ *(buf + i) ]);				//Pass each IQ sample to out through the lut
      }

      buf_offset_d += writeDataSet->data.size();
      samp_avail_d -= writeDataSet->data.size();

    }
    else 	// Else read remainder of buffer
    {

      for (int i = 0; i < samp_avail_d; ++i)
      {
        out.push_back(lut_d[ *(buf + i) ]);					//Pass each IQ sample to out through the lut
      }

      // Move to next buffer and free current buffer.
      {  //Get lock on mutex
        boost::mutex::scoped_lock lock(buf_mutex_d);
        buf_head_d = (buf_head_d + 1) % buf_num_d;
        buf_used_d--;
      }  //Release lock

      buf = buf_d[buf_head_d];

      int remaining = writeDataSet->data.size() - samp_avail_d;

      for (int i = 0; i < remaining; ++i)
      {
        out.push_back(lut_d[ *(buf + i) ]);					//Pass each IQ sample to out through the lut
      }

      buf_offset_d = remaining;
      samp_avail_d = (BUF_SIZE / BYTES_PER_SAMPLE) - remaining;
    }

    //Copy the input DataSet to the output DataSet
    copy(out.begin(), out.end(), writeDataSet->data.begin());

  }
  catch(...)
  {
    throw IrisException("Unexpected exception caught");
  }

  //Set the metadata
  double rate = rtlsdr_get_sample_rate(rtl_dev);
  currentTimestamp_d = currentTimestamp_d + outputBlockSize_x/rate;

  writeDataSet->sampleRate = rate;
  writeDataSet->timeStamp = currentTimestamp_d;

  //Release the DataSet
  releaseOutputDataSet("output1", writeDataSet);
}

void RtlRxComponent::parameterHasChanged(std::string name)
{
  if(name == "rate")
  {
    //Set properties on device
    LOG(LINFO) << "Setting RX Rate: " << (rate_x/1e6) << "Msps...";
    rtlsdr_set_sample_rate(rtl_dev, (uint32_t)rate_x);
    LOG(LINFO) << "Actual RX Rate: " << (rtlsdr_get_sample_rate(rtl_dev)/1e6) << "Msps...";
  }

  if(name == "frequency")
  {
    LOG(LINFO) << "Setting RX Frequency: " << (frequency_x/1e6) << "MHz...";
    rtlsdr_set_center_freq(rtl_dev, (uint32_t)frequency_x);
    rtlsdr_set_freq_correction( rtl_dev, (int)frequency_correction_x );
    LOG(LINFO) << "Actual RX Frequency: " << (rtlsdr_get_center_freq(rtl_dev)/1e6) << "MHz...";
  }

  if(name == "gain")
  {
    //Set gain mode
    int ret;
    LOG(LINFO) << "Setting Gain mode to " << ((gain_x)? "Manual" : "Automatic" );
    ret = rtlsdr_set_tuner_gain_mode(rtl_dev, gain_x);
    if (ret < 0)
      throw IrisException("Failed to set tuner gain mode.");
    ret = rtlsdr_set_agc_mode(rtl_dev, gain_x);
    if (ret < 0)
      throw IrisException("Failed to set agc mode.");
    ret = rtlsdr_reset_buffer( rtl_dev );
    if (ret < 0)
      throw IrisException("Failed to reset usb buffers.");

    //Find Gain range
    int count = rtlsdr_get_tuner_gains(rtl_dev, NULL);
    if (count > 0)
    {
      int* gains = new int[ count ];
      count = rtlsdr_get_tuner_gains(rtl_dev, gains);
      for(int i=0;i<count;i++)
        gains_d.push_back(gains[i]);
    }

    //Manual mode on
    if(gain_x)
    {
      //Set gain
      LOG(LINFO) << "Setting Gain: " << gain_x;
      rtlsdr_set_tuner_gain(rtl_dev, int(clipGain(gain_x) * 10.0) );
      LOG(LINFO) << "Actual Gain: " << ((double)rtlsdr_get_tuner_gain( rtl_dev ))/10.0;
      //Gain range values for the E4000 tuner
      //LOG(LINFO) << "Gain Range: -1, 1.5, 4, 6.5, 9, 11.5, 14, 16.5, 19, 21.5, 24, 29, 34, 42";
    }
  }

  if(name == "ifgain")
  {
    //Set the IF gain
    setIfGain( if_gain_x );
  }
}



/*! The callback function that gets called by the device every time samples are available.
 *
 *	Uses a helper function that copies the samples to the buffer
 */
void RtlRxComponent::rtlsdrCallback(unsigned char *buf, uint32_t len, void *ctx)
{
  RtlRxComponent *obj = (RtlRxComponent *)ctx;
  obj->rtlsdrCallbackHelp(buf, len);
}

void RtlRxComponent::rtlsdrCallbackHelp(unsigned char *buf, uint32_t len)
{
  //Skip initialisation samples
  if (skipped_d < BUF_SKIP)
  {
    skipped_d++;
    return;
  }
  //LOG(LINFO) << "Log: Read";
  {
    //Get lock on mutex
    boost::mutex::scoped_lock lock( buf_mutex_d );

    int buf_tail_d = (buf_head_d + buf_used_d) % buf_num_d;
      memcpy(buf_d[buf_tail_d], buf, len);

    if (buf_used_d == buf_num_d)
    {
      //std::cerr << "O" << std::flush;
      buf_head_d = (buf_head_d + 1) % buf_num_d;
    }
    else
    {
      buf_used_d++;
    }
  }
  //Release lock

  //LOG(LINFO) << "Log: Read Success";
  buf_cond_d.notify_one();
}
    
/*! The read function that waits for the device to output samples
 *
 *	Uses a helper function
 */
void RtlRxComponent::rtlsdrWait(RtlRxComponent *obj)
{
  obj->rtlsdrWaitHelp();
}
    
void RtlRxComponent::rtlsdrWaitHelp()
{
  // Function will block until cancelled using rtlsdr_cancel_async()
  int ret = rtlsdr_read_async(rtl_dev, rtlsdrCallback, (void *)this, 0, BUF_SIZE );

  running_d = false;

  if ( ret != 0 )
    LOG(LERROR) << "RHD read asynchronous function returned with " << ret;
}
    
/*! Function that deals with finding the valid gain ranges of the Rtl device
 *
 *	Function taken from gr-osmosdr block
 */
double RtlRxComponent::setIfGain(double gain)
{
  std::vector< Interval<int> > if_gains;

  if_gains += Interval<int>(-3, 6, 9);
  if_gains += Interval<int>(0, 9, 3);
  if_gains += Interval<int>(0, 9, 3);
  if_gains += Interval<int>(0, 2, 1);
  if_gains += Interval<int>(3, 15, 3);
  if_gains += Interval<int>(3, 15, 3);


  std::map< int, double > gains;
  Interval<int> range;

  //Initialize with minimum gains

  for (unsigned int i = 0; i < if_gains.size(); i++) {
    gains[ i + 1 ] = if_gains[ i ].minimum;
  }

  for (int i = if_gains.size() - 1; i >= 0; i--) {
    range = if_gains[ i ];

    double error = gain;

    for( double g = range.minimum; g <= range.maximum; g += range.step ) {

      double sum = 0;
      for (int j = 0; j < int(gains.size()); j++) {
        if ( i == j )
          sum += g;
        else
          sum += gains[ j + 1 ];
      }

      double err = abs(gain - sum);
      if (err < error) {
        error = err;
        gains[ i + 1 ] = g;
      }
    }
  }
#if 0
  std::cerr << gain << " => "; double sum = 0;
  for (unsigned int i = 0; i < gains.size(); i++) {
    sum += gains[ i + 1 ];
    std::cerr << gains[ i + 1 ] << " ";
  }
  std::cerr << " = " << sum << std::endl;
#endif
  for (unsigned int stage = 1; stage <= gains.size(); stage++) {
    rtlsdr_set_tuner_if_gain( rtl_dev, stage, int(gains[ stage ] * 10.0));
  }

  return gain;
}

double RtlRxComponent::clipGain(double gain)
{
  if(gains_d.size() == 0)
    return gain;

  double minDiff = gains_d[0] - gain;
  int idx = 0;
  for(int i=1;i<gains_d.size();i++)
  {
    double diff = gains_d[i]-gain;
    if(diff < minDiff)
    {
      minDiff = diff;
      idx = i;
    }
  }

  return gains_d[idx];
}


} // namesapce phy
} // namespace iris
