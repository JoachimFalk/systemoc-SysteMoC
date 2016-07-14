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

//#include <systemoc/detail/smoc_ngx_sync.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_graph_type.hpp>

size_t fsizeMapper(sc_core::sc_object* instance, size_t n);

smoc_fifo_chan_base::smoc_fifo_chan_base(const chan_init& i)
: smoc_root_chan(
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
    i.name
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  ),
#ifdef SYSTEMOC_ENABLE_VPC
  QueueFRVWPtr(fsizeMapper(this, i.n)),
  latencyQueue(std::bind1st(std::mem_fun(&this_type::latencyExpired), this), this, std::bind1st(std::mem_fun(&this_type::latencyExpired_dropped), this)),
  diiQueue(std::bind1st(std::mem_fun(&this_type::diiExpired), this)),
#else
  QueueRWPtr(fsizeMapper(this, i.n)),
#endif
  tokenId(0)
{}

void smoc_fifo_chan_base::finalise() {
  smoc_root_chan::finalise();
  assert(getEntries().size() == 1);
  assert(getOutlets().size() == 1);
}

size_t fsizeMapper(sc_core::sc_object* instance, size_t n) {
//FIXME: Reimplememt this!
/*// SGX --> SystemC
  if (smoc::Detail::NGXConfig::getInstance().hasNGX()) {
    SystemCoDesigner::SGX::Fifo::ConstPtr fifo =
      objAs<SystemCoDesigner::SGX::Fifo>(smoc::Detail::NGXCache::getInstance().get(instance));
    if (fifo) {
      n = fifo->size().get();
    } else {
      // XML node missing or no Fifo
    }
  }*/
  return n;
}
