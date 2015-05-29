/**
 * \file components/gpp/phy/UdpSocketRx/UdpSocketRxComponent.cpp
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
 * Implementation of a source component which listens to a UDP socket.
 */

#include "UdpSocketRxComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace boost::asio::ip;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, UdpSocketRxComponent);

UdpSocketRxComponent::UdpSocketRxComponent(string name)
  : PhyComponent(name,
                "udpsocketrx",
                "A udp socket rx",
                "Paul Sutton",
                "0.1")
  ,bStopping_(false)
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
  registerParameter("port",
                    "Port to listen on",
                    "1234",
                    false,
                    port_x);
  registerParameter("bufferSize",
                    "The size of the buffer used to receive datagrams",
                    "1316",
                    false,
                    bufferSize_x);
  registerParameter("outputType",
                    "The type of the output",
                    "uint8_t",
                    false,
                    outputType_x);
}

void UdpSocketRxComponent::registerPorts()
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
  registerOutputPort("output1", validTypes);
}

void UdpSocketRxComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //Output type is set in the parameters
  if( outputType_x == TypeInfo< uint8_t >::name() )
    outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
  if( outputType_x == TypeInfo< uint16_t >::name() )
    outputTypes["output1"] = TypeInfo< uint16_t >::identifier;
  if( outputType_x == TypeInfo< uint32_t >::name() )
    outputTypes["output1"] = TypeInfo< uint32_t >::identifier;
  if( outputType_x == TypeInfo< uint64_t >::name() )
    outputTypes["output1"] = TypeInfo< uint64_t >::identifier;
  if( outputType_x == TypeInfo< int8_t >::name() )
    outputTypes["output1"] = TypeInfo< int8_t >::identifier;
  if( outputType_x == TypeInfo< int16_t >::name() )
    outputTypes["output1"] = TypeInfo< int16_t >::identifier;
  if( outputType_x == TypeInfo< int32_t >::name() )
    outputTypes["output1"] = TypeInfo< int32_t >::identifier;
  if( outputType_x == TypeInfo< int64_t >::name() )
    outputTypes["output1"] = TypeInfo< int64_t >::identifier;
  if( outputType_x == TypeInfo< float >::name() )
    outputTypes["output1"] = TypeInfo< float >::identifier;
  if( outputType_x == TypeInfo< double >::name() )
    outputTypes["output1"] = TypeInfo< double >::identifier;
  if( outputType_x == TypeInfo< long double >::name() )
    outputTypes["output1"] = TypeInfo< long double >::identifier;
  if( outputType_x == TypeInfo< complex<float> >::name() )
    outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
  if( outputType_x == TypeInfo< complex<double> >::name() )
    outputTypes["output1"] = TypeInfo< complex<double> >::identifier;
  if( outputType_x == TypeInfo< complex<long double> >::name() )
    outputTypes["output1"] = TypeInfo< complex<long double> >::identifier;

  outputTypeId_ = outputTypes["output1"];
}

void UdpSocketRxComponent::initialize()
{
  //Create our buffer
  buffer_ = new char[bufferSize_x];

  //Create socket
  try
  {
    socket_ = new boost::asio::ip::udp::socket(ioService_);
  }
  catch(boost::system::system_error &e)
  {
    LOG(LERROR) << "Failed to create socket: " << e.what();
  }
}

void UdpSocketRxComponent::start()
{
  //Open socket
  try
  {
    socket_->open(udp::v4());
    socket_->bind(udp::endpoint(udp::v4(), port_x));
  }
  catch(boost::system::system_error &e)
  {
    LOG(LERROR) << "Failed to open socket: " << e.what();
  }
}

void UdpSocketRxComponent::process()
{
  if( outputBuffers.size() != 1 || inputBuffers.size() != 0)
  {
    //Need to throw an exception here
  }

  switch(outputTypeId_)
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
void UdpSocketRxComponent::writeOutput()
{
  if(!bStopping_)
  {
    //Get data from socket
    std::size_t size;
    try
    {
      udp::endpoint sender_endpoint;
      size = socket_->receive_from(boost::asio::buffer(buffer_, bufferSize_x), sender_endpoint);
    }
    catch(boost::system::system_error &e)
    {
      if(!bStopping_)
      {
        LOG(LERROR) << "Error receiving from socket: " << e.what();
      }
      return;
    }

    //Check that we've an integer number of data types in the datagram
    float f = size/(float)sizeof(T);
    float intpart, rem;
    rem = modf(f, &intpart);
    if(rem != 0)
    {
      LOG(LERROR) << "Did not receive an integer number of elements - data will be lost";
    }
    int numT = (int)intpart;

    //Get the output buffer
    WriteBuffer< T >* outBuf = castToType<T>(outputBuffers[0]);
    DataSet<T>* writeDataSet = NULL;
    outBuf->getWriteData(writeDataSet, numT);

    //Copy data into output
    T* bufT = (T*)buffer_;
    copy(bufT, bufT+numT, writeDataSet->data.begin());

    //Release the buffer
    outBuf->releaseWriteData(writeDataSet);
  }
}

void UdpSocketRxComponent::stop()
{
  //Close socket
  try
  {
    bStopping_ = true;
    socket_->shutdown(udp::socket::shutdown_receive);
    socket_->close();
  }
  catch(boost::system::system_error &e)
  {
      LOG(LERROR) << "Failed to close socket: " << e.what();
  }
}

UdpSocketRxComponent::~UdpSocketRxComponent()
{
  //Destroy our buffer
  delete [] buffer_;

  //Destroy socket
  delete socket_;
}

} // namespace phy
} // namespace iris
