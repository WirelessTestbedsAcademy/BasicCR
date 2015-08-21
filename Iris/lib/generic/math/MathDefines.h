/**
 * \file MathDefines.h
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
 * Math definitions for use in Iris.
 */

#ifndef MATHDEFINES_H
#define MATHDEFINES_H

#define IRIS_E    2.71828182845904523536
#define IRIS_LOG2E  1.44269504088896340736
#define IRIS_LOG10E   0.434294481903251827651
#define IRIS_LN2    0.693147180559945309417
#define IRIS_LN10   2.30258509299404568402
#define IRIS_PI     3.14159265358979323846
#define IRIS_PI_2   1.57079632679489661923
#define IRIS_PI_4   0.785398163397448309616
#define IRIS_1_PI   0.318309886183790671538
#define IRIS_2_PI   0.636619772367581343076
#define IRIS_2_SQRTPI 1.12837916709551257390
#define IRIS_SQRT2  1.41421356237309504880
#define IRIS_SQRT1_2  0.707106781186547524401

namespace iris
{
enum QAM
{
  BPSK=1,
  QPSK=2,
  QAM16=4
};
} // namespace iris

#endif

