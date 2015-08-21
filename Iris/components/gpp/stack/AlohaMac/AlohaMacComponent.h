/**
 * \file components/gpp/stack/AlohaMac/AlohaMacComponent.h
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

#ifndef STACK_ALOHAMACCOMPONENT_H_
#define STACK_ALOHAMACCOMPONENT_H_

#include "irisapi/StackComponent.h"
#include <stdio.h>
#include "alohamac.pb.h"

#define BROADCAST_ADDRESS "ffffffffffff"

namespace iris
{
namespace stack
{

class AlohaMacComponent
  : public StackComponent
{
public:
  AlohaMacComponent(std::string name);
  virtual ~AlohaMacComponent();

  virtual void initialize();
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
  virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

  virtual void start();
  virtual void stop();

private:
  //Exposed parameters
  std::string localAddress_x;         ///< Source address of this client (may get overwritten)
  std::string destinationAddress_x;   ///< Address of destination client
  bool isEthernetDevice_x;            ///< Whether to interpret incoming frames as Ethernet frames
  std::string ethernetDeviceName_x;   ///< Name of the Ethernet device to use (e.g. tap0)
  int ackTimeout_x;                   ///< Time to wait for ACK packets (ms)
  int maxRetry_x;                     ///< Number of retransmissions

  // local variables
  StackDataBuffer rxPktBuffer_, txPktBuffer_;
  uint32_t txSeqNo_;          ///< sequence number of outgoing data packets
  uint32_t rxSeqNo_;          ///< sequence number of incoming data packets
  boost::condition_variable ackArrivedCond_;
  boost::mutex seqNoMutex_;

  // thread pointers
  boost::scoped_ptr< boost::thread > rxThread_, txThread_;

  // private functions
  void sendAckPacket(const std::string destination, uint32_t seqno);
  void rxThreadFunction();
  void txThreadFunction();
};

} // namespace stack
} // namespace iris

#endif // STACK_ALOHAMACCOMPONENT_H_
