/**
 * \file components/gpp/phy/FileRawWriter/FileRawWriterComponent.cpp
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
 * Implementation of a sink component to write data to file.
 */

#include "FileRawWriterComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "irisapi/TypeVectors.h"
#include "utility/RawFileUtility.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, FileRawWriterComponent);

FileRawWriterComponent::FileRawWriterComponent(string name)
  : PhyComponent(name,
                "filerawwriter",
                "A filewriter",
                "Paul Sutton",
                "0.1")
{
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
  registerParameter("endian",
                    "Endianess of file (little|big|native)",
                    "native",
                    false,
                    endian_x);
}

void FileRawWriterComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes = convertToTypeIdVector<IrisDataTypes>();

  /*
   *  format:
   * registerInputPort(name, vector of valid types)
   */
  registerInputPort("input1", validTypes);
}

void FileRawWriterComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //No output
}

void FileRawWriterComponent::initialize()
{
  hOutFile_.open(fileName_x.c_str(), ios::out|ios::binary);
  if (hOutFile_.fail() || hOutFile_.bad() || !hOutFile_.is_open())
  {
    cout << "Could not open file " << fileName_x << " for writing." << endl;
    //TODO: Throw an exception
  }
}

void FileRawWriterComponent::process()
{
  if( outputBuffers.size() != 0 || inputBuffers.size() != 1)
  {
      //Need to throw an exception here
  }

  switch(inputBuffers[0]->getTypeIdentifier())
  {
    case 0:
      writeBlock<uint8_t>();
      break;
    case 1:
      writeBlock<uint16_t>();
      break;
    case 2:
      writeBlock<uint32_t>();
      break;
    case 3:
      writeBlock<uint64_t>();
      break;
    case 4:
      writeBlock<int8_t>();
      break;
    case 5:
      writeBlock<int16_t>();
      break;
    case 6:
      writeBlock<int32_t>();
      break;
    case 7:
      writeBlock<int64_t>();
      break;
    case 8:
      writeBlock<float>();
      break;
    case 9:
      writeBlock<double>();
      break;
    case 10:
      writeBlock<long double>();
      break;
    case 11:
      writeBlock< complex<float> >();
      break;
    case 12:
      writeBlock< complex<double> >();
      break;
    case 13:
      writeBlock<complex< long double> >();
      break;
    default:
      break;
  }
}

template<typename T>
void FileRawWriterComponent::writeBlock()
{
  //Get the input buffer
  ReadBuffer< T >* inBuf = castToType<T>(inputBuffers[0]);
  DataSet<T>* readDataSet = NULL;
  inBuf->getReadData(readDataSet);

  //Write to file
  RawFileUtility::write(readDataSet->data.begin(), readDataSet->data.end(),
                        hOutFile_, endian_x);

  //Release data set
  inBuf->releaseReadData(readDataSet);
}

FileRawWriterComponent::~FileRawWriterComponent()
{
  hOutFile_.close();
}

} // namespace phy
} // namespace iris
