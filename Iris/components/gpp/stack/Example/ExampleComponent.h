/**
 * \file components/gpp/stack/Example/ExampleComponent.h
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
 * An example StackComponent to be used as a template for new StackComponents.
 */

#ifndef STACK_EXAMPLECOMPONENT_H_
#define STACK_EXAMPLECOMPONENT_H_

#include "irisapi/StackComponent.h"

namespace iris
{
namespace stack
{

/** An example StackComponent to be used as a template for new StackComponents.
 *
 * Copy this component, rename folder, .h and .cpp files.
 * Edit all files for your new component and build.
 */
class ExampleComponent
  : public StackComponent
{
public:
	/** Call the constructor on StackComponent and pass in all details
   * about the component. Register all parameters and events.
   *
   * \param name The name given to this component in the radio config
   */
  ExampleComponent(std::string name);

  /// Do any initialization required by this component.
  virtual void initialize();
  /// Start this component.
  virtual void start();
  /// Stop this component
  virtual void stop();

  /**Called whenever this component receives a message from above.
   *
   * \param set Shared pointer to the received message
   */
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);

  /** Called whenever this component receives a message from below.
   *
   * \param set Shared pointer to the received message
   */
	virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

private:
  uint32_t example_x; //!< Example parameter exposed by this component
};

} // namespace stack
} // namespace iris

#endif // STACK_EXAMPLECOMPONENT_H_
