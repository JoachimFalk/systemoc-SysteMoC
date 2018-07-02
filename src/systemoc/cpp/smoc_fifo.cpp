// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_fifo.hpp>
#include <smoc/smoc_graph.hpp>

smoc_fifo_chan_base::smoc_fifo_chan_base(const chan_init& i)
: ChanBase(
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
    i.name
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  ),
#ifdef SYSTEMOC_ENABLE_ROUTING
  QueueFRVWPtr(i.n),
#else //!SYSTEMOC_ENABLE_ROUTING
  QueueRWPtr(i.n),
#endif //!SYSTEMOC_ENABLE_ROUTING
  tokenId(0)
{}

void smoc_fifo_chan_base::before_end_of_elaboration() {
  ChanBase::before_end_of_elaboration();
  assert(getEntries().size() == 1);
  assert(getOutlets().size() == 1);
}

void smoc_fifo_chan_base::end_of_simulation() {
  ChanBase::end_of_simulation();
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg
            << this->name() << "\t"
            << this->visibleCount() << "\t"
            << this->qfSize() << "\t"
            << this->freeCount() << "\t"
            << this->usedCount() << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void smoc_fifo_chan_base::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_fifo_chan_base::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  ChanBase::doReset();
  // queue and initial tokens set up by smoc_fifo_storage...
  emmSpace.reset();
  emmData.reset();

  emmSpace.increasedCount(freeCount());
  emmData.increasedCount(visibleCount());
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_fifo_chan_base::doReset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}
