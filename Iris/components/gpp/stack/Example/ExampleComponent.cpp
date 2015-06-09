/**
 * \file components/gpp/stack/Example/ExampleComponent.cpp
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
 * Implementation of an example StackComponent to be used as a template
 * for new StackComponents.
 */

#include "ExampleComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{
namespace stack
{

//! Export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, ExampleComponent);

ExampleComponent::ExampleComponent(std::string name)
  : StackComponent(name,                          // Component name
                   "example",                     // Component type
                   "An example stack component",  // Description
                   "Paul Sutton",                 // Author
                   "0.1")                         // Version
{
  //Register all parameters
  /*
   * Format:
   * registerParameter(name,
   *                   description,
   *                   default value,
   *                   dynamic?,
   *                   parameter,
   *                   allowed values)
   */
  registerParameter("exampleparameter",
                    "An example parameter",
                    "0",
                    true,
                    example_x,
                    Interval<uint32_t>(0,5));

  /*
   * Format:
   * registerEvent(name,
   *               description,
   *               data type);
   */
  registerEvent("exampleevent",
                "An example event",
                TypeInfo< uint32_t >::identifier);
}

void ExampleComponent::initialize()
{}

void ExampleComponent::start()
{}

void ExampleComponent::stop()
{}

void ExampleComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> set)
{
  sendDownwards(set); // Simply send the message on
}

void ExampleComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> set)
{
  sendUpwards(set);  // Simply send the message on
}

} // namespace stack
} // namespace iris
