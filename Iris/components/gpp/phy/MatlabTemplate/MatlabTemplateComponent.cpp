/**
 * \file components/gpp/phy/MatlabTemplate/MatlabTemplateComponent.cpp
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
 * Implementation of a template component used to read/write data to
 * a matlab script.
 */

#include "MatlabTemplateComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "irisapi/TypeVectors.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, MatlabTemplateComponent);

MatlabTemplateComponent::MatlabTemplateComponent(string name)
  : TemplatePhyComponent<MatlabTemplateComponent>(
      name,
      "matlabtemplate",
      "A component which interfaces to a Matlab script",
      "Paul Sutton",
      "0.1")
    ,matlabInput_(NULL)
    ,matlabOutput_(NULL)
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
  registerParameter("scriptName",
                    "The Matlab script to be called",
                    "Plot(matlab_input)",
                    false,
                    scriptName_x);
  registerParameter("hasOutput",
                    "Does this component output data?",
                    "false",
                    false,
                    hasOutput_x);
  registerParameter("passThrough",
                    "Does this component pass data through?",
                    "false",
                    false,
                    passThrough_x);
}

MatlabTemplateComponent::~MatlabTemplateComponent()
{
  mxDestroyArray(matlabInput_);
  mxDestroyArray(matlabOutput_);
}

void MatlabTemplateComponent::registerPorts()
{
  //This component accepts all the Iris data types
  vector<int> validTypes = convertToTypeIdVector<IrisDataTypes>();

  //format:        (name, vector of valid types)
  registerInputPort("input1", validTypes);

  if(hasOutput_x || passThrough_x)
  {
    registerOutputPort("output1", validTypes);
  }
}

void MatlabTemplateComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //Output type is the same as input type
  if(hasOutput_x || passThrough_x)
  {
    outputTypes["output1"] = inputTypes["input1"];
  }
}

template <class Tin, class Tout>
void MatlabTemplateComponentImpl<Tin,Tout>::initialize()
{
  inBuf_ = castToType<T>(inputBuffers[0]);
  if(hasOutput_x || passThrough_x)
  {
    outBuf_ = castToType<T>(outputBuffers[0]);
  }

  //Open Matlab Engine
  if(!matlab_.open(""))
  {
    throw ResourceNotFoundException("Failed to start Matlab engine");
  }

  //Clear Matlab WorkSpace
  matlab_.evalString("clear all;");

  //Write the matlab command
  command_ = "matlab_output = ";
  command_ += scriptName_x;
  command_ += ";";

  //Setup an initial mxArray
  matlabInput_ = mxCreateDoubleMatrix(1, 10, mxREAL);
}

template <class Tin, class Tout>
void MatlabTemplateComponentImpl<Tin,Tout>::process()
{
  //Get the input buffer
  inBuf_->getReadData(readDataSet_);

  //Check dimensions of our matlab matrix
  const mwSize* dims = mxGetDimensions(matlabInput_);
  size_t s = readDataSet_->data.size();
  if(s != dims[0])
  {
    mxDestroyArray(matlabInput_);
    if(TypeInfo<T>::isComplex)
    {
      matlabInput_ = mxCreateDoubleMatrix(1, s, mxCOMPLEX);
    }
    else
    {
      matlabInput_ = mxCreateDoubleMatrix(1, s, mxREAL);
    }
  }

  //Copy data into our matlab matrix
  copyInData(readDataSet_->data, matlabInput_);

//bug in matlab windows - has to be reopened every time
#ifdef _WIN32 // _WIN32 is defined by all Windows 32 compilers, but not by others.
  if(!matlab_.open(""))
  {
    throw ResourceNotFoundException("Failed to start Matlab engine");
  }
#endif

  //Send data to matlab
  matlab_.putVariable("matlab_input", matlabInput_);

  //Use OutputBuffer to capture MATLAB output
  memset(buffer_, 0, 256 * sizeof(char));
  matlab_.outputBuffer(buffer_, 256);

  //Process in Matlab
  matlab_.evalString(command_.c_str());

  //The evaluate string returns the result into the output buffer
  if (buffer_[0] != 0)
  {
    LOG(LWARNING) << buffer_;
  }

  //Output data if required
  if(hasOutput_x || passThrough_x)
  {
    if(passThrough_x)
    {
      //Get a write data set
      outBuf_->getWriteData(writeDataSet_, readDataSet_->data.size());
      //Copy data through
      copy(readDataSet_->data.begin(), readDataSet_->data.end(), writeDataSet_->data.begin());
      writeDataSet_->sampleRate = readDataSet_->sampleRate;
      writeDataSet_->timeStamp = readDataSet_->timeStamp;
    }
    else
    {
      //Get the matlab output
      mxArray* matlab_output = matlab_.getVariable("matlab_output");
      size_t m = mxGetM(matlab_output);
      size_t n = mxGetN(matlab_output);
      //Get a write data set of the correct size and copy data
      outBuf_->getWriteData(writeDataSet_, m>n?m:n);
      copyOutData(matlab_output, writeDataSet_->data);
    }

    //Release write data set
    outBuf_->releaseWriteData(writeDataSet_);
  }

  //Release read data set
  inBuf_->releaseReadData(readDataSet_);
}

template <class Tin, class Tout>
MatlabTemplateComponentImpl<Tin,Tout>::~MatlabTemplateComponentImpl()
{}

} // namespace phy
} // namespace iris
