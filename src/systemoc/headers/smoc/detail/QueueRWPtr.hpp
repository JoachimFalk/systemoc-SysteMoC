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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUERWPTR_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUERWPTR_HPP

#include <CoSupport/Math/ModuloGroup.hpp>

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

  class QueueRVWPtr;
  class QueueFRVWPtr;

  class QueueRWPtr {
    friend class QueueRVWPtr;
    friend class QueueFRVWPtr;
  public:
    typedef CoSupport::Math::ModuloGroup<CoSupport::Math::Modulus<size_t> > MG;
  private:
    /*             fsize
     *   ____________^___________
     *  /                        \
     * |FFVVVVVVVVVVVVVVVVVVVVVFFF|
     *    ^                    ^
     *  rindex               windex
     *  findex               vindex
     *
     *  F: The free space area of size (findex - windex - 1) % fsize
     *  V: The visible token area of size (vindex - rindex) % fsize
     */

    MG::M   fsize;   ///< Ring buffer size == FIFO size + 1
    size_t  rindex;  ///< The FIFO read    ptr
    size_t  windex;  ///< The FIFO write   ptr
  public:
    QueueRWPtr(size_t n)
      : fsize(n + 1), rindex(0), windex(0) {}

    size_t usedCount() const {
      // windex - rindex in modulo fsize arith
      return (MG(windex, fsize) - rindex).getValue();
    }

    // For two ptr queues the visible and the used
    // token sets are equal.
    size_t visibleCount() const {
      return usedCount();
    }

    size_t depthCount() const {
      return fsize.m() - 1;
    }

    size_t freeCount() const {
      // rindex - windex - 1 in modulo fsize arith
      return (MG(rindex, fsize) - windex - 1).getValue();
    }

    size_t qfSize() const {
      return fsize.m();
    }
    const size_t &rIndex() const {
      return rindex;
    }
    const size_t &fIndex() const {
      return rindex;
    }
    const size_t &wIndex() const {
      return windex;
    }
    const size_t &vIndex() const {
      return windex;
    }

    void wpp(size_t n) {
      assert(n <= freeCount());
      // windex = windex + n in modulo fsize arith
      windex = (MG(windex, fsize) + n).getValue();
    }

    void vpp(size_t n) {
      // Dummy does nothing
    }

    void rpp(size_t n) {
      assert(n <= visibleCount());
      // rindex = rindex + n in modulo fsize arith
      rindex = (MG(rindex, fsize) + n).getValue();
    }

    void fpp(size_t n) {
      // Dummy does nothing
    }

    void resetQueue() {
      rindex = 0;
      windex = 0;
    }

    void resize(size_t n) {
      assert(rindex == 0);
      assert(windex <= n);
      fsize = MG::M(n + 1);
    }

    /// Invalidate n tokens in the visible area. Update rindex assuming that
    /// the tokens in the queue are moved to the right. This method will be
    /// overloaded in QueueFRVWPtr to also update the findex (free index).
    void dropRVisible(size_t n)   { rpp(n); }
  };

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_QUEUERWPTR_HPP */
