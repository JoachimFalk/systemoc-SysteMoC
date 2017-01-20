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

#ifndef __INCLUDED__SMOC_GRAPH__TT__HPP__
#define __INCLUDED__SMOC_GRAPH__TT__HPP__


#include <CoSupport/compatibility-glue/nullptr.h>

#include <boost/smart_ptr.hpp>

#include "../smoc/detail/GraphBase.hpp"
#include "../smoc/detail/NodeQueue.hpp"

/**
 * TimeTriggered graph with FSM which schedules children by selecting
 * scheduling is done timetriggered-parameters (offset, period)
 */
class smoc_graph_tt : public smoc::Detail::GraphBase {
public:
  // construct graph with name
  explicit smoc_graph_tt(const sc_core::sc_module_name& name);

  // construct graph with generated name
  smoc_graph_tt();
  
  /**
   * disables the executability of an actor
   */
  void disableActor(std::string actor_name);

  /**
   * reenables the executability of an actor
   */
  void reEnableActor(std::string actor_name);

protected:
  /// @brief See GraphBase
  virtual void before_end_of_elaboration();

private:
  
  // common constructor code
  void constructor();

  void initTT();

  // schedule children of this graph
  void scheduleTT();

  // a list containing the transitions of the graph's children
  // that may be executed
  typedef CoSupport::SystemC::EventOrList<smoc::smoc_event>
          smoc_node_ready_list;
  typedef CoSupport::SystemC::EventOrList<smoc_node_ready_list>
          smoc_nodes_ready_list;

  // OrList for the dataflow-driven nodes
  smoc_node_ready_list ddf_nodes_activations;
  // OrList for the activation of the graph
  smoc::smoc_event_or_list graph_activation; // nodes scheduleable?

  // handling of the time-triggered nodes
  smoc::Detail::NodeQueue ttNodeQueue;

  // graph scheduler FSM state
  smoc_firing_state run;

  std::map<std::string, smoc_root_node*> nameToNode;
  std::map<smoc_root_node*, bool> nodeDisabled;
};

#endif //__INCLUDED__SMOC_GRAPH__TT__HPP__
