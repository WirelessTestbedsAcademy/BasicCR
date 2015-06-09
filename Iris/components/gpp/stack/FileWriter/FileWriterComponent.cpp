/**
 * \file components/gpp/stack/FileWriter/FileWriterComponent.h
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
 * Implementation of a sink stack component which writes data to file.
 */

#include "FileWriterComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "utility/RawFileUtility.h"

using namespace std;

namespace iris
{
namespace stack
{

// export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, FileWriterComponent);

FileWriterComponent::FileWriterComponent(std::string name)
  : StackComponent(name,
                   "filewriter",
                   "A simple stack component for writing bytes to a file",
                   "Colman O'Sullivan & Paul Sutton",
                   "0.1")
{
  registerParameter("filename",
                    "The file to write",
                    "temp.bin",
                    false,
                    fileName_x);
  registerParameter("frombelow",
                    "Write data from below rather than above",
                    "true",
                    false,
                    fromBelow_x);
}

void FileWriterComponent::initialize()
{
  hOutFile_.open(fileName_x.c_str(), ios::out|ios::binary);
  if (hOutFile_.fail() || hOutFile_.bad() || !hOutFile_.is_open())
  {
    LOG(LFATAL) << "Could not open file " << fileName_x << " for writing.";
    throw ResourceNotFoundException("Could not open file " + fileName_x + " for writing.");
  }
}

void FileWriterComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> set)
{
  if(!fromBelow_x)
  {
    writeBlock(set);
  }
  sendDownwards(set);
}

void FileWriterComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> set)
{
  if(fromBelow_x)
  {
    writeBlock(set);
  }
  sendUpwards(set);
}


void FileWriterComponent::writeBlock(boost::shared_ptr<StackDataSet> toWrite)
{
  RawFileUtility::write(toWrite->data.begin(), toWrite->data.end(), hOutFile_);
}

} // namespace stack
} // namespace iris
