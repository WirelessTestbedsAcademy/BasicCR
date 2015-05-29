/**
 * \file components/gpp/phy/FileRawWriterTemplate/FileRawWriterTemplateComponent.cpp
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
 * Implementation of a template sink component used to write data to file.
 */

#include "FileRawWriterTemplateComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "irisapi/TypeVectors.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, FileRawWriterTemplateComponent);

FileRawWriterTemplateComponent::FileRawWriterTemplateComponent(string name)
  : TemplatePhyComponent<FileRawWriterTemplateComponent>(
      name,
      "filerawwritertemplate",
      "A templated filewriter",
      "Paul Sutton & Jorg Lotze",
      "0.2")
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
  registerParameter("fileName",
                    "The file to read",
                    "temp.bin",
                    false,
                    fileName_x);
}

void FileRawWriterTemplateComponent::registerPorts()
{
  //Register all ports
  vector<int> validTypes = convertToTypeIdVector<IrisDataTypes>();
  //format: registerInputPort(name, vector of valid types)
  registerInputPort("input1", validTypes);
}

void FileRawWriterTemplateComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //No output
}

template <class Tin, class Tout>
void FileRawWriterTemplateComponentImpl<Tin,Tout>::initialize()
{
  hOutFile_.open(fileName_x.c_str(), ios::out|ios::binary);
  if (hOutFile_.fail() || hOutFile_.bad() || !hOutFile_.is_open())
  {
    cout << "Could not open file " << fileName_x << " for writing." << endl;
    //TODO: Throw an exception
  }
}

template <class Tin, class Tout>
void FileRawWriterTemplateComponentImpl<Tin,Tout>::process()
{
  inBuf_ = castToType<T>(inputBuffers[0]);

  //Get the input buffer
  inBuf_->getReadData(readDataSet_);

  //Write to file
  hOutFile_.write((char*)&readDataSet_->data[0],
                  (streamsize)readDataSet_->data.size()*sizeof(T));

  //Release data set
  inBuf_->releaseReadData(readDataSet_);
}

template <class Tin, class Tout>
FileRawWriterTemplateComponentImpl<Tin,Tout>::~FileRawWriterTemplateComponentImpl()
{
  hOutFile_.close();
}

} // namespace phy
} // namespace iris
