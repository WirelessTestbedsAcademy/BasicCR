/**
 * \file lib/generic/utility/UdpSocketTransmitter.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012 The Iris Project Developers. See the
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
 * Utility class for a transmitting UDP socket.
 */

#ifndef UDPSOCKETTRANSMITTER_H
#define UDPSOCKETTRANSMITTER_H

#include <irisapi/Logging.h>
#include <boost/asio.hpp>
#include <iterator>

namespace bip = boost::asio::ip;

/** A utility class to hold a transmitting UDP socket.
 *
 *
 */
class UdpSocketTransmitter
{
public:
  UdpSocketTransmitter(std::string address, int port)
    :address_(address)
    ,port_(port)
  {
    //Create socket
    try
    {
      socket_ = new bip::udp::socket(ioService_);
      socket_->open(bip::udp::v4());
      endPoint_ = new bip::udp::endpoint(bip::address::from_string(address_),
                                         port_);
    }
    catch(boost::system::system_error &e)
    {
      LOG(LERROR) << "Failed to create socket: " << e.what();
    }
  }

  ~UdpSocketTransmitter()
  {
    //Close socket
    try
    {
      socket_->close();
    }
    catch(boost::system::system_error &e)
    {
        LOG(LERROR) << "Failed to close socket: " << e.what();
    }
    delete socket_;
    delete endPoint_;
  }

  template <typename Iterator>
  void write(Iterator begin, Iterator end)
  {
    int bufSize = (end-begin)*sizeof(typename std::iterator_traits<Iterator>::value_type);
    //Send data to socket
    try
    {
      socket_->send_to(boost::asio::buffer(&(*begin), bufSize), *endPoint_);
    }
    catch(boost::system::system_error &e)
    {
        LOG(LERROR) << "Error sending data to socket: " << e.what();
    }
  }

  std::string getName()
  {return "UdpSocketTransmitter";}

private:
  std::string address_;      ///< The address to transmit to.
  unsigned short port_;      ///< The port to transmit to.
  boost::asio::io_service ioService_;
  boost::asio::ip::udp::socket* socket_;
  boost::asio::ip::udp::endpoint* endPoint_;
};

#endif // UDPSOCKETTRANSMITTER_H

