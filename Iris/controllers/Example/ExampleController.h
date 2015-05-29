/**
 * \file controllers/Example/ExampleController.h
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
 * An example controller to be used when creating new controllers.
 */

#ifndef CONTROLLERS_EXAMPLECONTROLLER_H_
#define CONTROLLERS_EXAMPLECONTROLLER_H_

#include "irisapi/Controller.h"

namespace iris
{

/** An example Controller to be used as a template for new Controllers
 *
 * Copy this controller, rename folder, .h and .cpp files.
 * Edit all files for your new controller and build.
 */
class ExampleController
  : public Controller
{
public:
  /** Construct the controller.
   *
   * Call the constructor on the parent Controller class, pass in all
   * details about this controller.
   */
  ExampleController();

  /** Subscribe to events.
  *
  * Subscribe to events on specific components that this controller is interested in.
  */
	virtual void subscribeToEvents();

	/** Initialize this controller.
	*
	* Do any initialization required by this controller.
	* This function is guaranteed to be called by this controller's thread.
	*/
	virtual void initialize();

	/** Process an event.
	*
	* If an event which this controller has subscribed to is activated, this function is called by
	* the controller thread.
	*
	* \param  e The event which was activated by the component
	*/
  virtual void processEvent(Event &e);

  /** Destroy this controller
  *
  * Do any destruction required by this controller.
  * This function is guaranteed to be called by this controller's thread.
  * IMPORTANT: This function should never throw an exception.
  */
	virtual void destroy();

private:
  uint32_t counter_;
};

} // namespace iris

#endif // CONTROLLERS_EXAMPLECONTROLLER_H_
