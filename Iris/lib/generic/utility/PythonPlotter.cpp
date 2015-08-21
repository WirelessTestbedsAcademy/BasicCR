/**
 * \file lib/generic/utility/PythonPlotter.cpp
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
 * Implementation of the PythonPlotter class.
 */

#include "PythonPlotter.h"
#include "Python.h"
#include <sstream>

PythonPlotter::PythonPlotter()
{
  Py_Initialize();
}

void PythonPlotter::plot(std::string& command)
{
  PyRun_SimpleString(command.c_str());
}

std::string PythonPlotter::buildScalarCommand(std::string data, std::string title, int id)
{
  std::stringstream ss;
  ss << "from pylab import *\n"
     << "ion()\n"
     << "D = ["
     <<  data
     << "]\n"
     << "fig = figure()\n"
     << "plot([k for k in D])\n"
     << "title('"
     << title
     << "')\n"
     << "draw()\n"
     << "show()\n";
  return ss.str();
}
std::string PythonPlotter::buildComplexCommand(std::string data, std::string title, int id)
{
  std::stringstream ss;
  ss << "from pylab import *\n"
     << "ion()\n"
     << "D = ["
     <<  data
     << "]\n"
     << "fig = figure()\n"
     << "subplot(4,1,1)\n"
     << "plot([real(k) for k in D])\n"
     << "title('"
     << title
     << " - In-Phase')\n"
     << "subplot(4,1,2)\n"
     << "plot([imag(k) for k in D])\n"
     << "title('"
     << title
     << " - Quadrature')\n"
     << "subplot(4,1,3)\n"
     << "plot([abs(k) for k in D])\n"
     << "title('"
     << title
     << " - Magnitude')\n"
     << "subplot(4,1,4)\n"
     << "plot([angle(k) for k in D])\n"
     << "title('"
     << title
     << " - Phase')\n"
     << "fig.subplots_adjust(hspace=0.5)\n"
     << "draw()\n"
     << "show()\n";
    return ss.str();
}

