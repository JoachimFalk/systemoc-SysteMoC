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

#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_graph.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

smoc_multiplex_fifo_chan_base::smoc_multiplex_fifo_chan_base(const chan_init &i)
#ifdef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
	: smoc_root_chan(),
#else
	: smoc_root_chan(i.name),
#endif
	
#ifdef SYSTEMOC_ENABLE_VPC
    smoc::Detail::QueueFRVWPtr(i.n),
#else
    smoc::Detail::QueueRWPtr(i.n),
#endif
    fifoOutOfOrder(i.m)
{
  assert(fifoOutOfOrder + 1 <= depthCount());
}

void smoc_multiplex_fifo_chan_base::registerVOutlet(const VOutletMap::value_type &entry) {
  sassert(vOutlets.insert(entry).second);
}

void smoc_multiplex_fifo_chan_base::deregisterVOutlet(FifoId fifoId) {
  vOutlets.erase(fifoId);
}

void smoc_multiplex_fifo_chan_base::registerVEntry(const VEntryMap::value_type &entry) {
  sassert(vEntries.insert(entry).second);
}

void smoc_multiplex_fifo_chan_base::deregisterVEntry(FifoId fifoId) {
  vEntries.erase(fifoId);
}
