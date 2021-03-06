// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_RINGACCESS_HPP
#define _INCLUDED_SMOC_DETAIL_RINGACCESS_HPP

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

template<class T, class PortIf>
class RingAccess: public PortIf {
  typedef RingAccess<T,PortIf>                 this_type;
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
  RingAccess(storage_type *ringStorage, size_t ringStorageSize, const size_t *ringOffset):
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

protected:
#ifdef SYSTEMOC_ENABLE_SGX
  void resize(storage_type *ringStorage, size_t ringStorageSize) {
    this->ringStorage     = ringStorage;
    this->ringStorageSize = ringStorageSize;
  }
#endif // SYSTEMOC_ENABLE_SGX
};

template <class PortIf>
class RingAccess<void, PortIf>: public PortIf {
  typedef RingAccess<void, PortIf> this_type;
public:
  typedef void                           return_type;
  typedef void                           storage_type;
private:
public:
  RingAccess()
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

} } // smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_RINGACCESS_HPP */
