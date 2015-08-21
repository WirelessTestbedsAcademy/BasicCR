/**
 * \file lib/generic/utility/PythonPlotter.h
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
 * This utility class uses the python engine to plot sets of data.
 */

#ifndef PYTHONPLOTTER_H_
#define PYTHONPLOTTER_H_

#include <iterator>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <string.h>
#include <iostream>
#include <sstream>

/** A utility class to plot data sets using python.
 *
 * PythonPlotter provides a simple interface to plot data sets using
 * begin and end iterators. All data types are supported. Complex data
 * is plotted in a compound figure including real, imaginary, magnitude
 * and phase.
 */
class PythonPlotter
{
public:
  PythonPlotter();

  /** Plot a set of complex data.
   *
   * Boost enable_if is used to activate this template specialization
   * if the underlying input type is complex.
   *
   * @param begin Iterator to first data element.
   * @param end   Iterator to one past last data element.
   * @param title Optional title string for figure.
   * @param id    Optional figure id number.
   */
  template <class It>
  void plot(It begin, It end, std::string title = "Data", int id=0,
            typename boost::enable_if_c< boost::is_complex<
              typename std::iterator_traits<It>::value_type
            >::value >::type* dummy=0)
  {
    int size = end-begin;
    std::stringstream ss;
    for(int i=0;i<size;i++,begin++)
      ss << begin->real() << "+" << begin->imag() << "j,";
    std::string command = buildComplexCommand(ss.str(), title, id);
    plot(command);
  }

  /** Plot a set of scalar data.
   *
   * Boost disable_if is used to deactivate this template specialization
   * if the underlying input type is complex.
   *
   * @param begin Iterator to first data element.
   * @param end   Iterator to one past last data element.
   * @param title Optional title string for figure.
   * @param id    Optional figure id number.
   */
  template <class It>
  void plot(It begin, It end, std::string title = "Data", int id=0,
            typename boost::disable_if_c< boost::is_complex<
              typename std::iterator_traits<It>::value_type
            >::value >::type* dummy=0)
  {
    int size = end-begin;
    std::stringstream ss;
    for(int i=0;i<size;i++,begin++)
      ss << *begin << ",";
    std::string command = buildScalarCommand(ss.str(), title, id);
    plot(command);
  }

private:

  void plot(std::string& command);
  std::string buildScalarCommand(std::string data, std::string title, int id);
  std::string buildComplexCommand(std::string data, std::string title, int id);

}; // class PythonPlotter

#endif // PYTHONPLOTTER_H_
