/**
 * \file components/gpp/phy/UdpSocketRx/UdpSocketRxComponent.h
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
 * A source component which listens to a UDP socket.
 */

#ifndef PHY_UDPSOCKETRXCOMPONENT_H_
#define PHY_UDPSOCKETRXCOMPONENT_H_

#include "irisapi/PhyComponent.h"

//For boost asio sockets
#include <boost/asio.hpp>

namespace iris
{
namespace phy
{

/** A PhyComponent which receives from a UDP socket.
 *
 * The UdpSocketRxComponent receives data from a UDP socket. The port
 * number, buffer size and data type can be set using parameters.
 */
class UdpSocketRxComponent
  : public PhyComponent
{
public:
  UdpSocketRxComponent(std::string name);
  ~UdpSocketRxComponent();
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void start();
  virtual void process();
  virtual void stop();

private:
  /// Template function to write output.
  template<typename T> void writeOutput();

  unsigned short port_x;      ///< The port to receive from.
  unsigned int bufferSize_x;  ///< Size of the buffer used to receive datagrams.
  std::string outputType_x;   ///< The data type of output data.

  int outputTypeId_;
  boost::asio::io_service ioService_;
  boost::asio::ip::udp::socket* socket_;
  char* buffer_;
  bool bStopping_;
};

} // namespace phy
} // namespace iris

#endif // PHY_UDPSOCKETRXCOMPONENT_H_
