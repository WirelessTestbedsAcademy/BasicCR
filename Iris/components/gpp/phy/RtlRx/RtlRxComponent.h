/**
 * \file components/gpp/phy/RtlRx/RtlRxComponent.h
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
 * A Receiver component for the Realtek Dongle, using the RHD
 */

#ifndef PHY_RTLRXCOMPONENT_H_
#define PHY_RTLRXCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <boost/thread.hpp>

// Rtl Hardware Driver (RHD)
#include <rtl-sdr.h>

namespace iris
{
namespace phy
{

/*!
 * A Receiver component for the Realtek Dongle, using the RHD
 * 
 * This component streams data from the RTL Dongle and sets a timestamp and
 * sampleRate on the generated DataSet.
 * 
 */
class RtlRxComponent
  : public PhyComponent
{

 public:
  RtlRxComponent(std::string name);
  ~RtlRxComponent();
  virtual void calculateOutputTypes(
      std::map<std::string,int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

private:
	
	// The Device
	rtlsdr_dev *rtl_dev;
  std::vector<int> gains_d;
  double currentTimestamp_d;
	
	bool running_d;
	unsigned int skipped_d;			//Skip a certain number of buffers while the receiver is initialising.
	unsigned int buf_num_d;			//Number of buffers
	unsigned int buf_head_d;
	unsigned int buf_used_d;
	unsigned int buf_offset_d;
	boost::mutex buf_mutex_d;		//Need to have seperate threads for reading and outputting the samples 
	boost::condition_variable buf_cond_d;
	boost::thread thread_d;
	unsigned short **buf_d;
	int samp_avail_d;
	int rtl_clock_freq_d; 
	int tuner_clock_freq_d;
	std::vector<std::complex<float> > lut_d;
	

	// Exposed Parameters
  std::string args_x;             //See rtl-sdr.h in rtl-sdr/include/
  double frequency_x;             //Receive frequency
  double frequency_correction_x;	//Frequency correction
  double rate_x;                  //Receive rate
  int if_gain_x;                  //Set Intermediate Frequency gain
  double gain_x;                  //Receive gain
  int outputBlockSize_x;          //Output block size
  int device_index_x;             //Device index
	
	//Helper functions
	double setIfGain(double gain);
  double clipGain(double gain);
	
	static void rtlsdrCallback(unsigned char *buf, uint32_t len, void *ctx);
	void rtlsdrCallbackHelp(unsigned char *buf, uint32_t len);
	static void rtlsdrWait(RtlRxComponent *obj);
	void rtlsdrWaitHelp();

};

} // namespace phy
} // namespace iris

#endif // PHY_RTLRXCOMPONENT_H_
