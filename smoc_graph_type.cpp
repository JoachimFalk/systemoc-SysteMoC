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

const smoc_node_list smoc_graph::getNodes() const {
  smoc_node_list subnodes;
  
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    //std::cerr << (*iter)->name() << std::endl;
    
    smoc_root_node *node = dynamic_cast<smoc_root_node *>(*iter);
    if (node != NULL)
      subnodes.push_back(node);
  }
  return subnodes;
}

const smoc_chan_list smoc_graph::getChans() const {
  smoc_chan_list channels;
  
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_chan *chan = dynamic_cast<smoc_root_chan *>(*iter);
    
    if (chan != NULL )
      channels.push_back(chan);
  }
  return channels;
}

void smoc_graph::getChansRecursive( smoc_chan_list & channels) const {

  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_chan *chan = dynamic_cast<smoc_root_chan *>(*iter);
    
    if (chan != NULL )
      channels.push_back(chan);

    smoc_graph *graph = dynamic_cast<smoc_graph *>(*iter);
    if (graph != NULL ){
      graph->getChansRecursive( channels );
    }
  }
}

void smoc_graph::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph::finalise() begin, name == " << name() << std::endl;
#endif
  // finalise for actors must precede finalise for channels,
  // because finalise for channels needs the patched in actor
  // references in the ports which are updated by the finalise
  // methods of their respective actors
  {
    smoc_node_list nodes = getNodes();
    
    for ( smoc_node_list::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      (*iter)->finalise();
  }
  {
    smoc_chan_list chans = getChans();
    
    for ( smoc_chan_list::iterator iter = chans.begin();
          iter != chans.end();
          ++iter )
      (*iter)->finalise();
  }
  smoc_root_node::finalise();
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph::finalise() end, name == " << name() << std::endl;
#endif
}

//sc_module *smoc_graph::myModule()
//  { return this; }

void smoc_graph::pgAssemble(
    smoc_modes::PGWriter &pgw,
    const smoc_root_node *n) const {
  const sc_module *m = this;
  const smoc_node_list ns  = getNodes();
  const smoc_chan_list cs  = getChans();
  const smoc_port_list ps  = n->getPorts();
  
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_chan_list::const_iterator iter = cs.begin();
          iter != cs.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter ) {
      smoc_port_list nsps = (*iter)->getPorts();
      
      for ( smoc_port_list::const_iterator ps_iter = nsps.begin();
            ps_iter != nsps.end();
            ++ps_iter ) {
        if ( (*ps_iter)->getParentPort() != NULL ) {
          pgw << "<portmapping "
              << "from=\"" << pgw.getId(*ps_iter) << "\" "
              << "to=\"" << pgw.getId((*ps_iter)->getParentPort()) << "\" "
              << "id=\"" << pgw.getId() << "\"/>" << std::endl;
        }
      }
    }
    pgw.indentDown();
  }
  pgw << "</problemgraph>" << std::endl;
}

void smoc_graph::smocCallTop() {
  if (top != NULL)
    top->schedule(this);
}

void smoc_graph::end_of_elaboration() {
  if (top != NULL)
    top->elabEnd(this);
}

void smoc_graph::assembleActor(
    smoc_modes::PGWriter &pgw) const {}

//
void smoc_graph::scheduleDataFlow(){
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<smoc_scheduler_top::schedule>" << std::endl;
#endif
    //while (ol) { // looping is done in FSM
      smoc_firing_types::transition_ty   &transition = ol.getEventTrigger();
      Expr::Detail::ActivationStatus          status = transition.getStatus();
      
      switch (status.toSymbol()) {
        case Expr::Detail::_DISABLED: {
          ol.remove(transition);
          break;
        }
        case Expr::Detail::_ENABLED: {
          smoc_root_node &n = transition.getActor();
          smoc_firing_types::resolved_state_ty *oldState = n._currentState;
          
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<actor name=\"" << n.name() << "\">" << std::endl;
#endif
          transition.execute(&n._currentState, &n);
//        if (oldState != n._currentState) {
            for ( smoc_firing_types::transitionlist_ty::iterator titer
                    = oldState->tl.begin();
                  titer != oldState->tl.end();
                  ++titer )
              ol.remove(*titer);
            for ( smoc_firing_types::transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              ol |= *titer;
            }
//        }
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</actor>" << std::endl;
#endif
          break;
        }
        default: {
          assert(status.toSymbol() == Expr::Detail::_ENABLED ||
                 status.toSymbol() == Expr::Detail::_DISABLED   );
        }
      }
      //}
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</smoc_scheduler_top::schedule>" << std::endl;
#endif
    // smoc_wait(ol); // the graph-FSM waits for "ol" using till
#ifdef SYSTEMOC_DEBUG
    std::cerr << ol << std::endl;
#endif
}

//
void smoc_graph::initDataFlow(){
  smoc_node_list nodes = this->getNodes();
  
  for ( smoc_node_list::const_iterator iter = nodes.begin();
        iter != nodes.end();
        ++iter ) {
    smoc_root_node *node = *iter;
    // Is this a SysteMoV v2 actor?
    if (dynamic_cast<hscd_choice_active_node *>(node) == NULL) {
      // yes
      smoc_firing_types::resolved_state_ty *rs = node->_currentState;
      assert(rs != NULL);
      for ( smoc_firing_types::transitionlist_ty::iterator titer
              = rs->tl.begin();
            titer != rs->tl.end();
            ++titer ){
        ol |= *titer;
      }
    } // else no => is v1 actor!
  }
}

//
smoc_graph::smoc_graph(sc_module_name name)
  : smoc_root_node(name, init),
    top(NULL)
{
  this->constructor();
}

//
smoc_graph::smoc_graph()
  : smoc_root_node(sc_gen_unique_name("smoc_graph"), init),
    top(NULL)
{
  this->constructor();
}

//
void smoc_graph::constructor() {

  // this is the scheduler realised as an FSM
  // either a parent graph or the top_moc executes this FSM

  init = // collect all transitions that may be executed
    CALL(smoc_graph::initDataFlow)       >> schedule;

  schedule = // if there is at least one active transition: execute it
    Expr::till(ol)                       >>
    CALL(smoc_graph::scheduleDataFlow)   >> schedule;

  SC_THREAD(smocCallTop);
}

//
void smoc_graph_sr::smocCallTop() {
  if (top != NULL)
    top->scheduleSR(this);
}

