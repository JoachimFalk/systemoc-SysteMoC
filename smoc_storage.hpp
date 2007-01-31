// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef SMOC_STORAGE_HPP
#define SMOC_STORAGE_HPP

#include <cassert>
#include <new>

template<class T>
class smoc_storage
{
private:
  char mem[sizeof(T)];
  bool valid;
private:
  T* ptr()
  { return reinterpret_cast<T*>(mem); }

  const T* ptr() const
  { return reinterpret_cast<const T*>(mem); }
public:
  smoc_storage() : valid(false) {
  }

  const T& get() const {
    assert(valid);
    return *ptr();
  }

  operator const T&() const
    { return get(); }

  void put(const T &t) {
    if(valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }

  void operator=(const T& t)
    { put(t); }  

  ~smoc_storage() {
    if(valid) {
      ptr()->~T();
      valid = false;
    }
  }
};

/// smoc storage with read only memory interface
// typedef const T & smoc_storage_rom<T>

/// smoc storage with write only memory interface
template<class T>
class smoc_storage_wom
{
private:
  typedef smoc_storage_wom<T> this_type;
private:
  smoc_storage<T> &s;
public:
  smoc_storage_wom(smoc_storage<T> &_s)
    : s(_s) {}

  void operator=(const T& t)
    { s.put(t); }  
};

/// smoc storage with read write memory interface
template<class T>
class smoc_storage_rw
{
private:
  typedef smoc_storage_wom<T> this_type;
private:
  smoc_storage<T> &s;
public:
  smoc_storage_rw(smoc_storage<T> &_s)
    : s(_s) {}
 
  operator const T&() const
    { return s.get(); }

  void operator=(const T& t)
    { s.put(t); }  
};

namespace CoSupport {
  // provide isType for smoc_storage_rw
  template <typename T, typename X>
  static inline
  bool isType( const smoc_storage_rw<X> &of )
   { return isType<T>(static_cast<const X &>(of)); }
};

template<class T>
struct smoc_storage_in
{
  typedef const smoc_storage<T>  storage_type;
  typedef const T               &return_type;
};

template<class T>
struct smoc_storage_out
{
  typedef smoc_storage<T>        storage_type;
  typedef smoc_storage_wom<T>    return_type;
};

template<class T>
struct smoc_storage_inout
{
  typedef smoc_storage<T>        storage_type;
  typedef smoc_storage_rw<T>     return_type;
};

template<>
struct smoc_storage_in<void>
{
  typedef const void storage_type;
  typedef const void return_type;
};

template<>
struct smoc_storage_out<void>
{
  typedef void storage_type;
  typedef void return_type;
};

template<>
struct smoc_storage_inout<void>
{
  typedef void storage_type;
  typedef void return_type;
};

#endif // SMOC_STORAGE_HPP
