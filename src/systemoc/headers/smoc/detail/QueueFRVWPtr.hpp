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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUEFRVWPTR_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUEFRVWPTR_HPP

#include <systemoc/smoc_config.h>

#include "QueueRVWPtr.hpp"

namespace smoc { namespace Detail {

  class QueueFRVWPtr: public QueueRVWPtr {
    typedef QueueFRVWPtr  this_type;
    typedef QueueRVWPtr   base_type;
  protected:
    /*             fsize
     *   ____________^___________
     *  /                        \
     * |FFCCCCCCCVVVVVVVPPPPPPPFFF|
     *    ^      ^      ^      ^
     *  findex rindex vindex windex
     *
     *  F: The free space area of size (findex - windex - 1) % fsize
     *  V: The visible token area of size (vindex - rindex) % fsize
     *  P: The token which are still in the pipeline (latency not expired)
     *  C: The token which are in the process of being consumed (actor dii not expired)
     */
    size_t findex;  ///< The FIFO free    ptr
  public:
    QueueFRVWPtr(size_t n)
      : QueueRVWPtr(n), findex(0) {}

    size_t freeCount() const {
      // findex - windex - 1 in modulo fsize arith
      return (MG(findex, fsize) - windex - 1).getValue();
    }

    const size_t &fIndex() const {
      return findex;
    }

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // This differs from QueueRWPtr::wpp due to the different definition of
    // freeCount. Therefore, the assertion is more paranoid.
    void wpp(size_t n) {
      assert(n <= freeCount());
      base_type::wpp(n);
    }
#endif

    void fpp(size_t n) {
      // PRECONDITION PARANOIA: windex < findex <= rindex in modulo fsize arith
      //   windex == rindex   implies true
      //   rindex-windex == 1 implies findex == rindex
      assert(MG(findex, fsize).between(MG(windex, fsize) + 1, rindex));
/*    assert(findex < fsize &&
        (windex >= rindex && (findex >  windex || findex <= rindex) ||
         windex <  rindex && (findex >  windex && findex <= rindex)));
 */
      // inconsume = rindex - finex in modulo fsize arith
      assert(n <= (MG(rindex, fsize) - findex).getValue());
      // findex = findex + n in modulo fsize arith
      findex = (MG(findex, fsize) + n).getValue();
    }

    void resetQueue() {
      findex = 0;
      base_type::resetQueue();
    }

    void resize(size_t n) {
      assert(findex == 0);
      base_type::resize(n);
    }

    /// Invalidate n tokens in the visible area. Update findex and rindex
    /// assuming that the tokens in the queue are moved to the right.
    void dropRVisible(size_t n)   { base_type::dropRVisible(n); fpp(n); }
    /// Invalidate n tokens in the invisible area. Update findex, rindex, and
    /// vindex assuming that the tokens in the queue are moved to the right.
    void dropRInvisible(size_t n) { base_type::dropRInvisible(n); fpp(n); }
  };

} } // namespace Detail

#endif /* _INCLUDED_SMOC_DETAIL_QUEUEFRVWPTR_HPP */
