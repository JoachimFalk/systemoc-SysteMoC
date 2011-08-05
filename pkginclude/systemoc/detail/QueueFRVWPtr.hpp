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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUEVRVWPTR_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUEVRVWPTR_HPP

#include <systemoc/smoc_config.h>

#include "QueueRVWPtr.hpp"

namespace Detail {

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

    /*
     * method updateInvalidateToken updates the indexes, after the storage has been updated.
     */
    void updateInvalidateToken(size_t n){
        vindex = (MG(vindex, fsize) +n ).getValue();
        rindex = (MG(rindex, fsize) +n ).getValue();
       }
  };

} // namespace Detail

#endif // _INCLUDED_SMOC_DETAIL_QUEUEFRVWPTR_HPP
