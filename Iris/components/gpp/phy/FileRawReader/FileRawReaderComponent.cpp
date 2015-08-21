/**
 * \file components/gpp/phy/FileRawReader/FileRawReaderComponent.cpp
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
 * Implementation of a source component to read data from file.
 */

#include "FileRawReaderComponent.h"

#include <algorithm>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>

#include "utility/EndianConversion.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, FileRawReaderComponent);

FileRawReaderComponent::FileRawReaderComponent(string name)
  : PhyComponent(name,
                "FileRawReader",
                "A filereader",
                "Paul Sutton",
                "0.2")
{
  list<string> allowedTypes;
  allowedTypes.push_back(TypeInfo< uint8_t >::name());
  allowedTypes.push_back(TypeInfo< uint16_t >::name());
  allowedTypes.push_back(TypeInfo< uint32_t >::name());
  allowedTypes.push_back(TypeInfo< uint64_t >::name());
  allowedTypes.push_back(TypeInfo< int8_t >::name());
  allowedTypes.push_back(TypeInfo< int16_t >::name());
  allowedTypes.push_back(TypeInfo< int32_t >::name());
  allowedTypes.push_back(TypeInfo< int64_t >::name());
  allowedTypes.push_back(TypeInfo< float >::name());
  allowedTypes.push_back(TypeInfo< double >::name());
  allowedTypes.push_back(TypeInfo< long double >::name());
  allowedTypes.push_back(TypeInfo< complex<float> >::name());
  allowedTypes.push_back(TypeInfo< complex<double> >::name());
  allowedTypes.push_back(TypeInfo< complex<long double> >::name());

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
  registerParameter("filename",
                    "The file to read",
                    "temp.bin",
                    false,
                    fileName_x);
  registerParameter("blocksize",
                    "Size of output blocks",
                    "1024",
                    false,
                    blockSize_x,
                    Interval<int>(1, 1024000));
  registerParameter("datatype",
                    "Type of data in file",
                    "uint8_t",
                    false,
                    dataType_x,
                    allowedTypes);
  registerParameter("endian",
                    "Endianess of file (little|big|native)",
                    "native",
                    false,
                    endian_x);
  registerParameter("delay",
                    "Delay in us between reads",
                    "0",
                    true,
                    delay_x,
                    Interval<uint32_t>(0,5000000));
}

void FileRawReaderComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes;
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

void FileRawReaderComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //Set output type
  if( dataType_x == "uint8_t" )
    outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
  if( dataType_x == "uint16_t" )
    outputTypes["output1"] = TypeInfo< uint16_t >::identifier;
  if( dataType_x == "uint32_t" )
    outputTypes["output1"] = TypeInfo< uint32_t >::identifier;
  if( dataType_x == "uint64_t" )
    outputTypes["output1"] = TypeInfo< uint64_t >::identifier;
  if( dataType_x == "int8_t" )
    outputTypes["output1"] = TypeInfo< int8_t >::identifier;
  if( dataType_x == "int16_t" )
    outputTypes["output1"] = TypeInfo< int16_t >::identifier;
  if( dataType_x == "int32_t" )
    outputTypes["output1"] = TypeInfo< int32_t >::identifier;
  if( dataType_x == "int64_t" )
    outputTypes["output1"] = TypeInfo< int64_t >::identifier;
  if( dataType_x == "float" )
    outputTypes["output1"] = TypeInfo< float >::identifier;
  if( dataType_x == "double" )
    outputTypes["output1"] = TypeInfo< double >::identifier;
  if( dataType_x == "long double" )
    outputTypes["output1"] = TypeInfo< long double >::identifier;
  if( dataType_x == "complex<float>" )
    outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
  if( dataType_x == "complex<double>" )
    outputTypes["output1"] = TypeInfo< complex<double> >::identifier;
  if( dataType_x == "complex<long double>" )
    outputTypes["output1"] = TypeInfo< complex<long double> >::identifier;
}

void FileRawReaderComponent::initialize()
{
  //Open the file and retrieve its size
  hInFile_.open(fileName_x.c_str(), ios::in|ios::binary|ios::ate);
  if(hInFile_.fail() || hInFile_.bad() || !hInFile_.is_open())
  {
    LOG(LFATAL) << "Could not open file " << fileName_x << " for reading.";
    throw ResourceNotFoundException(
        "Could not open file " + fileName_x + " for reading.");
  }
  hInFile_.seekg(0, ios::beg);
}

void FileRawReaderComponent::process()
{
  switch (outputBuffers[0]->getTypeIdentifier())
  {
    case TypeInfo<uint8_t>::identifier:
      readBlock<uint8_t> ();
      break;
    case TypeInfo<uint16_t>::identifier:
      readBlock<uint16_t> ();
      break;
    case TypeInfo<uint32_t>::identifier:
      readBlock<uint32_t> ();
      break;
    case TypeInfo<uint64_t>::identifier:
      readBlock<uint64_t> ();
      break;
    case TypeInfo<int8_t>::identifier:
      readBlock<int8_t> ();
      break;
    case TypeInfo<int16_t>::identifier:
      readBlock<int16_t> ();
      break;
    case TypeInfo<int32_t>::identifier:
      readBlock<int32_t> ();
      break;
    case TypeInfo<int64_t>::identifier:
      readBlock<int64_t> ();
      break;
    case TypeInfo<float>::identifier:
      readBlock<float> ();
      break;
    case TypeInfo<double>::identifier:
      readBlock<double> ();
      break;
    case TypeInfo<long double>::identifier:
      readBlock<long double> ();
      break;
    case TypeInfo<complex<float> >::identifier:
      readBlock<complex<float> > ();
      break;
    case TypeInfo<complex<double> >::identifier:
      readBlock<complex<double> > ();
      break;
    case TypeInfo<complex<long double> >::identifier:
      readBlock<complex<long double> > ();
      break;
    default:
      break;
  }
  boost::this_thread::sleep(boost::posix_time::microseconds(delay_x));
}

template<typename T>
void FileRawReaderComponent::readBlock()
{
  //Get the output buffer & work directly on it
  WriteBuffer< T >* outBuf = castToType<T>(outputBuffers[0]);
  DataSet<T>* writeDataSet = NULL;
  outBuf->getWriteData(writeDataSet, blockSize_x);

  char *bytebuf = reinterpret_cast<char*>(&writeDataSet->data[0]);
  ifstream::pos_type toread = blockSize_x * sizeof(T);

  //Read a block (loop if necessary)
  while( toread > 0 )
  {
    hInFile_.read(bytebuf, toread);
    toread -= hInFile_.gcount();
    bytebuf += hInFile_.gcount();
    if( hInFile_.eof() )
    {
      hInFile_.clear();
      hInFile_.seekg(0, ios::beg);
    }
  }

  if (sizeof(T) > 1)
  {
    //Convert endianess
    if (endian_x == "native")
      ; // nothing to do
    else if (endian_x == "little")
      transform(writeDataSet->data.begin(), writeDataSet->data.end(),
                writeDataSet->data.begin(), lit2sys<T> );
    else if (endian_x == "big")
      transform(writeDataSet->data.begin(), writeDataSet->data.end(),
                writeDataSet->data.begin(), big2sys<T> );
  }

  outBuf->releaseWriteData(writeDataSet);
}

} // namespace phy
} // namespace iris
