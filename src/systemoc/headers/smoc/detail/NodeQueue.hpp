// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_NODEQUEUE_HPP
#define _INCLUDED_SMOC_DETAIL_NODEQUEUE_HPP

#include <smoc/detail/NodeBase.hpp>

#include "../smoc_event.hpp"

#include <systemc>

#include <boost/shared_ptr.hpp>

#include <queue>


namespace smoc { namespace Detail {

class NodeQueue
  : public sc_core::sc_module,
    public smoc::smoc_event
{
  SC_HAS_PROCESS(NodeQueue);
public:
  NodeQueue(sc_core::sc_module_name name);
  
  // register an event with its next releasetime in the EventQueue
  void registerNode(NodeBase *node, sc_core::sc_time time);
  
  NodeBase *getNextNode();
private:

  /* struct used to store an event with a certain release-time */
  struct TimeNodePair {
    TimeNodePair(sc_core::sc_time time, NodeBase *node)
      : time(time), node(node) {}

    sc_core::sc_time  time;
    NodeBase   *node;
  };

  /* struct used for comparison
   * needed by the priority_queue */
  struct NodeCompare {
    bool operator()(const TimeNodePair &tnp1,
                    const TimeNodePair &tnp2) const
      { return tnp1.time > tnp2.time; }
  };

  typedef std::priority_queue<TimeNodePair, std::vector<TimeNodePair>, NodeCompare>  TimedQueue;

  boost::shared_ptr<TimeNodePair> current;
  sc_core::sc_event               node_added;
  sc_core::sc_event               nodes_processed;
  TimedQueue                      pqueue;

  //SystemC-process, it tops the queue and waits the specific amount of time
  void waiter();

};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_NODEQUEUE_HPP */
