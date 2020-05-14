// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <smoc/detail/NodeQueue.hpp>
#include <smoc/smoc_periodic_actor.hpp>

namespace smoc { namespace Detail {

NodeQueue::NodeQueue(sc_core::sc_module_name name)
  : sc_core::sc_module(name)
{
  SC_THREAD(waiter);
};

// register an event with its next releasetime in the EventQueue
void NodeQueue::registerNode(NodeBase* node, sc_core::sc_time time) {
  if (time < sc_core::sc_time_stamp()) {
    std::cerr << "Warning: re-activation of a time-triggered NodeBase with a release-time in the past! ("
              << node->name() << ") "<< time << " < " << sc_core::sc_time_stamp() << std::endl
              << "         Maybe the real execution-time was larger then the period or exceeds the deadline?" << std::endl
              << "         time-triggered activation will be moved to the next periodic point of time in the future" << std::endl;
    smoc_periodic_actor *p_actor = dynamic_cast<smoc_periodic_actor *>(node);
    if (!p_actor) {
      std::cerr << "only a smoc_periodic_actor can determine it's next execution-time itself" << std::endl;
      assert(0);
    }
  }
  TimeNodePair tnp(time, node);
  pqueue.push(tnp);
  //is the new node earlier to release then the current node? or is there currently no node aktiv? -> reactivate the waiter
  if ((current!=nullptr && time < current->time) ||
       current==nullptr) {
    node_added.notify();
  }
}

NodeBase *NodeQueue::getNextNode() {
  TimeNodePair    pair     = pqueue.top();
  NodeBase *top_node = pair.node;
  pqueue.pop();
  
  if (pqueue.empty() || pqueue.top().time > sc_core::sc_time_stamp()) {
    smoc::smoc_reset(*this);
    nodes_processed.notify();
  }
  
  return top_node;
}

void NodeQueue::waiter() {
  while (true) {
    if (!pqueue.empty()) {
      current=boost::shared_ptr<TimeNodePair>(new TimeNodePair(pqueue.top()));
      sc_core::sc_time toWait=current->time-sc_core::sc_time_stamp();
      // if not, something very strange happened
      assert(toWait >= sc_core::sc_time(0,sc_core::SC_NS));
      wait(toWait, node_added);
      //node_added.cancel();
      if(current->time == sc_core::sc_time_stamp()){
        //NodeQueue is an Event, so let's notify itself. After that, the graph-scheduler knows that some periodic tasks could be executed
        this->notify();
        //wait until all nodes of this step of time are activated
        wait(nodes_processed);
        //nodes_processed.cancel();
      }
    } else {
      //no node registered in the queue, so wait for a new one
      current.reset();
      wait(node_added);
      //node_added.cancel();
    }
  }
}

} } // namespace smoc::Detail
