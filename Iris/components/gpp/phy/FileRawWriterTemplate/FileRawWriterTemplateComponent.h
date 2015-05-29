/**
 * \file components/gpp/phy/FileRawWriterTemplate/FileRawWriterTemplateComponent.h
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
 * A template sink component used to write data to file.
 */

#ifndef PHY_FILERAWWRITERTEMPLATECOMPONENT_H_
#define PHY_FILERAWWRITERTEMPLATECOMPONENT_H_

#include <fstream>

#include "irisapi/TemplatePhyComponent.h"

namespace iris
{
namespace phy
{

// forward declaration
template <class Tin, class Tout>
class FileRawWriterTemplateComponentImpl;


/** A template PhyComponent used to write data to a named file.
 *
 * The FileRawWriterTemplateComponent is derived from TemplatePhyComponent,
 * so we need to give the class name itself as a template parameter. This
 * component is an example of a template component for Iris and will write
 * any data type to a named file.
 */
class FileRawWriterTemplateComponent
  : public TemplatePhyComponent<FileRawWriterTemplateComponent>
{
 public:
  /**
   * Supported types at the input port
   * (can be a boost::mpl::vector of types, or of other
   * mpl vectors if multiple inputs are present)
   */
  typedef IrisDataTypes supportedInputTypes;

  /**
   * We have no output, so empty vector here.
   * \todo we should typdef the empty mpl::vector<> to some name for convenience.
   */
  typedef boost::mpl::vector<> supportedOutputTypes;


  FileRawWriterTemplateComponent(std::string name);
  virtual ~FileRawWriterTemplateComponent(){};
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();

  /** Creates a new instance of the implementation class,
   * with the correct template parameters.
   *
   * Also copies all values of the parameters to the newly
   * created instance.
   * \param comp The component used to create the instance
   * \return The created component instance
   */
  template <typename Tin, typename Tout>
  static PhyComponent* createInstance(const PhyComponent* comp)
  {
    return new FileRawWriterTemplateComponentImpl<Tin,Tout>(*comp);
  }

 protected:
  std::string fileName_x; ///< The name of the file to write
};

/** The actual FileRawWriterTemplateComponent implementation.
 *
 * The actual implementation of the FileRawWriter - implemented as template.
 */
template <class Tin, class Tout>
class FileRawWriterTemplateComponentImpl
  : public FileRawWriterTemplateComponent
{
 public:
  /** Constructor - call the constructor on parent
   * and assign all values from other.
   *
   * \param other the PhyComponent with correct i/o datatypes
   */
  FileRawWriterTemplateComponentImpl(const PhyComponent& other)
   : FileRawWriterTemplateComponent(other.getName())
  {
    // assign all values from other to this
    assign(other);
  }

  ~FileRawWriterTemplateComponentImpl();
  virtual void initialize();
  virtual void process();

 private:
  std::ofstream hOutFile_;  ///< The file stream

  typedef typename boost::mpl::front<Tin>::type T; ///< The input port type

  ReadBuffer<T>* inBuf_;    ///< Convenience pointer for incoming DataSet buffer
  DataSet<T>* readDataSet_; ///< Convenience pointer for incoming DataSet
};

} // namespace phy
} // namespace iris

#endif // PHY_FILERAWWRITERTEMPLATECOMPONENT_H_
