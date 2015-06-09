/**
 * \file components/gpp/stack/MultiportExample/MultiportExampleComponent.cpp
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
 * Implementation of an example StackComponent which uses multiple ports
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "MultiportExampleComponent.h"

using namespace std;
namespace b = boost;

namespace iris
{
namespace stack
{

/// Export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, MultiportExampleComponent);

MultiportExampleComponent::MultiportExampleComponent(std::string name)
  : StackComponent(name,
                   "multiportexample",
                   "An example stack component with multiple ports",
                   "Paul Sutton",
                   "0.1")
{
  registerParameter("exampleparameter",       // name
                    "An example parameter",   // description
                    "0",                      // default value
                    true,                     // dynamic?
                    example_x,                // parameter
                    Interval<uint32_t>(0,5)); // allowed values
}

//We're overriding registerPorts as we want to use multiple ports
void MultiportExampleComponent::registerPorts()
{
  std::vector<int> types;
  types.push_back( int(TypeInfo< uint8_t >::identifier) );

  //The port on top of the component
  registerInputPort("topport1", types);

  //The first port below the component
  registerInputPort("bottomport1", types);

  //The second port below the component
  registerInputPort("bottomport2", types);
};

void MultiportExampleComponent::initialize()
{}

/** Process a message from above
*
*	This example copies StackDataSets onto all its bottom ports
*/
void MultiportExampleComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> set)
{
  //Create a second StackDataSet and copy the set into it
  b::shared_ptr<StackDataSet> cpy(new StackDataSet);
  *cpy = *set;

  sendDownwards("bottomport1", set);
  sendDownwards("bottomport2", cpy);
}

/** Process a message from below
*
*	This example just passes data through.
*/
void MultiportExampleComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> set)
{
  sendUpwards(set);
}

} // namespace stack
} // namespace iris
