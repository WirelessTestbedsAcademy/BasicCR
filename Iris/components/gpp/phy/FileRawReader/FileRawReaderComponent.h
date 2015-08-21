/**
 * \file components/gpp/phy/FileRawReader/FileRawReaderComponent.h
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
 * Source component to read data from file.
 */

#ifndef PHY_FILERAWREADERCOMPONENT_H_
#define PHY_FILERAWREADERCOMPONENT_H_

#include <fstream>

#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A PhyComponent to read data from a named file.
 *
 * The FileRawReaderComponent reads raw data from a named file
 * and interprets it as a given data type. The size of blocks
 * to read and the data endianness can be specified using parameters.
 */
class FileRawReaderComponent
  : public PhyComponent
{
 public:
  FileRawReaderComponent(std::string name);
  virtual void calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();

 private:
  /// Template function used to read the data
  template<typename T> void readBlock();

  int blockSize_x;          ///< Size of blocks to read from file
  std::string fileName_x;   ///< Name of file to read
  std::string dataType_x;   ///< Interpret the data as this data type
  std::string endian_x;     ///< Endianness of the data
  uint32_t delay_x;         ///< Time to wait between blocks.

  std::ifstream hInFile_;   ///< The file stream
};

} // namespace phy
} // namespace iris
#endif // PHY_FILERAWREADERCOMPONENT_H_

