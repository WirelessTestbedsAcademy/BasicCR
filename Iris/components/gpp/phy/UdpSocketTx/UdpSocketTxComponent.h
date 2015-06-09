/**
 * \file components/gpp/phy/UdpSocketTx/UdpSocketTxComponent.h
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
 * A sink component which writes to a UDP socket.
 */

#ifndef PHY_UDPSOCKETTXCOMPONENT_H_
#define PHY_UDPSOCKETTXCOMPONENT_H_

#include "irisapi/PhyComponent.h"

//For boost asio sockets
#include <boost/asio.hpp>

namespace iris
{
namespace phy
{

/** A PhyComponent which transmits data over a UDP socket.
 *
 * The UdpSocketTxComponent transmits data over a UDP socket
 * to a specified IP address and port.
 */
class UdpSocketTxComponent
  : public PhyComponent
{
public:
  UdpSocketTxComponent(std::string name);
  ~UdpSocketTxComponent();
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();

private:
  /// Template function to write output.
  template<typename T> void writeOutput();

  std::string address_x;  //!< The IP address to send to
  unsigned short port_x;  //!< The destination port number

  boost::asio::io_service ioService_;
  boost::asio::ip::udp::socket* socket_;
  boost::asio::ip::udp::endpoint* endPoint_;

};

} // namespace phy
} // namespace iris

#endif // PHY_UDPSOCKETTXCOMPONENT_H_
