// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <smoc_fifo.hpp>

#ifdef ENABLE_SYSTEMC_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //ENABLE_SYSTEMC_VPC

const char* const smoc_fifo_kind::kind_string = "smoc_fifo";

namespace smoc_detail {
#ifdef ENABLE_SYSTEMC_VPC
  void LatencyQueue::RequestQueue::doSomething(size_t n) {
    for (;n > 0; --n) {
      smoc_ref_event_p le(new smoc_ref_event());
      SystemC_VPC::EventPair p(&dummy, &*le);
      SystemC_VPC::Director::getInstance().  	
        compute( getTop().fifo->name(), "1", p);
      getTop().visibleQueue.addEntry(1, le);
    }
  }

  void LatencyQueue::VisibleQueue::doSomething(size_t n) {
    getTop().fifo->incrVisible(n);
  }
#endif // ENABLE_SYSTEMC_VPC
};

