// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
  
  T& get() {
    assert(valid);
    return *ptr();
  }

  const T& get() const {
    assert(valid);
    return *ptr();
  }

  operator T&() {
    assert(valid);
    return *ptr();
  }

  operator const T&() const {
    assert(valid);
    return *ptr();
  }
  
  void put(const T &t) {
    if(valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }
  
  ~smoc_storage() {
    if(valid) {
      ptr()->~T();
      valid = false;
    }
  }
};

template<class T>
class smoc_write_only_storage
{
public:
  typedef smoc_write_only_storage<T> this_type;
  
private:
  smoc_storage<T> &s;

public:
  smoc_write_only_storage(smoc_storage<T> &_s) :
    s(_s)
  {}
  
  void operator=(const T& t) {
    s.put(t);
  }  
};


template<class T>
struct smoc_storage_in
{
  typedef const smoc_storage<T> storage_type;
  typedef const T &		return_type;
};

template<class T>
struct smoc_storage_out
{
  typedef smoc_storage<T>	     storage_type;
  typedef smoc_write_only_storage<T> return_type;
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

#endif // SMOC_STORAGE_HPP
