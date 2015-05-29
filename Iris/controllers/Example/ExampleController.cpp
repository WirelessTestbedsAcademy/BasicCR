/**
 * \file controllers/Example/ExampleController.cpp
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
 * Implementation of an example controller to be used when creating
 * new controllers.
 */

#include <sstream>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "ExampleController.h"

using namespace std;

namespace iris
{

//! Export library functions
IRIS_CONTROLLER_EXPORTS(ExampleController);

ExampleController::ExampleController()
  : Controller("Example", "An example controller", "Paul Sutton", "0.1")
{
  counter_ = 1;
}

void ExampleController::subscribeToEvents()
{
  // Format: subscribeToEvent(Event name, Component name);
  subscribeToEvent("testevent", "snk1");
}

void ExampleController::initialize()
{}

void ExampleController::processEvent(Event &e)
{
  //Reconfigure the radio by creating a reconfiguration set
  ReconfigSet r;

  ParametricReconfig p;
  p.engineName = "phyengine1";
  p.componentName = "src1";
  p.parameterName = "counter_";
  stringstream str;
  str << counter_++;
  p.parameterValue = str.str();

  r.paramReconfigs.push_back(p);

  reconfigureRadio(r);
}

void ExampleController::destroy()
{}

} // namespace iris
