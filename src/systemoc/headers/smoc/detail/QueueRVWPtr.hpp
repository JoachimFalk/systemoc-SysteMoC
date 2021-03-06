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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUERVWPTR_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUERVWPTR_HPP

#include <systemoc/smoc_config.h>

#include "QueueRWPtr.hpp"

namespace smoc { namespace Detail {

  class QueueRVWPtr: public QueueRWPtr {
    typedef QueueRVWPtr this_type;
    typedef QueueRWPtr  base_type;

    friend class QueueFRVWPtr;
  protected:
    typedef QueueRVWPtr queue_type;

    /*             fsize
     *   ____________^___________
     *  /                        \
     * |FFFFFFFFFVVVVVVVPPPPPPPFFF|
     *           ^      ^      ^
     *         rindex vindex windex
     *         findex
     *  F: The free space area of size (findex - windex - 1) % fsize
     *  V: The visible token area of size (vindex - rindex) % fsize
     *  P: The token which are still in the pipeline (latency not expired)
     */
  private:
    size_t vindex;  ///< The FIFO visible ptr
  public:
    QueueRVWPtr(size_t n)
      : QueueRWPtr(n), vindex(0) {}

    size_t visibleCount() const {
      // vindex - rindex in modulo fsize arith
      return (MG(vindex, fsize) - rindex).getValue();
    }

    const size_t &vIndex() const {
      return vindex;
    }

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // This differs from QueueRWPtr::rpp due to the different definition of
    // visibleCount. Therefore, the assertion is more paranoid.
    void rpp(size_t n) {
      assert(n <= visibleCount());
      base_type::rpp(n);
    }
#endif

    void vpp(size_t n) {
      // PRECONDITION PARANOIA: rindex <= vindex <= windex in modulo fsize arith
      //   rindex == windex   implies rindex == windex == vindex
      //   rindex-windex == 1 implies true
      assert(MG(vindex, fsize).between(rindex, windex));
/*    assert(vindex < fsize &&
        (windex >= rindex && (vindex >= rindex && vindex <= windex) ||
         windex <  rindex && (vindex >= rindex || vindex <= windex)));
 */
      // invisible = windex - vindex in modulo fsize arith
      assert(n <= (MG(windex, fsize) - vindex).getValue());
      // vindex = vindex + n in modulo fsize arith
      vindex = (MG(vindex, fsize) + n).getValue();
    }

    void resetQueue() {
      vindex = 0;
      base_type::resetQueue();
    }

#ifdef SYSTEMOC_ENABLE_SGX
    void resize(size_t n) {
      assert(vindex == 0);
      base_type::resize(n);
    }
#endif //SYSTEMOC_ENABLE_SGX

    /// Invalidate n tokens in the invisible area. Update rindex and vindex
    /// assuming that the tokens in the queue are moved to the right. This
    /// method will be overloaded in QueueFRVWPtr to also update the findex
    /// (free index).
    void dropRInvisible(size_t n) { vpp(n); base_type::dropRVisible(n); }
  };

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_QUEUERVWPTR_HPP */
