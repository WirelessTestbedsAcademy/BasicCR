/**
 * \file components/gpp/stack/MultiportExample/MultiportExampleComponent.h
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
 * An example StackComponent which uses multiple ports
 */

#ifndef STACK_MULTIPORTEXAMPLECOMPONENT_H_
#define STACK_MULTIPORTEXAMPLECOMPONENT_H_

#include "irisapi/StackComponent.h"

namespace iris
{
namespace stack
{

/// An example of a StackComponent with multiple ports.
class MultiportExampleComponent
  : public StackComponent
{
public:
  MultiportExampleComponent(std::string name);
  virtual void registerPorts(); //We're overriding this to specify multiple ports
  virtual void initialize();
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
	virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

private:
  uint32_t example_x; ///< An example parameter.
};

} // namespace stack
} // namespace iris

#endif // STACK_MULTIPORTEXAMPLECOMPONENT_H_
