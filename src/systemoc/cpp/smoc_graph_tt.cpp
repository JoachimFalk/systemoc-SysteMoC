//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/smoc_tt.hpp>
#include <smoc/detail/DebugOStream.hpp>

smoc_graph_tt::smoc_graph_tt(const sc_core::sc_module_name& name) :
  smoc_graph_base(name, run),
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  ttNodeQueue("__smoc_ttEventQueue"),
  run("run")
{
  constructor();
}

smoc_graph_tt::smoc_graph_tt() :
  smoc_graph_base(sc_core::sc_gen_unique_name("smoc_graph_tt"), run),
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  ttNodeQueue("__smoc_ttNodeQueue"),
  run("run")
{
  constructor();
}
  
void smoc_graph_tt::before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_graph::before_end_of_elaboration name=\"" << name() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
  
  smoc_graph_base::before_end_of_elaboration();
  initTT();

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_graph::before_end_of_elaboration>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void smoc_graph_tt::constructor() {
  // if there is at least one active transition: execute it
  graph_activation |= ddf_nodes_activations;
  graph_activation |= ttNodeQueue;
  run = smoc::Expr::till(graph_activation) >> SMOC_CALL(smoc_graph_tt::scheduleTT) >> run;
}

void smoc_graph_tt::initTT() {
  for(smoc_node_list::const_iterator iter = getNodes().begin();
      iter != getNodes().end(); ++iter)
  {
    nameToNode[(*iter)->name()] = *iter;
    smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( (*iter) );
    if(entry){
      ttNodeQueue.registerNode(entry, entry->getOffset());
    }else{
      //nodes of other types then smoc_periodic_actor will be added to ddf_nodes_activations
      //could be another graph or other nodes
      ddf_nodes_activations |= **iter;
    }
  }
}

void smoc_graph_tt::scheduleTT() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_graph_tt::scheduleTT name=\"" << name() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
  while(ddf_nodes_activations){
    //schedule the "normal" Tasks (DDF)
    smoc_root_node& n = dynamic_cast<smoc_root_node&>( ddf_nodes_activations.getEventTrigger());
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "<node name=\"" << n.name() << "\">" << std::endl
             << smoc::Detail::Indent::Up;
    }
#endif // SYSTEMOC_DEBUG
    n.schedule();
    smoc_periodic_actor *p_node = dynamic_cast<smoc_periodic_actor *>( &n);
    if(p_node){ // it is a TT-Node
      //remove it from ddf_nodes_activations and re-register it as a tt-node
      ddf_nodes_activations.remove(n);
      ttNodeQueue.registerNode(p_node, p_node->getNextReleaseTime());
    }
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</node>" << std::endl;
    }
#endif // SYSTEMOC_DEBUG
  }
  while(ttNodeQueue){ // TT-Scheduled
    smoc_root_node* next = ttNodeQueue.getNextNode();
    smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( next);
    assert(entry);
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "<node name=\"" << next->name() << "\">" << std::endl
             << smoc::Detail::Indent::Up;
    }
#endif // SYSTEMOC_DEBUG
    if(nodeDisabled[entry] == false){
      entry->schedule();
#ifdef SYSTEMOC_ENABLE_VPC
      if(entry->inCommState()){ // Node needs some time to process (VPC is used), switch node to DDF
        ddf_nodes_activations |= *next;
      }else{ // Node completely processed -> re-register it in the ttNodeQueue
#endif //SYSTEMOC_ENABLE_VPC
        ttNodeQueue.registerNode(entry, entry->getNextReleaseTime());
#ifdef SYSTEMOC_ENABLE_VPC
      }
#endif //SYSTEMOC_ENABLE_VPC
    }
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</node>" << std::endl;
    }
#endif // SYSTEMOC_DEBUG
  }
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_graph_tt::scheduleTT>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void smoc_graph_tt::disableActor(std::string actor_name){
#ifdef SYSTEMOC_ENABLE_VPC
  if(nameToNode[actor_name] == 0){
      //first run of disableActor (no initialization of graph)
    for(smoc_node_list::const_iterator iter = getNodes().begin();
          iter != getNodes().end(); ++iter)
      {
        nameToNode[(*iter)->name()] = *iter;
      }
  }
#endif //SYSTEMOC_ENABLE_VPC

  assert(nameToNode[actor_name] != 0);
  smoc_root_node* nodeToDisable = nameToNode[actor_name];

#ifdef SYSTEMOC_ENABLE_VPC
  smoc_actor *entry = dynamic_cast<smoc_actor *>( nodeToDisable );
  entry->setActive(false);
#endif //SYSTEMOC_ENABLE_VPC

  if (ddf_nodes_activations.contains(*nodeToDisable)) {
    ddf_nodes_activations.remove(*nodeToDisable);
  } else {
    //so it must be a tt-actor
    //ttNodeQueue.disableNode(nodeToDisable);
    nodeDisabled[nodeToDisable] = true;
  }

}

void smoc_graph_tt::reEnableActor(std::string actor_name) {
  assert(nameToNode[actor_name] != 0);
  smoc_root_node* nodeToEnable = nameToNode[actor_name];
#ifdef SYSTEMOC_ENABLE_VPC
  smoc_actor *actor = dynamic_cast<smoc_actor *>( nodeToEnable );
  actor->setActive(true);
#endif //SYSTEMOC_ENABLE_VPC
  smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( nodeToEnable );
  if (entry) {
    nodeDisabled[entry] = false;
    ttNodeQueue.registerNode(entry, entry->getNextReleaseTime());
  } else {
    //nodes of other types then smoc_periodic_actor will be added to ddf_nodes_activations
    //could be another graph or other nodes
    ddf_nodes_activations |= *nodeToEnable;
  }
}
