// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_STORAGE_HPP
#define _INCLUDED_SMOC_DETAIL_STORAGE_HPP

#include <cassert>
#include <new>

#include <CoSupport/SystemC/ChannelModificationSender.hpp> 

#include <type_traits>

#include <boost/utility/enable_if.hpp>

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

template<class T, bool DC = std::is_default_constructible<T>::value>
class Storage;

template<class T>
class Storage<T, true>
  : public CoSupport::SystemC::ChannelModificationSender<T>
{
private:
  char mem[sizeof(T)];
  bool valid;
private:
  T       *ptr()
    { return reinterpret_cast<T       *>(mem); }

  T const *ptr() const
    { return reinterpret_cast<T const *>(mem); }
public:
  Storage() : valid(false) {}

  Storage(const T& t) : valid(true) { new(mem) T(t); }

  T &get() {
    if (!valid) {
      // Default construct element on demand.
      new(mem) T();
      valid = true;
    }
    T &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified(t);
    return t;
  }

  operator T &()
    { return get(); }

  T const &get() const {
    assert(valid);
    T const &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified(t);
    return t;
  }

  operator T const &() const
    { return get(); }

  void put(T const &t) {
    this->fireModified(t);
    if (valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }

  const bool isValid() const
    { return valid; }

  // invalidate data (e.g., to avoid reread of commited data)
  void invalidate() {
    if (valid) {
      ptr()->~T();
      valid = false;
    }
  }

  ~Storage()
    { invalidate(); }
};

template<class T>
class Storage<T, false>
  : public CoSupport::SystemC::ChannelModificationSender<T>
{
private:
  char mem[sizeof(T)];
  bool valid;
private:
  T       *ptr()
    { return reinterpret_cast<T       *>(mem); }

  T const *ptr() const
    { return reinterpret_cast<T const *>(mem); }
public:
  Storage() : valid(false) {}

  Storage(const T& t) : valid(true) { new(mem) T(t); }

  T &get() {
    assert(valid);
    T &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified(t);
    return t;
  }

  operator T &()
    { return get(); }

  T const &get() const {
    assert(valid);
    T const &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified(t);
    return t;
  }

  operator T const &() const
    { return get(); }

  void put(T const &t) {
    this->fireModified(t);
    if (valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }

  const bool isValid() const
    { return valid; }

  // invalidate data (e.g., to avoid reread of commited data)
  void invalidate() {
    if (valid) {
      ptr()->~T();
      valid = false;
    }
  }

  ~Storage()
    { invalidate(); }
};

template<class T>
struct StorageTraitsIn {
  typedef Storage<T> const storage_type;
  /// smoc storage with read only memory interface
  typedef const T          &return_type;
};

template<class T>
struct StorageTraitsOut {
  typedef Storage<T>       storage_type;
  /// smoc storage with write only memory interface
  class return_type {
    typedef return_type this_type;
  private:
    storage_type &s;
  public:
    return_type(storage_type &s): s(s) {}

    this_type &operator=(const T &t)
      { s.put(t); return *this; }
  };
};

template<class T>
struct StorageTraitsInOut {
  typedef Storage<T>       storage_type;
  /// smoc storage with read write memory interface
  class return_type {
    typedef return_type this_type;
  private:
    storage_type &s;
  public:
    return_type(storage_type &s): s(s) {}

    operator T &() const
      { return s.get(); }

    this_type &operator=(T const &t)
      { s.put(t); return *this; }
    this_type &operator=(this_type const &rhs)
      { s.put(rhs); return *this; }
  };
};

template<>
struct StorageTraitsIn<void>
{
  typedef void storage_type;
  typedef void return_type;
};

template<>
struct StorageTraitsOut<void>
{
  typedef void storage_type;
  typedef void return_type;
};

template<>
struct StorageTraitsInOut<void>
{
  typedef void storage_type;
  typedef void return_type;
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_STORAGE_HPP */
