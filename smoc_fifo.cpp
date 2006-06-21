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

const char* const smoc_fifo_kind::kind_string = "smoc_fifo";

namespace smoc_detail {
#ifdef ENABLE_SYSTEMC_VPC
  void LatencyQueue::VisibleQueue::doSomething(size_t n)
    { getTop().fifo->incrVisible(n); }

#if 0
  bool LatencyQueue::signaled(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "smoc_fifo_kind<X>::wpp::_::signaled(...)" << std::endl;
# endif
    assert(!queue.empty());
    assert(_e == &*queue.front().second);
    assert(*_e);
    _e->delListener(this);
    size_t visible;
    do {
      visible = queue.front().first;
      queue.pop(); // pop from front of queue
    } while (!queue.empty() && *queue.front().second);
    fifo->incrVisible(visible);
    if (!queue.empty())
      queue.front().second->addListener(this);
    return false;
  }

  void LatencyQueue::eventDestroyed(smoc_event_waiter *_e) {
    assert(1 ? 0 : "eventDestroyed must never be called !!!");
  }

  void LatencyQueue::addEntry(size_t n, const smoc_ref_event_p &le) {
    bool queueEmpty = queue.empty();
    
    if (!queueEmpty || (le && !*le)) {
      queue.push(Entry(n, le)); // insert at back of queue
      if (queueEmpty)
        le->addListener(this);
    } else {
      // latency event allready signaled and top of latencyQueue or no latency event
      fifo->incrVisible(n);
    }
  }
#endif
#endif // ENABLE_SYSTEMC_VPC
};

