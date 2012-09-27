//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_DETAIL_SMOC_RING_ACCESS_HPP
#define _INCLUDED_DETAIL_SMOC_RING_ACCESS_HPP

#include <systemoc/smoc_config.h>

#include "smoc_sysc_port.hpp"

template<class S, class T>
class smoc_ring_access: public smoc_1d_port_access_if<T> {
  typedef smoc_ring_access<S,T> this_type;
public:
  typedef T                     return_type;
  typedef S                     storage_type;
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

template <>
class smoc_ring_access<void, void>: public smoc_1d_port_access_if<void> {
  typedef smoc_ring_access<void,void> this_type;
public:
  typedef void                        return_type;
  typedef void                        storage_type;
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

#endif // _INCLUDED_DETAIL_SMOC_RING_ACCESS_HPP
