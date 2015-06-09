/**
 * \file components/gpp/stack/AlohaMac/AlohaMacComponent.cpp
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
 * Implementation of a simple Aloha MAC component.
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "AlohaMacComponent.h"
#include "utility/StackHelper.h"

#ifdef __unix__
#include "utility/NetworkingHelper.h"
#endif

using namespace std;

namespace iris
{
namespace stack
{

// export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, AlohaMacComponent);

AlohaMacComponent::AlohaMacComponent(std::string name)
  : StackComponent(name,
                   "alohamacstackcomponent",
                   "A simple Aloha MAC component",
                   "Andre Puschmann",
                   "0.1")
  ,txSeqNo_(1)
  ,rxSeqNo_(0)
  ,rxPktBuffer_(10000)
  ,txPktBuffer_(10000)
{
  //Format: registerParameter(name, description, default, dynamic?, parameter, allowed values);
  registerParameter("localaddress", "Address of this client", "f009e090e90e", false, localAddress_x);
  registerParameter("destinationaddress", "Address of the destination client", "00f0f0f0f0f0", false, destinationAddress_x);
#ifdef __unix__
  registerParameter("isethdevice", "Whether to act as an Ethernet device", "false", false, isEthernetDevice_x);
  registerParameter("ethdevicename", "Name of the Ethernet device", "false", "tap0", ethernetDeviceName_x);
#endif
  registerParameter("acktimeout", "Time to wait for ACK packets in ms", "100", false, ackTimeout_x);
  registerParameter("maxretry", "Number of retransmissions", "100", false, maxRetry_x);
}


AlohaMacComponent::~AlohaMacComponent()
{
}


void AlohaMacComponent::initialize()
{
    maxRetry_x++; // first attempt does not count as retransmission
    // set local address according to user configuration
#ifdef __unix__
    if (isEthernetDevice_x) {
        LOG(LINFO) << "Trying to retrieve MAC address from " << ethernetDeviceName_x << ".";
        std::string backup(localAddress_x);
        if ((NetworkingHelper::getLocalAddress(ethernetDeviceName_x, localAddress_x)) == false) {
            LOG(LERROR) << "Failed! Using local address given by user.";
            localAddress_x = backup;
        }
    }
#endif
    LOG(LINFO) << "Local address is: " << localAddress_x;
}


void AlohaMacComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> incomingFrame)
{
  //StackHelper::printDataset(incomingFrame, "vanilla from above");
  txPktBuffer_.pushDataSet(incomingFrame);
}


void AlohaMacComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> incomingFrame)
{
  //StackHelper::printDataset(incomingFrame, "vanilla from below");
  rxPktBuffer_.pushDataSet(incomingFrame);
}


void AlohaMacComponent::start()
{
  rxThread_.reset(new boost::thread(boost::bind( &AlohaMacComponent::rxThreadFunction, this)));
  txThread_.reset(new boost::thread(boost::bind( &AlohaMacComponent::txThreadFunction, this)));
}

void AlohaMacComponent::stop()
{
  // stop threads
  rxThread_->interrupt();
  rxThread_->join();
  txThread_->interrupt();
  txThread_->join();
}

void AlohaMacComponent::rxThreadFunction()
{
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  LOG(LINFO) << "Rx thread started.";

  try
  {
    while(true)
    {
      boost::this_thread::interruption_point();

      boost::shared_ptr<StackDataSet> frame = rxPktBuffer_.popDataSet();

      AlohaPacket newPacket;
      StackHelper::deserializeAndStripDataset(frame, newPacket);

      // handle broadcast frames first
      if (newPacket.type() == AlohaPacket::BROADCAST) {
          LOG(LINFO) << "Received broadcast packet from " << newPacket.source();
          sendUpwards(frame);
      }

      // handle DATA and ACK only if they are for us
      if (newPacket.destination() == localAddress_x) {
        switch(newPacket.type()) {
        case AlohaPacket::DATA:
        {
          LOG(LINFO) << "Got DATA " << newPacket.seqno() << " from " << newPacket.source();
          sendAckPacket(newPacket.source(), newPacket.seqno());

          // check if packet contains new data
          if (newPacket.seqno() > rxSeqNo_ || newPacket.seqno() == 1) {
            // send new data packet up
            sendUpwards(frame);
            rxSeqNo_ = newPacket.seqno(); // update seqno
            if (newPacket.seqno() == 1) LOG(LINFO) << "Receiver restart detected.";
          }
          break;
        }
        case AlohaPacket::ACK:
        {
          LOG(LINFO) << "Got ACK  " << newPacket.seqno();
          boost::unique_lock<boost::mutex> lock(seqNoMutex_);
          if (newPacket.seqno() == txSeqNo_) {
            // received right ACK
            lock.unlock();
            ackArrivedCond_.notify_one();
          } else if (newPacket.seqno() > txSeqNo_) {
            LOG(LERROR) << "Received future ACK.";
          } else {
            LOG(LERROR) << "Received too old ACK";
          }
          break;
        }
        default:
          LOG(LERROR) << "Undefined packet type.";
          break;
        }
      }
    } // while
  }
  catch(IrisException& ex)
  {
    LOG(LFATAL) << "Error in AlohaMac component: " << ex.what() << " - Rx thread exiting.";
  }
  catch(boost::thread_interrupted)
  {
    LOG(LINFO) << "Thread " << boost::this_thread::get_id() << " in stack component interrupted.";
  }
}


void AlohaMacComponent::txThreadFunction()
{
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  LOG(LINFO) << "Tx thread started.";

  try
  {
    while(true)
    {
      boost::this_thread::interruption_point();

      boost::shared_ptr<StackDataSet> frame = txPktBuffer_.popDataSet();

      // determine frame source and destination
      std::string source(localAddress_x);
      std::string destination(destinationAddress_x);
#ifdef __unix__
      if (isEthernetDevice_x) {
        NetworkingHelper::getAddressFromEthernetFrame(frame, source, destination);
      }
#endif
      bool isBroadcast = (destination == BROADCAST_ADDRESS ? true : false);

      boost::unique_lock<boost::mutex> lock(seqNoMutex_);
      AlohaPacket dataPacket;
      dataPacket.set_source(source);
      dataPacket.set_destination(destination);
      if (isBroadcast) {
          dataPacket.set_type(AlohaPacket::BROADCAST);
      } else {
          dataPacket.set_type(AlohaPacket::DATA);
          dataPacket.set_seqno(txSeqNo_);
      }
      StackHelper::mergeAndSerializeDataset(frame, dataPacket);

      if (isBroadcast) {
          // send to PHY and we are done
          LOG(LINFO) << "Tx BROADCAST";
          sendDownwards(frame);
      } else {
          bool stop_signal = false;
          int txCounter = 1;
          while (!stop_signal) {
            // send packet to PHY
            LOG(LINFO) << "Tx DATA  " << txSeqNo_;
            sendDownwards(frame);

            // wait for ACK
            if (ackArrivedCond_.timed_wait(lock, boost::posix_time::milliseconds(ackTimeout_x)) == false) {
              // returns false if timeout was reached
              LOG(LINFO) << "ACK time out for " << txCounter << ". transmission of " << txSeqNo_;
              // wait random time before trying again, here between ackTimeout and 2*ackTimeout
              int collisionTimeout = rand() % ackTimeout_x;
              collisionTimeout = std::min(ackTimeout_x + collisionTimeout, 2 * ackTimeout_x);
              boost::this_thread::sleep(boost::posix_time::milliseconds(collisionTimeout));
            } else {
              // ACK received before timeout
              stop_signal = true;
            }

            if (++txCounter > maxRetry_x) stop_signal = true;
          }

          // increment seqno for next data packet
          txSeqNo_++;
          if (txSeqNo_ == std::numeric_limits<uint32_t>::max()) txSeqNo_ = 1;
      }

      lock.unlock();
    }
  }
  catch(IrisException& ex)
  {
    LOG(LFATAL) << "Error in AlohaMac component: " << ex.what() << " - Tx thread exiting.";
  }
  catch(boost::thread_interrupted)
  {
    LOG(LINFO) << "Thread " << boost::this_thread::get_id() << " in stack component interrupted.";
  }
}


void AlohaMacComponent::sendAckPacket(const string destination, uint32_t seqno)
{
  AlohaPacket ackPacket;
  ackPacket.set_source(localAddress_x);
  ackPacket.set_destination(destination);
  ackPacket.set_type(AlohaPacket::ACK);
  ackPacket.set_seqno(seqno);

  boost::shared_ptr<StackDataSet> buffer(new StackDataSet);
  StackHelper::mergeAndSerializeDataset(buffer, ackPacket);
  //StackHelper::printDataset(buffer, "ACK Tx");

  sendDownwards(buffer);
  LOG(LINFO) << "Tx  ACK  " << seqno;
}

} // namespace stack
} // namespace iris
