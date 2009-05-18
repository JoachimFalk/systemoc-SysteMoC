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

#include <systemoc/smoc_multireader_fifo.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/smoc_graph_type.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

//using namespace SystemCoDesigner::SGX;
using namespace SysteMoC::NGXSync;
  
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

#ifndef __SCFE__
void smoc_multireader_fifo_chan_base::finalise() {
  // FIXME: need name before XML can be constructed
  generateName();
  assembleXML();
  smoc_root_chan::finalise();
}

void smoc_multireader_fifo_chan_base::assembleXML() {
  using namespace SystemCoDesigner::SGX;

  assert(!fifo);

  Fifo _fifo(this->name());
  fifo = &_fifo;
  proc = fifo;

  // set some attributes
  fifo->size().set(depthCount());

  smoc_graph_base* parent =
    dynamic_cast<smoc_graph_base*>(get_parent_object());

  if(parent)
    parent->addProcess(_fifo);
  else
    assert(!"FIFO has no parent!");
}
#endif

void smoc_multireader_fifo_chan_base::assemble(smoc_modes::PGWriter &pgw) const {
 
  // TODO: Implement

  IdAttr idChannel = idPool.printId(this);
  
  pgw << "<process name=\"" << name() << "\" "
                  "type=\"multireader_fifo\" "
                  "id=\"" << idChannel << "\">" << std::endl;

  pgw << "</process>" << std::endl;
}

void smoc_multireader_fifo_chan_base::channelAttributes(smoc_modes::PGWriter &pgw) const {
  pgw << "<attribute type=\"size\" value=\"" << depthCount() << "\"/>" << std::endl;
}
  
#ifdef SYSTEMOC_ENABLE_VPC
void smoc_multireader_fifo_chan_base::consume(size_t n, const smoc_ref_event_p &diiEvent)
#else
void smoc_multireader_fifo_chan_base::consume(size_t n)
#endif
{
#ifdef SYSTEMOC_TRACE
  TraceLog.traceCommExecIn(this, n);
#endif
  rpp(n);
  lessData(n);
#ifdef SYSTEMOC_ENABLE_VPC
  // Delayed call of diiExpired
  diiQueue.addEntry(n, diiEvent);
#else
  // Immediate call of diiExpired
  diiExpired(n);
#endif
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_multireader_fifo_chan_base::produce(size_t n, const smoc_ref_event_p &latEvent)
#else
void smoc_multireader_fifo_chan_base::produce(size_t n)
#endif
{
#ifdef SYSTEMOC_TRACE
  TraceLog.traceCommExecOut(this, n);
#endif
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
    i->first->moreData();
  }
}

void smoc_multireader_fifo_chan_base::lessData(size_t n) {
  //std::cout << "less data available: " << visibleCount() << std::endl;
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->lessData();
  }
}

void smoc_multireader_fifo_chan_base::moreSpace(size_t n) {
  //std::cout << "more space available: " << freeCount() << std::endl;
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->moreSpace();
  }
}

void smoc_multireader_fifo_chan_base::lessSpace(size_t n) {
  //std::cout << "less space available: " << freeCount() << std::endl;
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->lessSpace();
  }
}

void smoc_multireader_fifo_chan_base::start_of_simulation() {
  // this should account for initial tokens / space so our events
  // can always be created unnotified (and the scheduler needs not
  // to know about initial tokens?)
  moreSpace(freeCount()); moreData(visibleCount());
}

size_t smoc_multireader_fifo_chan_base::inTokenId() const {
  return tokenId - usedCount();
}

size_t smoc_multireader_fifo_chan_base::outTokenId() const {
  return tokenId;
}
  
size_t smoc_multireader_fifo_chan_base::numAvailable() const {
  return visibleCount();
}

size_t smoc_multireader_fifo_chan_base::numFree() const {
  return freeCount();
}
