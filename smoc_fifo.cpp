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
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

const char* const smoc_fifo_kind::kind_string = "smoc_fifo";

using namespace SysteMoC::NGX;
using namespace SysteMoC::NGXSync;

namespace smoc_detail {
#ifdef SYSTEMOC_ENABLE_VPC

# ifdef SYSTEMOC_TRACE
  struct DeferedTraceLogDumper
  : public smoc_event_listener {
    smoc_ref_event_p  event;
    smoc_fifo_kind   *fifo;
    const char       *mode;

    void signaled(smoc_event_waiter *_e) {
//    const char *name = fifo->name();
      
      TraceLog.traceStartActor(fifo, mode);
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper::signaled(...)" << std::endl;
#   endif
      assert(_e == event.get());
      assert(*_e);
      event = NULL;
      TraceLog.traceEndActor(fifo);
      return;
    }
    void eventDestroyed(smoc_event_waiter *_e) {
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper:: eventDestroyed(...)" << std::endl;
#   endif
      delete this;
    }

    DeferedTraceLogDumper
      (const smoc_ref_event_p &event, smoc_fifo_kind *fifo, const char *mode)
      : event(event), fifo(fifo), mode(mode) {};
 
    virtual ~DeferedTraceLogDumper() {}
  };
# endif
  
};

#endif

smoc_fifo_kind::smoc_fifo_kind( const chan_init &i )
  : smoc_nonconflicting_chan(
    i.name != NULL ? i.name : sc_gen_unique_name( "smoc_fifo" ) ),
#ifdef SYSTEMOC_ENABLE_VPC
    latencyQueue(this), 
#endif
    fsize(i.n+1),
    rindex(0),
#ifdef SYSTEMOC_ENABLE_VPC
    vindex(0), 
#endif
    windex(0),
    tokenId(0)
{
  // NGX --> SystemC
  if(NGXConfig::getInstance().hasNGX()) {

    Fifo::ConstPtr fifo =
      objAs<Fifo>(NGXCache::getInstance().get(this));

    if(fifo) {
      fsize = fifo->size().get() + 1;
    }
    else {
      // XML node missing or no Fifo
    }
  }

  // for lazy % overflow protection fsize must be less than half the datatype
  //  size
  assert(fsize < (MAX_TYPE(size_t) >> 1));
}
