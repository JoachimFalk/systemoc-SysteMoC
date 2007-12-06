//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_GRAPH_SYNTH_HPP
#define _INCLUDED_SMOC_GRAPH_SYNTH_HPP

#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

namespace SysteMoC {

// pair Commit.Available
typedef std::pair<size_t, size_t> PortReq;

// map port -> comm/available
typedef std::map<NGX::Port::ConstPtr, PortReq> PortReqMap;

class smoc_graph_synth : public smoc_graph_base
{
public:
  smoc_graph_synth(NGX::ProblemGraph::ConstRef pg);
private:  
  // associated problemgraph
  NGX::ProblemGraph::ConstRef pg;
  
  // start state of FSM (other states are temporaries)
  smoc_firing_state init;

  // Expression representing a size_t literal
  typedef Expr::DLiteral<size_t> ESizeT;

  // Expression representing a port requirement without
  // commit (can't steal tokens from fifo -> actor needs
  // them!)
  typedef Expr::BinOp<
    Expr::DPortTokens<smoc_root_port>,
    ESizeT,
    Expr::DOpBinGe>::type EPortGuard;

  // Expression representing any other expression
  typedef Expr::Ex<bool>::type EVariant;

  // recursively constructs a guard expression for multiple
  // port requirements
  EVariant portGuard(
      PortReqMap::const_iterator i,
      PortReqMap::const_iterator e);

  // convenience method for constructing a guard expression from
  // collected port requirements
  EVariant portGuard(const PortReqMap& pr);

  // port requirements of single actor for single phase
  typedef std::map<smoc_root_port*, size_t> Phase;
  
  // port requirements of single actor for all phases
  typedef std::vector<Phase> Phases;

  // cache single phase for single actor
  void cachePhase(NGX::PortRefList::ConstRef ports, Phase& phase);

  // cache all phases for single actor
  void cachePhases(NGX::Actor::ConstPtr actor, Phases& phases);

  struct NodeInfo {
    // cached port requirements for phases
    Phases phases;
    // number of node activations
    size_t count;
    // outgoing transitions
    smoc_transition_ready_list trans;
  };

  // infos for cluster nodes
  typedef std::map<smoc_root_node*, NodeInfo> NodeInfoMap;
  NodeInfoMap nodeInfos;

  // current node to be executed
  NodeInfoMap::iterator curNode;

  // faster switching of active node
  CoSupport::SystemC::VariantEventWaiter transList;

  // schedule loop stack entry
  struct SLStackEntry {
    NGX::ScheduleLoopItemList::ConstRef sl;
    NGX::ScheduleLoopItemList::const_iterator iter;
    size_t count;
    SLStackEntry(NGX::ScheduleLoopItemList::ConstRef sl, size_t count);
  };

  // schedule loop stack
  typedef std::list<SLStackEntry> SLStack;
  SLStack slStack;
  
  // FIXME: interface cache (can't use smoc_port_base where 
  // exact type would be already available)
  typedef std::map<smoc_root_port*, smoc_chan_in_base_if*> ChanInMap;
  ChanInMap chanInMap;
  
  // limit token ids for interface
  typedef std::map<smoc_chan_in_base_if*, size_t> ChanInLimit;
  ChanInLimit chanInLimit;

  // determines next actor to be activated
  void nextActorActivation();

  // prepares LoopedSchedule for execution
  void prepareExecute(NGX::Action::ConstPtr a);

  // determines if actor activation left in schedule
  bool activationsLeft() const;

  // checks if actor has reserved tokens / space left
  bool activationComplete() const;

  // executes transition list for current actor
  void executeTransitionList();

  // generates firing fsm for this graph
  void generateFSM();
};

} // namespace SysteMoC

#endif // _INCLUDED_SMOC_GRAPH_SYNTH_HPP