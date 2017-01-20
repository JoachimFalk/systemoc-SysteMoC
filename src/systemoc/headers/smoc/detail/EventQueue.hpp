//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_SMOC_DETAIL_EVENTQUEUE_HPP
#define _INCLUDED_SMOC_DETAIL_EVENTQUEUE_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <list>

# include <boost/function.hpp>

# include <CoSupport/compatibility-glue/nullptr.h>

# include <smoc/smoc_event.hpp>

# include <vpc.hpp>

namespace smoc { namespace Detail {

template<typename T>
class EventQueue
: protected smoc::smoc_event_listener {
private:
  typedef EventQueue this_type;
protected:
  typedef std::pair<T, smoc_vpc_event_p>   Entry;
  typedef std::list<Entry>                 Queue;
protected:
  Queue                            queue;
  const boost::function<void (T)>  process;
  const boost::function<void (T)>  process_dropped;
protected:

  template<typename TT>
  void signaled_helper(std::list<std::pair<TT, smoc_vpc_event_p> > &queue){
    do {
      process(queue.front().first);
      queue.pop_front();
    }
    while(!queue.empty() && *queue.front().second);
  }

  void signaled_helper(std::list<std::pair<size_t, smoc_vpc_event_p> > &queue){
    size_t n = 0;
    do {
        if(queue.front().second->isDropped()){
          assert(process_dropped); //if a message/task is dropped - a callback MUST be defined
          size_t m = queue.front().first;
          if(n!= 0){
            process(n);
            n=0;
          }
          //remove it from queues
          process_dropped(m);
        }else{
          n += queue.front().first;
        }
        queue.pop_front();
      }while(!queue.empty() && *queue.front().second);
    if(n!=0) process(n);
  }

  /// @brief See smoc_event_listener
  void signaled(smoc::smoc_event_waiter *e){
    assert(*e);
    assert(!queue.empty());
    assert(e == queue.front().second.get());
    
    e->delListener(this);
        
    this->signaled_helper(queue);

    if(!queue.empty())
      queue.front().second->addListener(this);    
  }

  void eventDestroyed(smoc::smoc_event_waiter *_e)
    { assert(!"eventDestroyed must never be called!"); }
public:
  EventQueue(const boost::function<void (T)> &proc, const boost::function<void (T)> &proc_drop =0)
    : process(proc), process_dropped(proc_drop) {}

  /// @brief Queue event  
  void addEntry(T t, const smoc_vpc_event_p& le) {
    bool queueEmpty = queue.empty();
    
    assert(nullptr != le); //if(queueEmpty && (!le || *le)) {
    if(queueEmpty && *le) {
      // shortcut processing
      process(t);
    } else {
      queue.push_back(Entry(t, le));
      if(queueEmpty)
        le->addListener(this);
    }
  }
  void dump(){
    for (typename Queue::iterator iter = queue.begin();
        iter != queue.end();
        ++iter){
      Entry &e = *iter;
      dump_helper(e);
    }
  }
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_VPC

#endif //_INCLUDED_SMOC_DETAIL_EVENTQUEUE_HPP
