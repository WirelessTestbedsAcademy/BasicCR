/**
 * \file components/gpp/phy/Example/ExampleComponent.cpp
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
 * Implementation of an example PhyComponent to be used as a template
 * for new PhyComponents.
 */

#include "ExampleComponent.h"

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, ExampleComponent);

ExampleComponent::ExampleComponent(std::string name)
  : PhyComponent(name,                      // component name
                "example",                  // component type
                "An example phy component", // description
                "Paul Sutton",              // author
                "0.1")                      // version
{
  registerParameter(
      "exampleparameter",                   // name
      "An example parameter",               // description
      "0",                                  // default value
      true,                                 // dynamic?
      example_x,                            // parameter
      Interval<uint32_t>(0,5));             // allowed values

  registerEvent(
      "exampleevent",                       // name
      "An example event",                   // description
      TypeInfo< uint32_t >::identifier);    // data type
}

void ExampleComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint32_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint32_t >::identifier);
}

void ExampleComponent::calculateOutputTypes(
    std::map<std::string,int>& inputTypes,
    std::map<std::string,int>& outputTypes)
{
  //One output type - always uint32_t
  outputTypes["output1"] = TypeInfo< uint32_t >::identifier;
}

void ExampleComponent::initialize()
{
}

void ExampleComponent::process()
{
  //Get a DataSet from the input DataBuffer
  DataSet<uint32_t>* readDataSet = NULL;
  getInputDataSet("input1", readDataSet);
  std::size_t size = readDataSet->data.size();

  //Get a DataSet from the output DataBuffer
  DataSet<uint32_t>* writeDataSet = NULL;
  getOutputDataSet("output1", writeDataSet, size);

  //Copy the input DataSet to the output DataSet
  copy(readDataSet->data.begin(), readDataSet->data.end(), writeDataSet->data.begin());

  //Copy the timestamp and sample rate for the DataSets
  writeDataSet->timeStamp = readDataSet->timeStamp;
  writeDataSet->sampleRate = readDataSet->sampleRate;

  //Release the DataSets
  releaseInputDataSet("input1", readDataSet);
  releaseOutputDataSet("output1", writeDataSet);
}

} // namesapce phy
} // namespace iris
