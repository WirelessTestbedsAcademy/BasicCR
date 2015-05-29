/**
 * \file components/gpp/phy/MatlabTemplate/MatlabTemplateComponent.h
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
 * A template component used to read/write data to a matlab script.
 */

#ifndef PHY_MATLABTEMPLATECOMPONENT_H_
#define PHY_MATLABTEMPLATECOMPONENT_H_

#include "irisapi/TemplatePhyComponent.h"
#include "utility/Matlab.h"

namespace iris
{
namespace phy
{

/// Forward declaration of our implementation class.
template <class Tin, class Tout>
class MatlabTemplateComponentImpl;

/** A template PhyComponent used to interface with Matlab.
 *
 * The MatlabTemplateComponent is derived from TemplatePhyComponent,
 * so we need to give the class name itself as a template parameter.
 * This component passes data to a named Matlab script and optionally
 * accepts returned data to be passed on. The script name is provided
 * using a parameter. Additional parameters can be set to provide
 * output data and/or to pass data through from the input to the output.
 */
class MatlabTemplateComponent
  : public TemplatePhyComponent<MatlabTemplateComponent>
{
 public:
  /** Supported types at the input port.
   *
   * Can be a boost::mpl::vector of types, or of other
   * mpl vectors if multiple inputs are present.
   */
  typedef IrisDataTypes supportedInputTypes;

  /// supported types at the output port
  typedef IrisDataTypes supportedOutputTypes;

  MatlabTemplateComponent(std::string name);
  virtual ~MatlabTemplateComponent();
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();

  /** Create a new instance of the implementation class,
   * with the correct template parameters.
   *
   * Also copies all values of the parameters to the newly
   * created instance.
   * \param comp The component used to create the instance.
   * \return The created component instance.
   */
  template <typename Tin, typename Tout>
  static PhyComponent* createInstance(const PhyComponent* comp)
  {
    return new MatlabTemplateComponentImpl<Tin,Tout>(*comp);
  }

 protected:
  std::string scriptName_x; ///< Matlab script to be called.
  bool hasOutput_x;         ///< Does this component output data?
  bool passThrough_x;       ///< Does this component pass data through?


  Matlab matlab_;           ///< The Matlab engine
  mxArray *matlabInput_;    ///< Array for Matlab input
  mxArray *matlabOutput_;   ///< Array for Matlab output

  std::string command_;     ///< Command used to execute Matlab script
};

/** The actual implementation of the MatlabTemplateComponent.
 *
 * The actual implementation of the MatlabComponent - implemented as template.
 */
template <class Tin, class Tout>
class MatlabTemplateComponentImpl
  : public MatlabTemplateComponent
{
 public:
  /** Constructor - call the constructor on parent
   * and assign all values from other.
   *
   * \param other the PhyComponent with correct i/o datatypes
   */
  MatlabTemplateComponentImpl(const PhyComponent& other)
    : MatlabTemplateComponent(other.getName())
  {
    // assign all values from other to this
    assign(other);
  }

  ~MatlabTemplateComponentImpl();
  virtual void initialize();
  virtual void process();

 private:
  typedef typename boost::mpl::front<Tin>::type T;  ///< Input port type

  /** Copy data from our vector into the mxArray container used by Matlab.
   *
   * \param data    The data to be copied.
   * \param matlabInput The target for the data to be copied.
   */
  template<class U>
  void copyInData(std::vector<U> &data, mxArray* matlabInput)
  {
    double* ptr = mxGetPr(matlabInput);
    for(size_t i=0;i<data.size();i++)
    {
     ptr[i] = (double)data[i];
    }
  }

  /** Copy data from our vector into the mxArray container used by Matlab.
   *
   * This specialization captures boost::mpl::void_ template instantiations.
   * \param data    The data to be copied.
   * \param matlabInput The target for the data to be copied.
   */
  void copyInData(std::vector<boost::mpl::void_> &data, mxArray* matlabInput)
  {}

  /** Copy data from our vector into the mxArray container used by Matlab.
   *
   * This specialization captures std::complex data types.
   * \param data    The data to be copied.
   * \param matlabInput The target for the data to be copied.
   */
  template<class U>
  void copyInData(std::vector< std::complex<U> > &data, mxArray* matlabInput)
  {
    double* pR = mxGetPr(matlabInput);
    double* pI = mxGetPi(matlabInput);
    for(size_t i=0;i<data.size();i++)
    {
     pR[i] = (double)data[i].real();
     pI[i] = (double)data[i].imag();
    }
  }

  /** Copy data from the mxArray container used by Matlab into our vector.
   *
   * \param matlabOutput The array container containing the data.
   * \param data    Our vector - the target for the data.
   */
  template<class U>
  void copyOutData(mxArray* matlabOutput, std::vector<U> &data)
  {
    double* ptr = mxGetPr(matlabOutput);
    for(size_t i=0;i<data.size();i++)
    {
     data[i] = (U)ptr[i];
    }
  }

  /** Copy data from the mxArray container used by Matlab into our vector.
   *
   * This captures boost::mpl::void_ template instantiations.
   * \param matlabOutput The array container containing the data.
   * \param data    Our vector - the target for the data.
   */
  void copyOutData(mxArray* matlabOutput, std::vector<boost::mpl::void_> &data)
  {}

  /** Copy data from the mxArray container used by Matlab into our vector.
   *
   * Template specialization to handle complex data.
   * \param matlabOutput The array container containing the data.
   * \param data    Our vector - the target for the data.
   */
  template<class U>
  void copyOutData(mxArray* matlabOutput, std::vector< std::complex<U> > &data)
  {
    double* pR = mxGetPr(matlabOutput);
    double* pI = mxGetPi(matlabOutput);
    for(size_t i=0;i<data.size();i++)
    {
     data[i] = std::complex<U>((U)pR[i], (U)pI[i]);
    }
  }

  ReadBuffer<T>* inBuf_;      ///< Convenience pointer to input buffer.
  DataSet<T>* readDataSet_;   ///< Convenience pointer to input data set.
  WriteBuffer<T>* outBuf_;    ///< Convenience pointer to output buffer.
  DataSet<T>* writeDataSet_;  ///< Convenience pointer to output data set.

  char buffer_[256];  ///< Buffer to capture messages from Matlab
};

} // namespace phy
} // namepace iris

#endif  // PHY_MATLABTEMPLATECOMPONENT_H_
