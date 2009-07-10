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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_multireader_fifo.hpp>
#include <systemoc/smoc_graph_type.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

smoc_multireader_fifo_chan_base::smoc_multireader_fifo_chan_base(const chan_init &i)
  : smoc_root_chan(i.name),
#ifdef SYSTEMOC_ENABLE_VPC
  QueueFRVWPtr(fsizeMapper(this, i.n)),
  latencyQueue(std::bind1st(std::mem_fun(&this_type::latencyExpired), this), this),
  diiQueue(std::bind1st(std::mem_fun(&this_type::diiExpired), this)),
#else
  QueueRWPtr(fsizeMapper(this, i.n)),
#endif
  tokenId(0),
  schedOutlets(i.so ? i.so : &schedDefault)
{}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_multireader_fifo_chan_base::consume(smoc_port_in_base_if *who, size_t n, const smoc_ref_event_p &diiEvent)
#else
void smoc_multireader_fifo_chan_base::consume(smoc_port_in_base_if *who, size_t n)
#endif
{
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_multireader_fifo_chan_base::consume this=\"" << this << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_TRACE
  TraceLog.traceCommExecIn(this, n);
#endif
#ifdef SYSTEMOC_DEBUG
  outDbg << "n: " << n << "; #avail: " << visibleCount()
         << "; #free: " << freeCount() << "; size: " << (fSize()-1) << std::endl;
#endif // SYSTEMOC_DEBUG
  rpp(n);
  lessData(n);
#ifdef SYSTEMOC_ENABLE_VPC
  // Delayed call of diiExpired
  diiQueue.addEntry(n, diiEvent);
#else
  // Immediate call of diiExpired
  diiExpired(n);
#endif
  // FIXME: This is an incredible hack to fix the GAU bug!!!
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    if (i->first == who)
      continue;
    smoc_root_node *src =
      dynamic_cast<smoc_root_node *>(getLeafPort(i->second)->get_parent());
    if (src == NULL)
      continue;
    smoc_graph_base *graph =
      dynamic_cast<smoc_graph_base *>(src->get_parent());
    if (graph == NULL)
      continue;
    graph->beforeStateChange(src);
    graph->afterStateChange(src);
  }
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_multireader_fifo_chan_base::consume>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_multireader_fifo_chan_base::produce(size_t n, const smoc_ref_event_p &latEvent)
#else
void smoc_multireader_fifo_chan_base::produce(size_t n)
#endif
{
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_multireader_fifo_chan_base::produce this=\"" << this << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_TRACE
  TraceLog.traceCommExecOut(this, n);
#endif
#ifdef SYSTEMOC_DEBUG
  outDbg << "n: " << n << "; #avail: " << visibleCount()
         << "; #free: " << freeCount() << "; size: " << (fSize()-1) << std::endl;
#endif // SYSTEMOC_DEBUG
  tokenId += n;
  wpp(n);
  lessSpace(n);
#ifdef SYSTEMOC_ENABLE_VPC
  // Delayed call of latencyExpired
  latencyQueue.addEntry(n, latEvent);
#else
  // Immediate call of latencyExpired
  latencyExpired(n);
#endif
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_multireader_fifo_chan_base::produce>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_multireader_fifo_chan_base::latencyExpired(size_t n) {
  vpp(n);
  moreData(n);
}

void smoc_multireader_fifo_chan_base::diiExpired(size_t n) {
  fpp(n);
  moreSpace(n);
}

void smoc_multireader_fifo_chan_base::moreData(size_t n) {
  //std::cout << "more data available: " << visibleCount() << std::endl;
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->moreData(n);
  }
}

void smoc_multireader_fifo_chan_base::lessData(size_t n) {
  //std::cout << "less data available: " << visibleCount() << std::endl;
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->lessData(n);
  }
}

void smoc_multireader_fifo_chan_base::moreSpace(size_t n) {
  //std::cout << "more space available: " << freeCount() << std::endl;
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->moreSpace(n);
  }
}

void smoc_multireader_fifo_chan_base::lessSpace(size_t n) {
  //std::cout << "less space available: " << freeCount() << std::endl;
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->lessSpace(n);
  }
}

void smoc_multireader_fifo_chan_base::doReset() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_multireader_fifo_chan_base::doReset name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  // queue and initial tokens set up by smoc_fifo_storage...
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->reset();
  }
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->reset();
  }

  moreSpace(freeCount());
  moreData(visibleCount());
    
  smoc_root_chan::doReset();

#ifdef SYSTEMOC_DEBUG
  outDbg << "#avail: " << visibleCount() << "; #free: " << freeCount()
         << "; size: " << (fSize()-1) << std::endl;
#endif // SYSTEMOC_DEBUG

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_multireader_fifo_chan_base::doReset>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

size_t smoc_multireader_fifo_chan_base::inTokenId() const {
  return tokenId - usedCount();
}

size_t smoc_multireader_fifo_chan_base::outTokenId() const {
  return tokenId;
}
