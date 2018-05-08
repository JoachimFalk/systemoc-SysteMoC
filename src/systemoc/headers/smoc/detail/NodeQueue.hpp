// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SMOC_DETAIL_NODEQUEUE_HPP
#define _INCLUDED_SMOC_DETAIL_NODEQUEUE_HPP

#include <smoc/detail/NodeBase.hpp>

#include <systemc>
#include <queue>

#include "../smoc_event.hpp"

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
