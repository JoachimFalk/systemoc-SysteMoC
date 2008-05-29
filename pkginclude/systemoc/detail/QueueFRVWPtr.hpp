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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUEVRVWPTR_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUEVRVWPTR_HPP

#include "QueueRVWPtr.hpp"

namespace Detail {

  class QueueFRVWPtr: public QueueRVWPtr {
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
      size_t unused =
        findex - windex - 1;
      
      if (unused > fsize)
        unused += fsize;
      return unused;
    }

#ifndef NDEBUG
    // This differs from QueueRWPtr::wpp due to the different definition of
    // freeCount. Therefore, the assertion is more paranoid.
    void wpp(size_t n) {
      assert(n <= freeCount());
      windex = (windex + n) % fsize;
    }
#endif

    void fpp(size_t n) {
#ifndef NDEBUG
      size_t inconsume =
        rindex - findex;
      if (inconsume > fsize)
        inconsume += fsize;
      assert(n <= inconsume);
#endif
      assert(n <= depthCount() - usedCount());
      findex = (findex + n) % fsize;
      // PARANOIA: windex <= findex <= rindex in modulo fsize arith
      assert(findex < fsize &&
        (rindex >= windex && (findex >= windex && findex <= rindex) ||
         rindex <  windex && (findex >= windex || findex <= rindex)));
    }
  };

} // namespace Detail

#endif // _INCLUDED_SMOC_DETAIL_QUEUEFRVWPTR_HPP
