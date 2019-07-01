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

#ifndef _INCLUDED_SYSTEMOC_DETAIL_SMOC_RING_ACCESS_HPP
#define _INCLUDED_SYSTEMOC_DETAIL_SMOC_RING_ACCESS_HPP

#include <systemoc/smoc_config.h>

#include "smoc_chan_if.hpp"

template<class T, class PortIf>
class smoc_ring_access: public PortIf {
  typedef smoc_ring_access<T,PortIf>                 this_type;
public:
  typedef typename PortIf::access_type::return_type  return_type;
  typedef smoc::Detail::Storage<T>                   storage_type;
private:
#if defined(SYSTEMOC_ENABLE_DEBUG)
  mutable size_t ringLimit;
#endif
  storage_type *ringStorage;
  size_t        ringStorageSize;
  const size_t *ringOffset;
public:
  smoc_ring_access(storage_type *ringStorage, size_t ringStorageSize, const size_t *ringOffset):
#if defined(SYSTEMOC_ENABLE_DEBUG)
      ringLimit(0),
#endif
      ringStorage(ringStorage), ringStorageSize(ringStorageSize), ringOffset(ringOffset) {}

#if defined(SYSTEMOC_ENABLE_DEBUG)
  void setLimit(size_t l) { ringLimit = l; }
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }

  return_type operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
#if defined(SYSTEMOC_ENABLE_DEBUG)
    assert(n < ringLimit);
#endif
    return *ringOffset + n < ringStorageSize
      ? ringStorage[*ringOffset + n]
      : ringStorage[*ringOffset + n - ringStorageSize];
  }
  const return_type operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
#if defined(SYSTEMOC_ENABLE_DEBUG)
    assert(n < ringLimit);
#endif
    return *ringOffset + n < ringStorageSize
      ? ringStorage[*ringOffset + n]
      : ringStorage[*ringOffset + n - ringStorageSize];
  }
};

template <class PortIf>
class smoc_ring_access<void, PortIf>: public PortIf {
  typedef smoc_ring_access<void, PortIf> this_type;
public:
  typedef void                           return_type;
  typedef void                           storage_type;
private:
public:
  smoc_ring_access()
    {}

#if defined(SYSTEMOC_ENABLE_DEBUG)
  void setLimit(size_t) {}
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }
};

#endif /* _INCLUDED_SYSTEMOC_DETAIL_SMOC_RING_ACCESS_HPP */
