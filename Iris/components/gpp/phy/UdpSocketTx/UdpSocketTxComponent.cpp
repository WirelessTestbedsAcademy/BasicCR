/**
 * \file components/gpp/phy/UdpSocketTx/UdpSocketTxComponent.cpp
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
 * Implementation of a sink component which writes to a UDP socket.
 */

#include "UdpSocketTxComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace boost::asio::ip;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, UdpSocketTxComponent);

UdpSocketTxComponent::UdpSocketTxComponent(string name)
  : PhyComponent(name,
                "udpsockettx",
                "A UDP socket tx",
                "Paul Sutton",
                "0.1")
{
  //Register all parameters
  /*
   * format:
   * registerParameter(name,
   *                   description,
   *                   default value,
   *                   dynamic?,
   *                   parameter,
   *                   allowed values)
   */
  registerParameter("address",
                    "Address of the target machine",
                    "127.0.0.1",
                    false,
                    address_x);
  registerParameter("port",
                    "Port of the target machine",
                    "1234",
                    false,
                    port_x);
  socket_ = NULL;
  endPoint_ = NULL;
}

void UdpSocketTxComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes; //This component supports all data types
  validTypes.push_back( int(TypeInfo< uint8_t >::identifier) );
  validTypes.push_back( int(TypeInfo< uint16_t >::identifier) );
  validTypes.push_back( int(TypeInfo< uint32_t >::identifier) );
  validTypes.push_back( int(TypeInfo< uint64_t >::identifier) );
  validTypes.push_back( int(TypeInfo< int8_t >::identifier) );
  validTypes.push_back( int(TypeInfo< int16_t >::identifier) );
  validTypes.push_back( int(TypeInfo< int32_t >::identifier) );
  validTypes.push_back( int(TypeInfo< int64_t >::identifier) );
  validTypes.push_back( int(TypeInfo< float >::identifier) );
  validTypes.push_back( int(TypeInfo< double >::identifier) );
  validTypes.push_back( int(TypeInfo< long double >::identifier) );
  validTypes.push_back( int(TypeInfo< complex<float> >::identifier) );
  validTypes.push_back( int(TypeInfo< complex<double> >::identifier) );
  validTypes.push_back( int(TypeInfo< complex<long double> >::identifier) );

  //format:        (name, vector of valid types)
  registerInputPort("input1", validTypes);
}

void UdpSocketTxComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //No output
}

void UdpSocketTxComponent::initialize()
{
  //Create socket
  try
  {
      socket_ = new boost::asio::ip::udp::socket(ioService_);
      socket_->open(udp::v4());
      endPoint_ = new udp::endpoint(address::from_string(address_x), port_x);
  }
  catch(boost::system::system_error &e)
  {
      LOG(LERROR) << "Failed to create socket: " << e.what();
  }
}

void UdpSocketTxComponent::process()
{
  if( outputBuffers.size() != 0 || inputBuffers.size() != 1)
  {
    //Need to throw an exception here
  }

  switch(inputBuffers[0]->getTypeIdentifier())
  {
    case 0:
      writeOutput<uint8_t>();
      break;
    case 1:
      writeOutput<uint16_t>();
      break;
    case 2:
      writeOutput<uint32_t>();
      break;
    case 3:
      writeOutput<uint64_t>();
      break;
    case 4:
      writeOutput<int8_t>();
      break;
    case 5:
      writeOutput<int16_t>();
      break;
    case 6:
      writeOutput<int32_t>();
      break;
    case 7:
      writeOutput<int64_t>();
      break;
    case 8:
      writeOutput<float>();
      break;
    case 9:
      writeOutput<double>();
      break;
    case 10:
      writeOutput<long double>();
      break;
    case 11:
      writeOutput< complex<float> >();
      break;
    case 12:
      writeOutput< complex<double> >();
      break;
    case 13:
      writeOutput<complex< long double> >();
      break;
    default:
      break;
  }
}

template<typename T>
void UdpSocketTxComponent::writeOutput()
{
  //Get a read buffer
  ReadBuffer<T>* inBuf = castToType<T>(inputBuffers[0]);
  DataSet<T>* readDataSet = NULL;
  inBuf->getReadData(readDataSet);

  try
  {
    socket_->send_to(boost::asio::buffer(readDataSet->data), *endPoint_);
  }
  catch (boost::system::system_error &e)
  {
    LOG(LERROR) << "An error occurred while sending data to " << address_x << ", port " << port_x << \
        ": " << e.what();
  }

  inBuf->releaseReadData(readDataSet);
}

UdpSocketTxComponent::~UdpSocketTxComponent()
{
  try
  {
    socket_->shutdown(udp::socket::shutdown_send);
    socket_->close();
  }
  catch (boost::system::system_error &e)
  {
    LOG(LERROR) << "An error occurred closing socket: " << e.what();
  }
  delete socket_;
  delete endPoint_;
}


} // namespace phy
} // namespace iris
