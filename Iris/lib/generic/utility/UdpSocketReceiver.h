/**
 * \file lib/generic/utility/UdpSocketReceiver.h
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
 * Utility class for a receiving UDP socket.
 */

#ifndef UDPSOCKETRECEIVER_H
#define UDPSOCKETRECEIVER_H

#include <irisapi/Logging.h>
#include <boost/asio.hpp>
#include <iterator>

namespace bip = boost::asio::ip;

/** A utility class to hold a receiving UDP socket.
 *
 *
 */
class UdpSocketReceiver
{
public:
  UdpSocketReceiver(int port)
    :port_(port)
  {
    //Create socket
    try
    {
      socket_ = new bip::udp::socket(ioService_);
      socket_->open(bip::udp::v4());
      socket_->bind(bip::udp::endpoint(bip::udp::v4(), port_));
    }
    catch(boost::system::system_error &e)
    {
      LOG(LERROR) << "Failed to create socket: " << e.what();
    }
  }

  ~UdpSocketReceiver()
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
  }

  template <typename Iterator>
  int read(Iterator begin, Iterator end)
  {
    int bufSize = (end-begin)*sizeof(typename std::iterator_traits<Iterator>::value_type);

    //Get data from socket
    std::size_t size;
    try
    {
      bip::udp::endpoint sender_endpoint;
      size = socket_->receive_from(boost::asio::buffer(&(*begin), bufSize),
                                   sender_endpoint);
    }
    catch(boost::system::system_error &e)
    {
        LOG(LERROR) << "Error receiving from socket: " << e.what();
    }

    return size;
  }

  std::size_t available()
  {
    return socket_->available();
  }

  std::string getName()
  {return "UdpSocketReceiver";}

private:
  unsigned short port_;      ///< The port to receive from.
  boost::asio::io_service ioService_;
  boost::asio::ip::udp::socket* socket_;
};

#endif // UDPSOCKETRECEIVER_H

