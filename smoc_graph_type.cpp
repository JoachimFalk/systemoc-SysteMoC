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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/smoc_graph_synth.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/smoc_firing_rules.hpp>

using namespace SystemCoDesigner::SGX;
using namespace SysteMoC::NGXSync;

smoc_graph_base::smoc_graph_base(
    sc_module_name name, smoc_firing_state& init, bool regObj) :
  smoc_root_node(name, init, regObj)
{
  // FIXME (multiple ids for the same object!!)
  if(regObj) idPool.regObj(this, 1);
}

const smoc_node_list& smoc_graph_base::getNodes() const
  { return nodes; } 

void smoc_graph_base::getNodesRecursive( smoc_node_list & subnodes) const {
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    //std::cerr << (*iter)->name() << std::endl;
    
    smoc_root_node *node = dynamic_cast<smoc_actor *>(*iter);
    if (node != NULL){
      //std::cerr << "add: " <<  node->name() << std::endl;
      subnodes.push_back(node);
    }
    smoc_graph_base *graph = dynamic_cast<smoc_graph_base *>(*iter);
    if (graph != NULL){
      //std::cerr << "sub_graph: " <<  graph->name() << std::endl;
      graph->getNodesRecursive(subnodes);
    }
  }
  //  return subnodes;
}
 
const smoc_chan_list& smoc_graph_base::getChans() const
  { return channels; }

void smoc_graph_base::getChansRecursive( smoc_chan_list & channels) const {

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

    smoc_graph_base *graph = dynamic_cast<smoc_graph_base *>(*iter);
    if (graph != NULL ){
      graph->getChansRecursive( channels );
    }
  }
}

void smoc_graph_base::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph_base::finalise() begin, name == " << name() << std::endl;
#endif
  NgId idGraph = idPool.getId(this, 1);

  // SystemC --> SGX
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

  // SGX --> SystemC
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
      const smoc_sysc_port_list& nsps = (*iter)->getPorts();
      
      for ( smoc_sysc_port_list::const_iterator ps_iter = nsps.begin();
            ps_iter != nsps.end();
            ++ps_iter ) {
        if ( (*ps_iter)->getParentPort() != NULL ) {
          pgw << "<portmapping "
              << "from=\"" << idPool.printId(*ps_iter) << "\" "
              << "to=\"" << idPool.printId((*ps_iter)->getParentPort()) << "\" "
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
      ++nIter) {
    // add transitions to list
    (*nIter)->addCurOutTransitions(ol);
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
  assert(ol || !"WTF?! smoc_graph::schedule() called but no enabled transition!");
  ExpandedTransition &transition = ol.getEventTrigger();
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
      transition.execute();
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
  smoc_graph_base(name, init, true)
{
  this->constructor();
}

smoc_graph_sr::smoc_graph_sr() :
  smoc_graph_base(sc_gen_unique_name("smoc_graph_sr"), init, true)
{
  this->constructor();
}

void smoc_graph_sr::constructor() {

  init =
    CALL(smoc_graph_sr::action) >> stop;
}

void smoc_graph_sr::action() {
  this->scheduleSR(this);
}



#ifdef SYSTEMOC_DEBUG
# define DEBUG_CODE(code) code
#else
# define DEBUG_CODE(code) do {} while(0);
#endif

void smoc_graph_sr::scheduleSR(smoc_graph_base *c) {
  std::map<smoc_root_node*, size_t> definedInputs;
  //  std::map<smoc_root_node*, size_t> definedOutputs;

  smoc_transition_ready_list bottom;    // starting point for each instant
  smoc_transition_ready_list nonStrict; // only partial known non-strict actor, possibly must be executed several times
  smoc_transition_ready_list nonStrictReleased; // only partial known non-strict actor, possibly must be executed several times
  //FIXME(MS): This list is possibly not needed!
  smoc_transition_ready_list defined;   // no more changes in this instant
  bool nonStrictStable = false;

#ifdef SYSTEMOC_ENABLE_VPC
  // Needed for VPC coupling??
  smoc_transition_ready_list inCommState; 
#endif // SYSTEMOC_ENABLE_VPC

  smoc_node_list nodes;
  this->getNodesRecursive(nodes);

  smoc_chan_list cs;
  c->getChansRecursive( cs );

  //paranoia
  bottom.clear();
  nonStrict.clear();
  defined.clear();

  { // initialize transition lists
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter ) {
      smoc_root_node &n = **iter;
      if( n.isNonStrict() ){
        definedInputs[&n]  = countDefinedInports(n);
        //  definedOutputs[&n] = countDefinedOutports(n);
      
        smoc_sysc_port_list ps  = (*iter)->getPorts();
        for(smoc_sysc_port_list::iterator iter = ps.begin();
            iter != ps.end(); iter++){
          sc_interface *iface = (*iter)->get_interface();
          if( !(*iter)->isInput() ) {

            smoc_multicast_entry_base* mc_sig = dynamic_cast<
              smoc_multicast_entry_base*>(iface);

            if(NULL != mc_sig){
              mc_sig->multipleWriteSameValue(true);
            }

            assert( NULL != mc_sig );
            //      cerr << " (out port) " << iface << endl;;
          }else if( (*iter)->isInput() ){
            smoc_multicast_outlet_base* mc_sig =
              dynamic_cast<smoc_multicast_outlet_base*>(iface);

            if(NULL != mc_sig){
              mc_sig->allowUndefinedRead(true);
            }

            assert( NULL != mc_sig );
            //      cerr << " (in port) " << iface << endl;;
          }
        }

        FiringStateImpl *rs = n.getCurrentState();
        for ( ExpandedTransitionList::iterator titer = rs->getTransitions().begin();
              titer != rs->getTransitions().end();
              ++titer ){
          nonStrict |= *titer;
        }
      }else{
        FiringStateImpl *rs = n.getCurrentState();
        for ( ExpandedTransitionList::iterator titer = rs->getTransitions().begin();
              titer != rs->getTransitions().end();
              ++titer ){
          bottom |= *titer;
        }
      }
    }
  }
  
  do {
    do{
#ifdef SYSTEMOC_ENABLE_VPC
      DEBUG_CODE(
        if(inCommState) cerr << " inCommState " << inCommState.size();
        else if(inCommState.size()) cerr << " (inCommState) "
                                         << inCommState.size();
        else            cerr << " ------------- " ;
      )
#endif // SYSTEMOC_ENABLE_VPC
      DEBUG_CODE(
        if(bottom)     cerr << " bottom " << bottom.size();
        else if(bottom.size()) cerr << " (bottom) " << bottom.size();
        else           cerr << " -------- " ;

        if(nonStrict)  cerr << " nonStrict " << nonStrict.size();
        else if(nonStrict.size()) cerr << " (nonStrict) " << nonStrict.size();
        else           cerr << " ----------- " ;

        if(nonStrictReleased) cerr << " nonStrictReleased "
                                  << nonStrictReleased.size();
        else if(nonStrictReleased.size()) cerr << " (nonStrictReleased) "
                                              << nonStrictReleased.size();
        else                  cerr << " ------------------- " ;

        if(defined)             cerr << "  defined: " << defined.size();
        else if(defined.size()) cerr << " (defined) " << defined.size();
        else                    cerr << " ------------ " ;

        cerr << endl;
      )
      //Select one of the transition lists by priority
      // (inCommState, bottom, nonStrict)
#ifdef SYSTEMOC_ENABLE_VPC
      if( inCommState ){
        // release all ready actors in commstate:
        // data production hopefully making signals become "more defined"

        ExpandedTransition 
          &transition = inCommState.getEventTrigger();

        Expr::Detail::ActivationStatus      status = transition.getStatus();
        smoc_root_node                   &n = transition.getActor();
        //FIXME(MS): rename n -> node, block..

        assert( status.toSymbol() == Expr::Detail::_ENABLED
                /*|| status.toSymbol() == Expr::Detail::_DISABLED */);

        // remove nodes transitions from list
        for ( ExpandedTransitionList::iterator titer = n.getCurrentState()->getTransitions().begin();
              titer != n.getCurrentState()->getTransitions().end();
              ++titer ){
          defined.remove(*titer);
          inCommState.remove(*titer);
        }

        DEBUG_CODE( std::cerr << "<actor type=\"commstate\" name=\""
                              << n.name() << "\">" << std::endl; )
        transition.execute();
        DEBUG_CODE( std::cerr << "</actor>" << std::endl; )
    
        // move transition to next list
        if(n.isNonStrict()){
          // reset state (state changes on "tick" only)
          n.setCurrentState(n.getLastState());

          if( 0 == countDefinedInports(n) ){
            //nothing changed: allow concurrent transitions to be executed
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              if( !nonStrictReleased.contains(*titer) // allready tested and deactivated
                  &&  (*titer != *n.getLastTransition()) )  // actually tested
                nonStrict |= *titer;
            }
          } else { // some output became defined -> remove all concurent transitions
            // remove nodes transitions from list
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              nonStrict.remove(*titer);
              defined.remove(*titer);
              //paranoia:
              nonStrictReleased.remove(*titer);
            }
          }
          nonStrictReleased |= *(n.getLastTransition()); // deactivate transition, until input changes
          assert( definedInputs.find(&n) != definedInputs.end() ); //if(!...) definedInputs[&n] = countDefinedInports(n);
        }else{ // all those strict actors can only be executed once per instant
          for ( ExpandedTransitionList::iterator titer
                  = n.getCurrentState()->getTransitions().begin();
                titer != n.getCurrentState()->getTransitions().end();
                ++titer ){
            defined           |= *titer;
          }
        }

      }else  // yet another spooky ifdef: "else if"
#endif // SYSTEMOC_ENABLE_VPC
        if( bottom ){
          // bottom contains strict blocks, releasing them hopefully causes data production

          ExpandedTransition
            &transition = bottom.getEventTrigger();
          Expr::Detail::ActivationStatus  status = transition.getStatus();
          smoc_root_node                      &n = transition.getActor();
          //FIXME(MS): rename n -> node, block..
          assert( (status.toSymbol() == Expr::Detail::_ENABLED)
                  || (status.toSymbol() == Expr::Detail::_DISABLED) );
          if( status.toSymbol() == Expr::Detail::_DISABLED ){
            // do not treat disabled transitions
            bottom.remove(transition);
            defined |= transition;
          }else{

            assert( !n.isNonStrict() );
            assert( !n.inCommState() );
            // remove nodes transitions from list
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              defined.remove(*titer);
              bottom.remove(*titer);
            }
            DEBUG_CODE( std::cerr << "<actor type=\"bottom\" name=\""
                                  << n.name() << "\">" << std::endl; )
            transition.execute();
            DEBUG_CODE( std::cerr << "</actor>" << std::endl; )

            // move transition to next list
#ifdef SYSTEMOC_ENABLE_VPC
            assert(n.inCommState());
            n.setLastTransition(&transition);
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              inCommState |= *titer; // treat communication delay
            }
#else
            assert( !n.isNonStrict() );
            assert( !n.inCommState() );
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              defined           |= *titer; 
            }
#endif  // SYSTEMOC_ENABLE_VPC
          }
  
        }else if( nonStrict ){
  // repeated releasing non strict blocks until no more signals became "more defined"
  // to allow caotic iteration all released blocks are moved into another list "nonStrictReleased"
  // if their inputs became "more defined" they will be moved back to list "nonStrict"

         ExpandedTransition
           &transition = nonStrict.getEventTrigger();
          Expr::Detail::ActivationStatus status = transition.getStatus();
          smoc_root_node                     &n = transition.getActor();
          //FIXME(MS): rename n -> node, block..

          assert( status.toSymbol() == Expr::Detail::_ENABLED
                  || status.toSymbol() == Expr::Detail::_DISABLED );
          // do not execute disabled transitions
          if( status.toSymbol() == Expr::Detail::_DISABLED ){
            nonStrict.remove(transition);
            nonStrictReleased |= transition;
          }else{
            // remove transitions from list
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              defined.remove(*titer);
              nonStrict.remove(*titer);
            }
    
            // test transition by execution
            n.setLastState(n.getCurrentState());
#ifdef SYSTEMOC_ENABLE_VPC
            n.setLastTransition(&transition);
#endif
            DEBUG_CODE( std::cerr << "<actor type=\"non strict\" name=\""
                                  << n.name() << "\">" << std::endl; )
            transition.execute(ExpandedTransition::GO);
            DEBUG_CODE( std::cerr << "</actor>" << std::endl; )

    // move transition to next list
#ifdef SYSTEMOC_ENABLE_VPC
            assert(n.inCommState());
            for ( ExpandedTransitionList::iterator titer
                    = n.getCurrentState()->getTransitions().begin();
                  titer != n.getCurrentState()->getTransitions().end();
                  ++titer ){
              inCommState |= *titer; // treat communication delay
            }
#else
            assert( n.isNonStrict() );
            n.setCurrentState(n.getLastState()); // reset state (state changes on "tick" only)
    
            if( 0 == countDefinedOutports(n) ){ //QUICK-FIX
              //nothing changed: allow concurrent transitions to be executed
              for ( ExpandedTransitionList::iterator titer
                      = n.getCurrentState()->getTransitions().begin();
                    titer != n.getCurrentState()->getTransitions().end();
                    ++titer ){
                if( !nonStrictReleased.contains(*titer) ) // allready tested and deactivated
                  nonStrict |= *titer;
              }
            } else {
              // some output became defined -> remove all concurent transitions
              // remove nodes transitions from list
              for ( ExpandedTransitionList::iterator titer
                      = n.getCurrentState()->getTransitions().begin();
                    titer != n.getCurrentState()->getTransitions().end();
                    ++titer ){
                nonStrict.remove(*titer);
                defined.remove(*titer);
                //paranoia:
                nonStrictReleased.remove(*titer);
              }
            }

            // deactivate transition, until input changes
            nonStrictReleased |= transition;
            // definedInputs[n] = countDefinedInports(n);
            assert( definedInputs.find(&n) != definedInputs.end() );
#endif  // SYSTEMOC_ENABLE_VPC

          }
          
        }else if(nonStrictReleased/* && inCommState.empty() && nonStrict.empty()*/){
  // all commstates treated and no other block is ready

  // lazy iterating all ready blocks in nonStrictReleased
  // if any has "more defined" inputs than reexecute it by moving into nonStrict list
          smoc_transition_ready_list temp;
          nonStrictStable = true; // also stable if nonStrictReleased.empty()
          while(nonStrictReleased){
            // identify ready blocks
            ExpandedTransition
              &transition = nonStrictReleased.getEventTrigger();
            smoc_root_node            &n = transition.getActor();
            //FIXME(MS): rename n -> node, block..
            assert(n.isNonStrict());
            if( Expr::Detail::_ENABLED == transition.getStatus().toSymbol()){
    
              size_t actualDefined = countDefinedInports(n);

              //compare number of defined input signals with previous run
              assert( definedInputs.find(&n) != definedInputs.end() );
              if(actualDefined > definedInputs[&n]){ // "more defined" inputs
                definedInputs[&n] = actualDefined;
                // again evaluate non strict blocks
                nonStrict |=  transition;
                nonStrictStable = false;
              }else{
                temp      |=  transition;
              }
            }else{
              assert( Expr::Detail::_DISABLED
                      == transition.getStatus().toSymbol());
              temp      |=  transition;
            }
            nonStrictReleased.remove(transition);
          }

          //redo temporary modifications from lazy iteration
          while(temp){
            ExpandedTransition
              &transition = temp.getEventTrigger();
            nonStrictReleased |= transition;
            temp.remove(transition);
          }
          assert(temp.empty());

          // test if fixpoint is reached
#ifdef SYSTEMOC_ENABLE_VPC
          if( nonStrictStable && inCommState.empty() ){
            //paranoia:
            assert(inCommState.empty());
#else
          if( nonStrictStable ){
#endif
            assert(!bottom);
            assert(nonStrict.empty());

            //cout << "FIXED POINT " << sc_time_stamp() << endl;
            //fixpoint reached

            //tick all ns transitions
            while(nonStrictReleased){
              // identify ready blocks
              ExpandedTransition &transition
                = nonStrictReleased.getEventTrigger();
              smoc_root_node         &n = transition.getActor();
              //FIXME(MS): rename n -> node, block..
              assert(n.isNonStrict());
              if( Expr::Detail::_ENABLED
                  == transition.getStatus().toSymbol()){
    
                size_t actualDefined  = countDefinedInports(n);
                actualDefined        += countDefinedOutports(n);
                if(actualDefined > 0){ // some outputs or inputs are defined

                  //FIXME(MS): assume not to call compute (no commState)
                  transition.execute(ExpandedTransition::TICK);
                  for ( ExpandedTransitionList::iterator titer
                          = n.getCurrentState()->getTransitions().begin();
                        titer != n.getCurrentState()->getTransitions().end();
                        ++titer ){
#ifdef SYSTEMOC_ENABLE_VPC
                    inCommState |= *titer;
#else
                    nonStrict |= *titer;
#endif
                  }
                }else{
                  nonStrict |= transition;
                }
              }else{
                assert( Expr::Detail::_DISABLED
                        == transition.getStatus().toSymbol());
                nonStrict |= transition;
              }
              nonStrictReleased.remove(transition);
            }

#ifdef SYSTEMOC_ENABLE_VPC
            while( !inCommState.empty() ){
              smoc_wait( inCommState );
              ExpandedTransition
                &transition = inCommState.getEventTrigger();
              Expr::Detail::ActivationStatus   status = transition.getStatus();
              smoc_root_node                   &n = transition.getActor();
              assert( status.toSymbol() == Expr::Detail::_ENABLED );
                      
              // remove nodes transitions from list
              for ( ExpandedTransitionList::iterator titer
                      = n.getCurrentState()->getTransitions().begin();
                    titer != n.getCurrentState()->getTransitions().end();
                    ++titer ){
                inCommState.remove(*titer);
              }

              DEBUG_CODE( std::cerr << "<actor type=\"commstate\" name=\""
                          << n.name() << "\">" << std::endl; )
              transition.execute(); //
              DEBUG_CODE( std::cerr << "</actor>" << std::endl; )
              for ( ExpandedTransitionList::iterator titer
                      = n.getCurrentState()->getTransitions().begin();
                    titer != n.getCurrentState()->getTransitions().end();
                    ++titer ){
                nonStrict |= *titer;
              }
            }
            assert(inCommState.empty());
#endif
            //cout << "FIXED POINT END " << sc_time_stamp() << endl;

            assert(nonStrictReleased.empty());

            //tick all signals
            for ( smoc_chan_list::const_iterator iter = cs.begin();
                  iter != cs.end();
                  ++iter ){
              // "tick()" each signal
              smoc_multicast_sr_signal_chan_base* mc_sig = dynamic_cast<
                smoc_multicast_sr_signal_chan_base* >(*iter);

              if(NULL != mc_sig){
                mc_sig->tick();
              }

              //QUICK-FIX
              // reset "sensitivity list"
              for(std::map<smoc_root_node*, size_t>::iterator i =
                    definedInputs.begin();
                  i != definedInputs.end();
                  ++i){
                i->second = 0;
              }

              assert( NULL != mc_sig );
            }

            //??move strict transitions back to bottom??
            bottom |= defined;
            defined.clear();
          }

        }
        if(
#ifdef SYSTEMOC_ENABLE_VPC
                 inCommState.empty() &&
#endif // SYSTEMOC_ENABLE_VPC
                 nonStrictReleased.empty() &&
                 nonStrict.empty() &&
                 !bottom ){
        // another way reaching the fixed point is:
        // having no non strict blocks, no commstate blocks,
        // and bottom may not be ready
        // that means only defined list is not empty

        //move strict transitions back to bottom
          bottom |= defined;
          defined.clear();
          //tick all signals
          for ( smoc_chan_list::const_iterator iter = cs.begin();
                iter != cs.end();
                ++iter ){
            // "tick()" each signal
            smoc_multicast_sr_signal_chan_base* mc_sig = dynamic_cast<
              smoc_multicast_sr_signal_chan_base* >(*iter);
          
            if(NULL != mc_sig){
              mc_sig->tick();
            }
          
            assert( NULL != mc_sig );
          }
  
        }

        if(
#ifdef SYSTEMOC_ENABLE_VPC
           !inCommState &&
#endif
           !bottom && !nonStrict ){
          //cerr << "WAIT" << endl;
          smoc_transition_ready_list all;
#ifdef SYSTEMOC_ENABLE_VPC
          if( !inCommState.empty()       ) all |= inCommState;
#endif
          if( !bottom.empty()            ) all |= bottom;
          if( !nonStrict.empty()         ) all |= nonStrict;
          if( !nonStrictReleased.empty() ) all |= nonStrictReleased;
          if( !all.empty()               ) smoc_wait(all);
        }
  
      }while(1);
    assert(0);
      
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</smoc_scheduler_top::scheduleSR>" << std::endl;
#endif
    smoc_transition_ready_list all;
    all |= bottom;
    all |= defined;
    all |= nonStrict;
    smoc_wait(all);
    //FIXME(MS): wait also for nonStrict list
#ifdef SYSTEMOC_DEBUG
    std::cerr << bottom << std::endl;
#endif
  } while ( 1 );
}

size_t smoc_graph_sr::countDefinedInports(smoc_root_node &n){
  size_t definedInPorts = 0;
  smoc_sysc_port_list ps  = n.getPorts();
  for(smoc_sysc_port_list::iterator iter = ps.begin();
      iter != ps.end(); iter++){
    sc_interface *iface = (*iter)->get_interface();
    if( (*iter)->isInput() ) {
      smoc_multicast_outlet_base* mc_sig = dynamic_cast<
        smoc_multicast_outlet_base*>(iface);

      if(NULL != mc_sig){
  if(mc_sig->isDefined()) definedInPorts++;
      }

      assert( NULL != mc_sig );
    }
  }
  return definedInPorts;
}

size_t smoc_graph_sr::countDefinedOutports(smoc_root_node &n){
  size_t definedOutPorts = 0;
  smoc_sysc_port_list ps  = n.getPorts();
  for(smoc_sysc_port_list::iterator iter = ps.begin();
      iter != ps.end(); iter++){
    sc_interface *iface = (*iter)->get_interface();
   if( !(*iter)->isInput() ) {
      smoc_multicast_entry_base* mc_sig = dynamic_cast<
        smoc_multicast_entry_base*>(iface);

      if(NULL != mc_sig){
  if(mc_sig->isDefined()) definedOutPorts++;
      }

      assert( NULL != mc_sig );

    }
  }
  return definedOutPorts;
}
