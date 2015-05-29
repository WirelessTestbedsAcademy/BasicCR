/**
 * \file lib/generic/utility/MatlabPlotter.cpp
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
 * Implementation of the MatlabPlotter class.
 * MatlabPlotter is built as a shared library - this allows us to
 * use an rpath to specify the directory containing matlab-specific
 * libraries.
 */

#include "engine.h"
#include "MatlabPlotter.h"
#include <irisapi/Exceptions.h>
#include <sstream>

MatlabPlotter::MatlabPlotter()
  :engine_(NULL)
{
  engine_ = (void*)engOpen("");
  if(!engine_)
  {
    std::cout << "[ERROR] MatlabPlotter: Failed to open matlab engine." << std::endl;
    throw iris::ResourceNotFoundException("Failed to start Matlab engine");
  }
}

MatlabPlotter::~MatlabPlotter()
{
  engClose((Engine*)engine_);
}

void* MatlabPlotter::createMxArray(int s, double*& pR, double*& pI)
{
  void* a = (void*)mxCreateDoubleMatrix(1, s, mxCOMPLEX);
  pR = mxGetPr((mxArray*)a);
  pI = mxGetPi((mxArray*)a);
  return a;
}

void* MatlabPlotter::createMxArray(int s, double*& pR)
{
  void* a = (void*)mxCreateDoubleMatrix(1, s, mxREAL);
  pR = mxGetPr((mxArray*)a);
  return a;
}

void MatlabPlotter::destroyMxArray(void* array)
{
  mxDestroyArray((mxArray*)array);
}

void MatlabPlotter::plot(void* array, std::string title, int id)
{
  //Send data to matlab
  engPutVariable((Engine*)engine_, "data", (mxArray*)array);

  //Use OutputBuffer to capture MATLAB output
  memset(buffer_, 0, 256 * sizeof(char));
  engOutputBuffer((Engine*)engine_, buffer_, 256);

  //Process in Matlab
  std::string command;
  if(mxIsComplex((mxArray*)array))
    command = buildComplexCommand(title, id);
  else
    command = buildScalarCommand(title, id);

  engEvalString((Engine*)engine_, command.c_str());

  //The evaluate string returns the result into the output buffer
  if (buffer_[0] != 0)
  {
    std::cout << "[WARNING} MatlabPlotter: " << buffer_ <<std::endl;
  }
}

std::string MatlabPlotter::buildScalarCommand(std::string title, int id)
{
  std::stringstream ss;
  if(id==0)
    ss << "figure;";
  else
    ss << "figure(" << id << ");";

  ss << "plot(data);title('" << title << "');";
  return ss.str();
}
std::string MatlabPlotter::buildComplexCommand(std::string title, int id)
{
  std::stringstream ss;
  if(id==0)
    ss << "figure;";
  else
    ss << "figure(" << id << ");";

  ss << "subplot(4,1,1);plot(real(data));title('" << title << " - Real'); \
      subplot(4,1,2);plot(imag(data));title('" << title << " - Imag'); \
      subplot(4,1,3);plot(abs(data));title('" << title << " - Magnitude'); \
      subplot(4,1,4);plot(angle(data));title('" << title << " - Phase');";
  return ss.str();
}

