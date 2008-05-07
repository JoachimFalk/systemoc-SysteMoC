//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SMOC_RING_ACCESS_HPP
#define _INCLUDED_SMOC_RING_ACCESS_HPP

template<class S, class T>
class smoc_ring_access
: public smoc_channel_access<T> {
public:
  typedef T                     return_type;
  typedef S                     storage_type;
  typedef smoc_ring_access<S,T> this_type;
private:
#ifndef NDEBUG
  size_t        limit;
#endif
  storage_type *storage;
  size_t        storageSize;
  size_t       *offset;
public:
  smoc_ring_access(storage_type *storage, size_t storageSize, size_t *offset):
#ifndef NDEBUG
      limit(0),
#endif
      storage(storage), storageSize(storageSize), offset(offset) {}

#ifndef NDEBUG
  void setLimit(size_t l) { limit = l; }
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }

  return_type operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
  const return_type operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
};

template <>
class smoc_ring_access<void, void>
: public smoc_channel_access<void> {
public:
  typedef void                        return_type;
  typedef void                        storage_type;
  typedef smoc_ring_access<void,void> this_type;
private:
public:
  smoc_ring_access()
    {}

#ifndef NDEBUG
  void setLimit(size_t) {}
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }
};

#endif // _INCLUDED_SMOC_RING_ACCESS_HPP
