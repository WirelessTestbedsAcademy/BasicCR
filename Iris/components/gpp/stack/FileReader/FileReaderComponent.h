/**
 * \file components/gpp/stack/FileReader/FileReaderComponent.h
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
 * A source stack component which reads data from file.
 */

#ifndef STACK_FILEREADERCOMPONENT_H_
#define STACK_FILEREADERCOMPONENT_H_

#include <fstream>
#include "irisapi/StackComponent.h"


namespace iris
{
namespace stack
{

/** A StackComponent to read data from a named file.
 *
 * File name, data block size etc. can be specified using parameters.
 */
class FileReaderComponent
  : public StackComponent
{
public:
  FileReaderComponent(std::string name);
  virtual void initialize();
  virtual void start();
  virtual void stop();
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
  virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

private:
  /** Read a block of data from the file
   *
   * @param readDataBuffer  Buffer to hold the read data.
   */
  void readBlock(boost::shared_ptr<StackDataSet> readDataBuffer);
  /// Entry point for file reading thread.
  void fileReadingLoop();

  //Exposed parameters
  uint32_t blockSize_x;     ///< Size of data blocks to read.
  std::string fileName_x;   ///< Name of file to read.
  bool sendBelow_x;         ///< Send data down?
  bool enabled_x;           ///< Is this reader enabled?
  uint32_t delay_x;         ///< Time to wait between blocks (ms).
  int packets_x;            ///< Number of packets to generate.

  int count_;
  bool running_;

  boost::scoped_ptr< boost::thread > thread_; ///< Handle for file reading thread.
  std::ifstream hInFile_; ///< The file stream.
};

} // namespace stack
} // namespace iris

#endif // STACK_FILEREADERCOMPONENT_H_
