/**
 * \file DataBufferTrivial.h
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
 * A stripped-down version of the DataBuffer for component tests.
 */

#ifndef DATABUFFERTRIVIAL_H_
#define DATABUFFERTRIVIAL_H_

#include "irisapi/DataBufferInterfaces.h"
#include "irisapi/Exceptions.h"
#include "irisapi/TypeInfo.h"

namespace iris
{

/** The DataBufferTrivial class implements a buffer between Iris components
 * for testing.
*
*	The buffer consists of a number of DataSet objects which can be written and
*	read by the components.	Components can get a DataSet to write to by calling
*	GetWriteSet(). When finished writing, the component	releases the DataSet by
*	calling ReleaseWriteSet(). Components can get a DataSet to read from by
*	calling GetReadSet(). When finished reading, the component releases the
*	DataSet by calling ReleaseReadSet().	The DataBufferTrivial is not
*	thread-safe. It does not block, and keeps growing if new DataSets are
*	requested.
*/
template <typename T>
class DataBufferTrivial
  : public ReadBuffer<T>, public WriteBuffer<T>
{
public:

  explicit DataBufferTrivial(std::size_t buffer_size = 3)
    :buffer_(buffer_size) ,
    isReadLocked_(false),
    isWriteLocked_(false),
    readIndex_(0),
    writeIndex_(0),
    notEmpty_(false),
    notFull_(true)
  {
    //Set the type identifier for this buffer
    typeIdentifier = TypeInfo<T>::identifier;
    if( typeIdentifier == -1)
      throw InvalidDataTypeException("Data type not supported");
  };

  virtual ~DataBufferTrivial(){};

  /// Get the identifier for the data type of this buffer
  virtual int getTypeIdentifier() const   {  return typeIdentifier; }

  /// Is there any data in this buffer?
  virtual bool hasData() const { return is_not_empty(); }

  // empty implementation - not needed for tests
  virtual void setLinkDescription(LinkDescription) {};
  virtual LinkDescription getLinkDescription() const { return LinkDescription(); }

  /** Get the next DataSet to read
   *
   * @param setPtr   A DataSet pointer which will be set by the buffer
   */
  virtual void getReadData(DataSet<T>*& setPtr)
  {
    if(isReadLocked_)
      throw DataBufferReleaseException("getReadData() called before previous DataSet was released");
    isReadLocked_ = true;
    setPtr = &buffer_[readIndex_];
  };

  /** Get the next DataSet to be written
   *
   * @param setPtr   A DataSet pointer which will be set by the buffer
   * @param size   The number of elements required in the DataSet
   */
  virtual void getWriteData(DataSet<T>*& setPtr, std::size_t size)
  {
    if(isWriteLocked_)
      throw DataBufferReleaseException("getWriteData() called before previous DataSet was released");
    isWriteLocked_ = true;
    if(buffer_[writeIndex_].data.size() != size)
      buffer_[writeIndex_].data.resize(size);
    setPtr = &buffer_[writeIndex_];
  };

  /** Release a read DataSet
   *
   * @param setPtr   A pointer to the DataSet to be released
   */
  virtual void releaseReadData(DataSet<T>*& setPtr)
  {
    if(++readIndex_ == buffer_.size())
    {
      buffer_.resize(readIndex_+1);
    }
    if(readIndex_ == writeIndex_)
      notEmpty_ = false;
    notFull_ = true;
    isReadLocked_ = false;
    setPtr = NULL;
  };

  /** Release a write DataSet
   *
   * @param setPtr   A pointer to the DataSet to be released
   */
  virtual void releaseWriteData(DataSet<T>*& setPtr)
  {
    if(++writeIndex_ == buffer_.size())
    {
      buffer_.resize(writeIndex_+1);
    }
    if(readIndex_ == writeIndex_)
      notFull_ = false;
    notEmpty_ = true;
    isWriteLocked_ = false;
    setPtr = NULL;
  };

  std::vector< DataSet<T> > getBuffer() { return buffer_; }

private:
  /// The data type of this buffer
  int typeIdentifier;

  /// The vector of DataSets
  std::vector< DataSet<T> > buffer_;

  bool isReadLocked_;
  bool isWriteLocked_;

  std::size_t readIndex_;
  std::size_t writeIndex_;

  bool notEmpty_;
  bool notFull_;

  bool is_not_empty() const { return notEmpty_; }
  bool is_not_full() const { return notFull_; }

};

} // namespace iris

#endif // DATABUFFERTRIVIAL_H_
