/**
 * \file components/gpp/stack/TunTap/TunTapComponent.cpp
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2011-2013 Andre Puschmann <andre.puschmann@tu-ilmenau.de>
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
 * This component implements a software connector between Iris and
 * virtual network device drivers TUN/TAP
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "TunTapComponent.h"

using namespace std;
using boost::mutex;
using boost::condition_variable;
using boost::shared_ptr;
using boost::lock_guard;
using std::string;

namespace iris
{
namespace stack
{

// export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, TunTapComponent);

TunTapComponent::TunTapComponent(std::string name)
  : StackComponent(name,
                   "tuntapstackcomponent",
                   "Interface to tun/tap virtual network devices",
                   "Andre Puschmann",
                   "0.1")
{
  //Format: registerParameter(name, description, default, dynamic?, parameter, allowed values);
  registerParameter("device",
                    "Name of the Tun/Tap device to attach to",
                    "tun0",
                    false,
                    tunTapDevice_x);
  registerParameter("readfrombelow",
                    "Accept blocks from below (as opposed to above)",
                    "true",
                    true,
                    readFromBelow_x);
}


void TunTapComponent::initialize()
{
}


void TunTapComponent::processMessage(boost::shared_ptr<StackDataSet> incomingFrame)
{
  size_t frameSize = incomingFrame->data.size();
  size_t writtenBytes;


  //LOG(LDEBUG) << "processMessage() called.";
  assert(frameSize <= MAX_BUF_SIZE);
  for (int i = 0; i < (int)frameSize; i++)
    buffer_[i] = incomingFrame->data[i];

  writtenBytes = write(tunFd_, buffer_, frameSize);

  if (writtenBytes != frameSize)
    LOG(LERROR) << "Less bytes written to tun/tap device then requested.";
  else
  {
    LOG(LDEBUG) << "Successfully wrote "
      << writtenBytes
      << " bytes to tun device";
  }
}


void TunTapComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> incomingFrame)
{
  //LOG(LDEBUG) << "processMessageFromAbove() called.";

  if(readFromBelow_x == false)
    processMessage(incomingFrame);
}


void TunTapComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> incomingFrame)
{
  //LOG(LDEBUG) << "processMessageFromBelow() called.";

  if(readFromBelow_x)
    processMessage(incomingFrame);
}


void TunTapComponent::start()
{
  //LOG(LDEBUG) << "start() called.";

  // Connect to the device
  strcpy(tunName_, tunTapDevice_x.c_str());
  int flags = strstr(tunName_, "tap") == NULL ? IFF_TUN : IFF_TAP | IFF_NO_PI;
  if ((tunFd_ = allocateTunDevice(tunName_, flags)) >= 0)
  {
    LOG(LINFO) << "Successfully attached to tun/tap device "
      << tunTapDevice_x;
  }
  else
  {
    LOG(LFATAL) << "Error allocating tun/tap interface.";
    return;
  }

  // start thread
  rxThread_.reset(new boost::thread(boost::bind( &TunTapComponent::rxThreadFunction, this)));
}


void TunTapComponent::stop()
{
  if (rxThread_) {
    rxThread_->interrupt();
    rxThread_->join();
  }
  close(tunFd_);
}


void TunTapComponent::rxThreadFunction()
{
  //LOG(LINFO) << "RX thread started, listening on tun/tap device " << x_tunTapDevice;
  fd_set socketSet;
  struct timeval selectTimeout;
  char buffer[MAX_BUF_SIZE];
  int nread;

  try
  {
    // read data coming from the kernel
    while(true)
    {
      boost::this_thread::interruption_point();

      // reset socket set and add tap descriptor
      FD_ZERO(&socketSet);
      FD_SET(tunFd_, &socketSet);

      // initialize timeout
      selectTimeout.tv_sec = 1;
      selectTimeout.tv_usec = 0;

      // suspend thread until we receive a packet or timeout
      if (select(tunFd_ + 1, &socketSet, NULL, NULL, &selectTimeout) == 0) {
        //LOG(LDEBUG) << "Timeout while waiting for incoming packet.";
      } else {
        if (FD_ISSET(tunFd_, &socketSet)) {
          if ((nread = read(tunFd_, buffer, MAX_BUF_SIZE)) < 0)
          {
            LOG(LFATAL) << "Error while reading from tun/tap interface.";
            continue;
          }
          LOG(LDEBUG) << "Read " << nread << " bytes from device "
            << tunName_;

          // copy received data into new StackDataSet
          shared_ptr<StackDataSet> packetBuffer(new StackDataSet);
          packetBuffer->data.assign(buffer, buffer + nread);

          // send downwards
          sendDownwards(packetBuffer);
        }
      }
    } // while (true)
    throw SystemException("Rx thread stopped unexpectedly.");
  }
  catch(IrisException& ex)
  {
    LOG(LFATAL) << "Error in TunTap component: " << ex.what()
      << " - TX thread exiting.";
  }
  catch(boost::thread_interrupted)
  {
    LOG(LINFO) << "Thread " << boost::this_thread::get_id()
      << " in stack component interrupted.";
  }
}


/* Arguments taken by the function:
*
* code shamelessly taken from
* http://backreference.org/2010/03/26/tuntap-interface-tutorial/
*
* char *dev: the name of an interface (or '\0'). MUST have enough
*   space to hold the interface name if '\0' is passed
* int flags: interface flags (eg, IFF_TUN etc.)
*/
int TunTapComponent::allocateTunDevice(char *dev, int flags)
{
  struct ifreq ifr;
  int fd, err;
  const char *clonedev = (const char *)"/dev/net/tun";


  // open the clone device
  if((fd = open(clonedev, O_RDWR)) < 0)
  {
    return fd;
  }

  // preparation of the struct ifr, of type "struct ifreq"
  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;   // IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI

  if (*dev)
  {
    // if a device name was specified, put it in the structure; otherwise,
    // the kernel will try to allocate the "next" device of the
    // specified type
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  // try to create the device
  if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
  {
    close(fd);
    return err;
  }

  // if the operation was successful, write back the name of the
  // interface to the variable "dev", so the caller can know
  // it. Note that the caller MUST reserve space in *dev (see calling
  // code below)
  strcpy(dev, ifr.ifr_name);

  // this is the special file descriptor that the caller will use to talk
  // with the virtual interface
  return fd;
}

} // namespace stack
} // namespace iris
