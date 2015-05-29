/**
 * \file controllers/SpectrogramDisplay/SpectrogramDisplayController.cpp
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
 * A spectrogram display using the qt realplot widget.
 */

#include <sstream>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "SpectrogramDisplayController.h"

using namespace std;

namespace iris
{

//! Export library functions
IRIS_CONTROLLER_EXPORTS(SpectrogramDisplayController);

SpectrogramDisplayController::SpectrogramDisplayController()
  : Controller("SpectrogramDisplay", "A spectrogram display", "Paul Sutton", "0.1")
{
  registerParameter("spectrogramcomponent", "Name of spectrogram component",
      "spectrogram1", false, spgrmCompName_x);
}

void SpectrogramDisplayController::subscribeToEvents()
{
  subscribeToEvent("psdevent", spgrmCompName_x);
}

void SpectrogramDisplayController::initialize()
{
  plot_.reset(new Realplot());
  plot_->setYAxisScale(-100, 0);
}

void SpectrogramDisplayController::processEvent(Event &e)
{
  //We've only subscribed to psdevent
  vector<float> data;
  for(int i=0;i<e.data.size();i++)
    data.push_back(boost::any_cast<float>(e.data[i]));

  plot_->setNewData(data.begin(), data.end());
}

void SpectrogramDisplayController::destroy()
{}

} // namespace iris
