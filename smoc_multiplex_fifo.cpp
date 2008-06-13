// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
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

#include <systemoc/smoc_config.h>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

smoc_multiplex_fifo_chan_base::smoc_multiplex_fifo_chan_base(const chan_init &i)
  : smoc_root_chan(i.name),
#ifdef SYSTEMOC_ENABLE_VPC
    Detail::QueueFRVWPtr(i.n),
#else
    Detail::QueueRWPtr(i.n),
#endif
    fifoOutOfOrder(i.m),
#ifdef SYSTEMOC_ENABLE_VPC
    latencyQueue(this),
    diiQueue(this),
#endif
    tokenId(0)
{
}

void smoc_multiplex_fifo_chan_base::registerVFifo(smoc_multiplex_vfifo_chan_base *vfifo) {
  vFifos[vfifo->fifoId] = vfifo;
}

void smoc_multiplex_fifo_chan_base::deregisterVFifo(smoc_multiplex_vfifo_chan_base *vfifo) {
  vFifos.erase(vfifo->fifoId);
  for (FifoSequence::iterator iter = fifoSequence.begin();
       iter !=  fifoSequence.end();
       ) {
    if (*iter == vfifo->fifoId) {
      iter = fifoSequence.erase(iter);
    } else {
      ++iter;
    }
  }
}

#ifdef SYSTEMOC_ENABLE_VPC
  void smoc_multiplex_fifo_chan_base::consume(FifoId from, size_t n, const smoc_ref_event_p &diiEvent)
#else
  void smoc_multiplex_fifo_chan_base::consume(FifoId from, size_t n)
#endif
{
//std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [BEGIN]" << std::endl;
//std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;

  rpp(n);
#ifdef SYSTEMOC_ENABLE_VPC
  // Delayed call of diiExpired(consume);
  diiQueue.addEntry(n, diiEvent);
#else
  diiExpired(n);
#endif
  
  for (FifoSequence::iterator iter = fifoSequenceOOO.begin();
       n > 0;
       ) {
    assert(iter !=  fifoSequenceOOO.end());
    if (*iter == from) {
      if (visibleCount() >= fifoOutOfOrder + n) {
        FifoId fId = fifoSequence.front();
        std::map<FifoId, smoc_multiplex_vfifo_chan_base *>::iterator fIter =
          vFifos.find(fId);
        fifoSequence.pop_front();
        fifoSequenceOOO.push_back(fId);
        fIter->second->latencyExpired(1);
      }
      iter = fifoSequenceOOO.erase(iter); --n;
    } else {
      ++iter;
    }
  }

//std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [END]" << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;
}

void smoc_multiplex_fifo_chan_base::diiExpired(size_t n) {
//std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [BEGIN]" << std::endl;
//std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;

  fpp(n);
  emmFree.increasedCount(freeCount());

//std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [END]" << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_multiplex_fifo_chan_base::produce(FifoId to, size_t n, const smoc_ref_event_p &le)
#else
void smoc_multiplex_fifo_chan_base::produce(FifoId to, size_t n)
#endif
{
//std::cerr << "smoc_multiplex_fifo_chan_base::produce(" << to << ", " << n << ") [BEGIN]" << std::endl;
//std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;

  wpp(n);
  for (size_t j = n; j > 0; --j)
    fifoSequence.push_back(to);
  emmFree.decreasedCount(freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
  // Delayed call of latencyExpired(n)
  latencyQueue.addEntry(n, le);
#else
  latencyExpired(n);
#endif

//std::cerr << "smoc_multiplex_fifo_chan_base::produce(" << to << ", " << n << ") [END]" << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;
}

void smoc_multiplex_fifo_chan_base::latencyExpired(size_t n) {
//std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [BEGIN]" << std::endl;
//std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  vpp(n);
  for (;
       visibleCount() - n <= fifoOutOfOrder && n > 0;
       --n) {
//  std::cerr << "n == " << n << std::endl;
    FifoId fId = fifoSequence.front();
    std::map<FifoId, smoc_multiplex_vfifo_chan_base *>::iterator fIter =
      vFifos.find(fId);
    fifoSequence.pop_front();
    fifoSequenceOOO.push_back(fId);
    fIter->second->latencyExpired(1);
  }

//std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [END]" << std::endl;
//std::cerr << "freeCount():    " << freeCount() << std::endl;
//std::cerr << "usedCount():    " << usedCount() << std::endl;
//std::cerr << "visibleCount(): " << visibleCount() << std::endl;
}

smoc_multiplex_vfifo_chan_base::smoc_multiplex_vfifo_chan_base( const chan_init &i )
  : //smoc_nonconflicting_chan(i.name),
    QueueRVWPtr(i.pChanImpl->depthCount()),
    fifoId(i.fifoId),
    pChanImpl(i.pChanImpl),
    tokenId(0)
{
  pChanImpl->registerVFifo(this);
}

smoc_multiplex_vfifo_chan_base::~smoc_multiplex_vfifo_chan_base() {
  pChanImpl->deregisterVFifo(this);
}
