/**
 * \file EndianConversion.h
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
 * Conversion functions for big/little endian.
 */

#ifndef _ENDIAN_CONVERSION_H_
#define _ENDIAN_CONVERSION_H_

#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>
#include <complex>

// define macros to switch byte order (only use with unsigned numbers!)
#define _swapbytes16(x) (((x)>>8) | ((x)<<8))
#define _swapbytes32(x) (((x)>>24) | (((x)<<8) & 0x00FF0000) | (((x)>>8) & 0x0000FF00) |  ((x)<<24))
#define _swapbytes64(x) ((_swapbytes32(x & 0x00000000FFFFFFFF) << 32) | (_swapbytes32(x >> 32) & 0x00000000FFFFFFFF))


// class for generic endianess conversion using union hack
template <class T>
struct __swap_bytes
{
  explicit __swap_bytes(const T& x )
  {
    union {T x; unsigned char byte[sizeof(T)]; } in, out;
    in.x = x;
    for (size_t i = 0; i < sizeof(T); ++i)
      out.byte[sizeof(T)-1-i] = in.byte[i];
    output = out.x;
  }

  operator T() { return output; }
  //operator const T() { return output; }
private:
  T output;
};


// ------------ all the template specialisations now

// for one-byte types, there is nothing to do
template<>
struct __swap_bytes<boost::uint8_t>
{
  explicit __swap_bytes(const boost::uint8_t& x) : output(x) {}
  operator boost::uint8_t() { return output;}
private:
  const boost::uint8_t& output;
};

template<>
struct __swap_bytes<boost::int8_t>
{
  explicit __swap_bytes(const boost::int8_t& x) : output(x) {}
  operator boost::int8_t() { return output;}
private:
  const boost::int8_t& output;
};

// for all signed/unsigned integer type, specialise and use the macro (more efficient)
template <>
struct __swap_bytes<boost::uint16_t>
{
  explicit __swap_bytes(const boost::uint16_t& x) : output(_swapbytes16(x)) {}
  operator boost::uint16_t() {return output;}
private:
  boost::uint16_t output;
};

template <>
struct __swap_bytes<boost::int16_t>
{
  explicit __swap_bytes(const boost::int16_t& x) : output(_swapbytes16((boost::uint16_t)x)) {}
  operator boost::int16_t() {return output;}
private:
  boost::int16_t output;
};

template <>
struct __swap_bytes<boost::uint32_t>
{
  explicit __swap_bytes(const boost::uint32_t& x) : output(_swapbytes32(x)) {}
  operator boost::uint32_t() {return output;}
private:
  boost::uint32_t output;
};

template <>
struct __swap_bytes<boost::int32_t>
{
  explicit __swap_bytes(const boost::int32_t& x) : output(_swapbytes32((boost::uint32_t)x)) {}
  operator boost::int32_t() {return output;}
private:
  boost::int32_t output;
};

template <>
struct __swap_bytes<boost::uint64_t>
{
  explicit __swap_bytes(const boost::uint64_t& x) : output(_swapbytes64(x)) {}
  operator boost::uint64_t() {return output;}
private:
  boost::uint64_t output;
};

template <>
struct __swap_bytes<boost::int64_t>
{
  explicit __swap_bytes(const boost::int64_t& x) : output(_swapbytes64((boost::uint64_t)x)) {}
  operator boost::int64_t() {return output;}
private:
  boost::int64_t output;
};


// for all floating point types, use the union trick and macro

template <>
struct __swap_bytes<float>
{
  explicit __swap_bytes(const float& x)
  {
    union {
      float f;
      boost::uint32_t i;
    } tmp;
    tmp.f = x;
    tmp.i = _swapbytes32(tmp.i);
    output = tmp.f;
  }
  operator float() {return output;}
private:
  float output;
};

template <>
struct __swap_bytes<double>
{
  explicit __swap_bytes(const double& x)
  {
    union {
      double f;
      boost::uint64_t i;
    } tmp;
    tmp.f = x;
    tmp.i = _swapbytes64(tmp.i);
    output = tmp.f;
  }
  operator double() {return output;}
private:
  double output;
};

template <>
struct __swap_bytes<long double>
{
  explicit __swap_bytes(const long double& x)
  {
    union {
      long double f;
      boost::uint64_t i[2];
    } tmp;
    tmp.f = x;
    tmp.i[0] = _swapbytes64(tmp.i[0]);
    tmp.i[1] = _swapbytes64(tmp.i[1]);
    std::swap(tmp.i[0], tmp.i[1]);
    output = tmp.f;
  }
  operator long double() {return output;}
private:
  long double output;
};


// for complex numbers, do conversion for real and imaginary parts separately
template <class T>
struct __swap_bytes<std::complex<T> >
{
  explicit __swap_bytes(const std::complex<T>& x) :
    output(std::complex<T>(__swap_bytes<T>(real(x)), __swap_bytes<T>(imag(x)))) {}
  operator std::complex<T>() {return output;}
private:
  std::complex<T> output;
};

// now all the user interface functions


//! swaps the byte order of the given number (bit to little or little to big)
template <typename T>
inline T swap_bytes(T x)
{
  return __swap_bytes<T>(x);
}


//! converts from big endian format to the systems's native format
template <typename T>
inline T big2sys(T x)
{
#ifdef BOOST_BIG_ENDIAN
  return x;
#else
  return swap_bytes(x);
#endif
}

//! converts from the system's native format to big endian
template <typename T>
inline T sys2big(T x)
{
#ifdef BOOST_BIG_ENDIAN
  return x;
#else
  return swap_bytes(x);
#endif
}

//! converts from little endian format to the systems's native format
template <typename T>
inline T lit2sys(T x)
{
#ifdef BOOST_BIG_ENDIAN
  return swap_bytes(x);
#else
  return x;
#endif
}

//! converts from the system's native format to little endian
template <typename T>
inline T sys2lit(T x)
{
#ifdef BOOST_BIG_ENDIAN
  return swap_bytes(x);;
#else
  return x;
#endif
}



#endif
