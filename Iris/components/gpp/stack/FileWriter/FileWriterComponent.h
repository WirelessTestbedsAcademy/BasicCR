/**
 * \file components/gpp/stack/FileWriter/FileWriterComponent.h
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
 * A sink stack component which writes data to file.
 */

#ifndef STACK_FILEWRITERCOMPONENT_H_
#define STACK_FILEWRITERCOMPONENT_H_

#include <fstream>

#include "irisapi/StackComponent.h"

namespace iris
{
namespace stack
{

/** A StackComponent to write data to a named file.
 *
 * File name can be specified using parameters.
 */
class FileWriterComponent
  : public StackComponent
{
public:
  FileWriterComponent(std::string name);
  virtual void initialize();
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
	virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

private:
	/** Write a block of data to file.
	 *
	 * @param toWrite   Pointer to the data set containing the data.
	 */
	void writeBlock(boost::shared_ptr<StackDataSet> toWrite);

  std::string fileName_x; ///< The name of the file to write to.
  bool fromBelow_x;       ///< Do we write data coming from below?

  std::ofstream hOutFile_;  ///< Handle to our file stream.

};

} // namespace stack
} // namespace iris

#endif // STACK_FILEWRITERCOMPONENT_H_
