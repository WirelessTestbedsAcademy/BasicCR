/**
 * \file components/gpp/stack/TunTap/TunTapComponent.h
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
 * This component implements a software connector between Iris and
 * virtual network device drivers TUN/TAP on Linux/Unix systems.
 *
 */

#ifndef STACK_TUNTAPCOMPONENT_H_
#define STACK_TUNTAPCOMPONENT_H_

#include "irisapi/StackComponent.h"
#include "TunTapComponent.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <boost/format.hpp>

#define MAX_BUF_SIZE (10 * 1024) // should be enough for now

namespace iris
{
namespace stack
{

class TunTapComponent
  : public StackComponent
{
public:
  TunTapComponent(std::string name);

  virtual void initialize();
  virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
  virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

  virtual void start();
  virtual void stop();

private:
  //Exposed parameters
  bool readFromBelow_x;       ///< Accept blocks from below (istead of above)
  std::string tunTapDevice_x; ///< Name of the Tun/Tap device to attach to

  char tunName_[IFNAMSIZ];
  int tunFd_;
  char buffer_[MAX_BUF_SIZE];
  boost::scoped_ptr< boost::thread > rxThread_;

  // private functions
  void processMessage(boost::shared_ptr<StackDataSet> incomingFrame);
  void rxThreadFunction();
  int allocateTunDevice(char *dev, int flags);

};

} // namespace stack
} // namespace iris

#endif // STACK_TUNTAPCOMPONENT_H_
