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

#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/hscd_node_types.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/smoc_graph_synth.hpp>

using namespace SysteMoC::NGX;
using namespace SysteMoC::NGXSync;

smoc_graph_base::smoc_graph_base(
    sc_module_name name, smoc_firing_state& init, bool regObj) :
  smoc_root_node(name, init, regObj),
  top(NULL)
{
  // FIXME (multiple ids for the same object!!)
  if(regObj) idPool.regObj(this, 1);
  SC_THREAD(smocCallTop);
}

void smoc_graph_base::smocCallTop() {
  if (top != NULL)
    top->schedule(this);
}

void smoc_graph_base::end_of_elaboration() {
  if (top != NULL)
    top->elabEnd(this);
}

const smoc_node_list& smoc_graph_base::getNodes() const
  { return nodes; } 
 
const smoc_chan_list& smoc_graph_base::getChans() const
  { return channels; }

void smoc_graph_base::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph_base::finalise() begin, name == " << name() << std::endl;
#endif
  NgId idGraph = idPool.getId(this, 1);

  // SystemC --> NGX
  for(std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
      iter != get_child_objects().end();
      ++iter)
  {
    // only processing children which are nodes
    smoc_root_node* node = dynamic_cast<smoc_root_node*>(*iter);
    if(!node) continue;
    
    // determine if node is in XML; otherwise it will be "hidden"
    if(NGXConfig::getInstance().hasNGX()) {  
      
      Process::ConstPtr proc =
        objAs<Process>(NGXCache::getInstance().get(node));

      if(!proc || proc->owner()->id() != idGraph)
        continue;
    }

    nodes.push_back(node);
  }
  
  for(std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
      iter != get_child_objects().end();
      ++iter)
  {
    // only processing children which are channels
    smoc_root_chan* channel = dynamic_cast<smoc_root_chan*>(*iter);
    if(!channel) continue;
    
    channels.push_back(channel);
  }

  // NGX --> SystemC
  if(NGXConfig::getInstance().hasNGX()) {
  
    ProblemGraph::ConstPtr pg =
      objAs<ProblemGraph>(NGXCache::getInstance().get(this, 1));

    if(!pg) {
      // XML node missing or no ProblemGraph
    }
    else {
      ActorList::ConstRef actors = pg->actors();
      for(ActorList::const_iterator aIter = actors.begin();
          aIter != actors.end();
          ++aIter)
      {
        sc_core::sc_object* scA = NGXCache::getInstance().get(*aIter);
        if(!scA) {
          // synthesize actor
          // TODO: as for now, we won't synthesize actors...
        }
        else {
          smoc_root_node* rn = dynamic_cast<smoc_root_node*>(scA);
          if(!rn) {
            // no smoc_root_node -> manipulated id?
          }
          else if(std::find(nodes.begin(), nodes.end(), rn) == nodes.end()) {
            // only add if not already in list (user moved an (existing)
            // actor into another (existing) graph)
            nodes.push_back(rn);
          }
        }
      }
     
      RefinedProcessList::ConstRef rps = pg->refinedProcesses();
      for(RefinedProcessList::const_iterator rpIter = rps.begin();
          rpIter != rps.end();
          ++rpIter)
      {
        // SysteMoC knows no refinements (graph is same object as
        // process, so there should be exactly one refinement)
        if(rpIter->refinements().size() != 1)
          continue;
        ProblemGraph::ConstRef rpPG = rpIter->refinements().front();

        sc_core::sc_object* scRP = NGXCache::getInstance().get(*rpIter);
        if(!scRP) {
          // synthesize graph
          nodes.push_back(new SysteMoC::smoc_graph_synth(rpPG));
        }
        else {
          // process compiled in -> graph should also be compiled in
          // (and be the same object)
          sc_core::sc_object* scPG = NGXCache::getInstance().get(rpPG);
          if(!scPG || scPG != scRP)
            continue;

          smoc_root_node* rn = dynamic_cast<smoc_root_node*>(scRP);
          if(!rn) {
            // no smoc_root_node -> manipulated id?
          }
          else if(std::find(nodes.begin(), nodes.end(), rn) == nodes.end()) {
            // only add if not already in list (user moved an (existing)
            // graph into another (existing) graph)
            nodes.push_back(rn);
          }
        }
      }
    }
  }

  // finalise for actors must precede finalise for channels,
  // because finalise for channels needs the patched in actor
  // references in the ports which are updated by the finalise
  // methods of their respective actors
  for(smoc_node_list::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
  {
    (*iter)->finalise();
  }
  
  for(smoc_chan_list::iterator iter = channels.begin();
      iter != channels.end();
      ++iter)
  {
    (*iter)->finalise();
  }
  
  smoc_root_node::finalise();

#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph_base::finalise() end, name == " << name() << std::endl;
#endif
}

#ifndef __SCFE__

void smoc_graph_base::pgAssemble(
    smoc_modes::PGWriter &pgw,
    const smoc_root_node *n) const
{
  // FIXME: multiple ids for the same object (-> process!)
  pgw << "<problemgraph name=\"" << name() << "_pg\" id=\""
      << idPool.printId(this, 1) << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_chan_list::const_iterator iter = channels.begin();
          iter != channels.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter ) {
      const smoc_port_hixhax_list& nsps = (*iter)->getPorts();
      
      for ( smoc_port_hixhax_list::const_iterator ps_iter = nsps.begin();
            ps_iter != nsps.end();
            ++ps_iter ) {
        if ( (*ps_iter)->outerConnectedPort() != NULL ) {
          pgw << "<portmapping "
              << "from=\"" << idPool.printId(*ps_iter) << "\" "
              << "to=\"" << idPool.printId((*ps_iter)->outerConnectedPort()) << "\" "
              << "id=\"" << idPool.printId() << "\"/>" << std::endl;
        }
      }
    }
    pgw.indentDown();
  }
  pgw << "</problemgraph>" << std::endl;
}

void smoc_graph_base::assembleActor(smoc_modes::PGWriter &pgw) const
{}

#endif // __SCFE__

smoc_graph::smoc_graph(sc_module_name name) :
  smoc_graph_base(name, init, true)
{
  this->constructor();
}

smoc_graph::smoc_graph() :
  smoc_graph_base(sc_gen_unique_name("smoc_graph"), init, true)
{
  this->constructor();
}

void smoc_graph::constructor() {
  // collect all transitions that may be executed
  init =
    CALL(smoc_graph::initScheduling)  >> run;

  // if there is at least one active transition: execute it
  run =
    Expr::till(ol)                    >>
    CALL(smoc_graph::schedule)        >> run;
}

void smoc_graph::initScheduling() {
  const smoc_node_list& nodes = getNodes();
#ifdef SYSTEMOC_DEBUG
  std::cerr << "<smoc_graph::initScheduling>" << std::endl;
#endif
  for(smoc_node_list::const_iterator nIter = nodes.begin();
      nIter != nodes.end();
      ++nIter)
  {
    smoc_root_node *node = *nIter;
    // only SysteMoC v2 actors will be scheduled
    if(dynamic_cast<hscd_choice_active_node *>(node))
      continue;
    // add transitions to list
    node->addCurOutTransitions(ol);
  }
#ifdef SYSTEMOC_DEBUG
    std::cerr << ol << std::endl;
    std::cerr << "</smoc_graph::initScheduling>" << std::endl;
#endif
}

void smoc_graph::schedule() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "<smoc_graph::schedule>" << std::endl;
#endif
  smoc_firing_types::transition_ty &transition = ol.getEventTrigger();
  Expr::Detail::ActivationStatus status = transition.getStatus();
      
  switch(status.toSymbol()) {
    case Expr::Detail::_DISABLED:
      ol.remove(transition);
      break;
    case Expr::Detail::_ENABLED: {
      smoc_root_node &n = transition.getActor();
#ifdef SYSTEMOC_DEBUG
      std::cerr << "<node name=\"" << n.name() << "\">" << std::endl;
#endif
      // remove transitions from list
      n.delCurOutTransitions(ol);
      // execute transition
      transition.execute(&n._currentState, &n);
      // add transitions to list
      n.addCurOutTransitions(ol);
#ifdef SYSTEMOC_DEBUG
      std::cerr << "</node>" << std::endl;
#endif
      break;
    }
    default:
      assert(0);
  }
#ifdef SYSTEMOC_DEBUG
    std::cerr << ol << std::endl;
    std::cerr << "</smoc_graph::schedule>" << std::endl;
#endif
}

smoc_graph_sr::smoc_graph_sr(sc_module_name name) :
  smoc_graph(name)
{}
  
smoc_graph_sr::smoc_graph_sr() :
  smoc_graph()
{}

void smoc_graph_sr::smocCallTop() {
  if (top != NULL)
    top->scheduleSR(this);
}
