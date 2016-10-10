// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_DETAIL_STORAGE_HPP
#define _INCLUDED_SMOC_DETAIL_STORAGE_HPP

#include <cassert>
#include <new>

#include <CoSupport/SystemC/ChannelModificationSender.hpp> 

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

template<class T>
class Storage
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

  //FIXME(MS): allow "const" access for non-strict
  //void operator=(const T& t) const {
  void operator = (T const &t)
    { put(t); }  

  const bool isValid() const
    { return valid; }

  // invalidate data (e.g., to avoid reread of commited data)
  void invalidate() {
    if (valid) {
      ptr()->~T();
      valid = false;
    }
  }

  // reset storage
  // used in SR (signals are reseted if fixed point is reached)
  //
  // FIXME: valid=false but mem may containt constructed data type!
  void reset()
    { valid = false; }

  ~Storage()
    { invalidate(); }
};

/// smoc storage with read only memory interface
// typedef const T & smoc_storage_rom<T>

/// smoc storage with write only memory interface
template<class T>
class StorageAccessWOM
{
private:
  typedef StorageAccessWOM<T> this_type;
private:
  Storage<T> &s;
public:
  StorageAccessWOM(Storage<T> &_s)
    : s(_s) {}

  void operator=(const T& t)
    { s.put(t); }  
};

/// smoc storage with read write memory interface
template<class T>
class StorageAccessRW
{
private:
  typedef StorageAccessRW<T> this_type;
private:
  Storage<T> &s;
public:
  StorageAccessRW(Storage<T> &_s)
    : s(_s) {}
 
  operator const T&() const
    { return s.get(); }

  void operator=(const T& t)
    { s.put(t); }  
};

template<class T>
struct StorageTraitsIn
{
  typedef const Storage<T>  storage_type;
  typedef const T               &return_type;
};

template<class T>
struct StorageTraitsOut
{
  typedef Storage<T>        storage_type;
  typedef StorageAccessWOM<T>    return_type;
};

template<class T>
struct StorageTraitsInOut
{
  typedef Storage<T>        storage_type;
  typedef StorageAccessRW<T>     return_type;
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

namespace CoSupport {
  // provide isType for StorageAccessRW
  template <typename T, typename X>
  static inline
  bool isType(const smoc::Detail::StorageAccessRW<X> &of )
   { return isType<T>(static_cast<const X &>(of)); }
};

#endif // _INCLUDED_SMOC_DETAIL_STORAGE_HPP