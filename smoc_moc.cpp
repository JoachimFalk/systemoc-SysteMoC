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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_sr_signal.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>

#include <cosupport/oneof.hpp>

#ifdef SYSTEMOC_DEBUG
# define DEBUG_CODE(code) code
#else
# define DEBUG_CODE(code) do {} while(0);
#endif

using namespace CoSupport;

void smoc_scheduler_top::elabEnd(smoc_graph_base *g) {
  g->finalise();
  if ((smoc_modes::dumpFileSMX != NULL) && 
      (!smoc_modes::dumpSMXWithSim)) {
    smoc_modes::dump(*g);
    sc_core::sc_stop();
  }
}

// FIXME: only needed in scheduleSR
// remove after redesign of SR scheduler
void smoc_scheduler_top::getLeafNodes(
    smoc_node_list &nodes, smoc_graph_base *node)
{
  const smoc_node_list& n = node->getNodes();
  
  for ( smoc_node_list::const_iterator iter = n.begin();
        iter != n.end();
        ++iter ) {
    if ( dynamic_cast<smoc_actor *>(*iter) != NULL ) {
      nodes.push_back(*iter);
    }
    if ( dynamic_cast<smoc_graph_base *>(*iter) != NULL ) {
      getLeafNodes(nodes, dynamic_cast<smoc_graph_base *>(*iter));
    }
  }
}

void smoc_scheduler_top::schedule(smoc_graph_base *g) {
  smoc_transition_ready_list ol;
  
  // add outgoing transitions to list
  g->addCurOutTransitions(ol);
  
  while(true) {
    smoc_wait(ol);
    while(ol) {
      smoc_firing_types::transition_ty &transition = ol.getEventTrigger();
      Expr::Detail::ActivationStatus status = transition.getStatus();
    
      switch(status.toSymbol()) {
        case Expr::Detail::_DISABLED:
          // remove disabled transition
          assert(&transition.getActor() == g);
          ol.remove(transition);
          break;
        case Expr::Detail::_ENABLED:
          // execute enabled transition
          assert(&transition.getActor() == g);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<node name=\"" << g->name() << "\">" << std::endl;
#endif
          // remove transitions from list
          g->delCurOutTransitions(ol);
          // execute transition
          transition.execute(&g->_currentState, g);
          // add transitions to list
          g->addCurOutTransitions(ol);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</node>" << std::endl;
#endif
          break;
        default:
          assert(0);
      }
    }
  }
}

void smoc_scheduler_top::scheduleSR(smoc_graph_base *c) {
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
  getLeafNodes(nodes, c);  
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
      
        smoc_port_list ps  = (*iter)->getPorts();
        for(smoc_port_list::iterator iter = ps.begin();
            iter != ps.end(); iter++){
          sc_interface *iface = (*iter)->get_interface();
          if( dynamic_cast<class smoc_root_port_out * >(*iter) ) {

            smoc_entry_kind* mc_sig = dynamic_cast<
              class smoc_entry_kind* >(&(*iface));

            if(NULL != mc_sig){
              mc_sig->multipleWriteSameValue(true);
            }

            assert( NULL != mc_sig );
            //      cerr << " (out port) " << iface << endl;;
          }else if( dynamic_cast<class smoc_root_port_in * >(*iter) ){
            smoc_outlet_kind* mc_sig =
              dynamic_cast<class smoc_outlet_kind* >(&(*iface));

            if(NULL != mc_sig){
              mc_sig->allowUndefinedRead(true);
            }

            assert( NULL != mc_sig );
            //      cerr << " (in port) " << iface << endl;;
          }
        }

        smoc_firing_types::resolved_state_ty *rs = n._currentState;
        for ( transitionlist_ty::iterator titer = rs->tl.begin();
              titer != rs->tl.end();
              ++titer ){
          nonStrict |= *titer;
        }
      }else{
        smoc_firing_types::resolved_state_ty *rs = n._currentState;
        for ( transitionlist_ty::iterator titer = rs->tl.begin();
              titer != rs->tl.end();
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

        transition_ty           &transition = inCommState.getEventTrigger();
        Expr::Detail::ActivationStatus      status = transition.getStatus();
        smoc_root_node                   &n = transition.getActor();
        //FIXME(MS): rename n -> node, block..

        assert( status.toSymbol() == Expr::Detail::_ENABLED
                /*|| status.toSymbol() == Expr::Detail::_DISABLED */);

        // remove nodes transitions from list
        for ( transitionlist_ty::iterator titer = n._currentState->tl.begin();
              titer != n._currentState->tl.end();
              ++titer ){
          defined.remove(*titer);
          inCommState.remove(*titer);
        }

        DEBUG_CODE( std::cerr << "<actor type=\"commstate\" name=\""
                              << n.name() << "\">" << std::endl; )
        transition.execute(&n._currentState, &n);
        DEBUG_CODE( std::cerr << "</actor>" << std::endl; )
    
        // move transition to next list
        if(n.isNonStrict()){
          // reset state (state changes on "tick" only)
          n._currentState = n.lastState;

          if( 0 == countDefinedInports(n) ){
            //nothing changed: allow concurrent transitions to be executed
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              if( !nonStrictReleased.contains(*titer) // allready tested and deactivated
                  &&  (*titer != *n.lastTransition) )  // actually tested
                nonStrict |= *titer;
            }
          } else { // some output became defined -> remove all concurent transitions
            // remove nodes transitions from list
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              nonStrict.remove(*titer);
              defined.remove(*titer);
              //paranoia:
              nonStrictReleased.remove(*titer);
            }
          }
          nonStrictReleased |= *(n.lastTransition); // deactivate transition, until input changes
          assert( definedInputs.find(&n) != definedInputs.end() ); //if(!...) definedInputs[&n] = countDefinedInports(n);
        }else{ // all those strict actors can only be executed once per instant
          for ( transitionlist_ty::iterator titer
                  = n._currentState->tl.begin();
                titer != n._currentState->tl.end();
                ++titer ){
            defined           |= *titer;
          }
        }

      }else  // yet another spooky ifdef: "else if"
#endif // SYSTEMOC_ENABLE_VPC
        if( bottom ){
          // bottom contains strict blocks, releasing them hopefully causes data production

          transition_ty              &transition = bottom.getEventTrigger();
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
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              defined.remove(*titer);
              bottom.remove(*titer);
            }
            DEBUG_CODE( std::cerr << "<actor type=\"bottom\" name=\""
                                  << n.name() << "\">" << std::endl; )
            transition.execute(&n._currentState, &n);
            DEBUG_CODE( std::cerr << "</actor>" << std::endl; )

            // move transition to next list
#ifdef SYSTEMOC_ENABLE_VPC
            assert(n.inCommState());
            n.lastTransition=&transition;
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              inCommState |= *titer; // treat communication delay
            }
#else
            assert( !n.isNonStrict() );
            assert( !n.inCommState() );
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              defined           |= *titer; 
            }
#endif  // SYSTEMOC_ENABLE_VPC
          }
  
        }else if( nonStrict ){
  // repeated releasing non strict blocks until no more signals became "more defined"
  // to allow caotic iteration all released blocks are moved into another list "nonStrictReleased"
  // if their inputs became "more defined" they will be moved back to list "nonStrict"

          transition_ty             &transition = nonStrict.getEventTrigger();
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
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              defined.remove(*titer);
              nonStrict.remove(*titer);
            }
    
            // test transition by execution
            n.lastState      = n._currentState;
#ifdef SYSTEMOC_ENABLE_VPC
            n.lastTransition = &transition;
#endif
            DEBUG_CODE( std::cerr << "<actor type=\"non strict\" name=\""
                                  << n.name() << "\">" << std::endl; )
            transition.execute(&n._currentState, &n, smoc_firing_types::transition_ty::GO);
            DEBUG_CODE( std::cerr << "</actor>" << std::endl; )

    // move transition to next list
#ifdef SYSTEMOC_ENABLE_VPC
            assert(n.inCommState());
            for ( transitionlist_ty::iterator titer
                    = n._currentState->tl.begin();
                  titer != n._currentState->tl.end();
                  ++titer ){
              inCommState |= *titer; // treat communication delay
            }
#else
            assert( n.isNonStrict() );
            n._currentState = n.lastState; // reset state (state changes on "tick" only)
    
            if( 0 == countDefinedOutports(n) ){ //QUICK-FIX
              //nothing changed: allow concurrent transitions to be executed
              for ( transitionlist_ty::iterator titer
                      = n._currentState->tl.begin();
                    titer != n._currentState->tl.end();
                    ++titer ){
                if( !nonStrictReleased.contains(*titer) ) // allready tested and deactivated
                  nonStrict |= *titer;
              }
            } else {
              // some output became defined -> remove all concurent transitions
              // remove nodes transitions from list
              for ( transitionlist_ty::iterator titer
                      = n._currentState->tl.begin();
                    titer != n._currentState->tl.end();
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
            transition_ty    &transition = nonStrictReleased.getEventTrigger();
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
            transition_ty           &transition = temp.getEventTrigger();
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
              transition_ty &transition
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
                  transition.execute(&n._currentState,
                                     &n,
                                     smoc_firing_types::transition_ty::TICK);
                  for ( transitionlist_ty::iterator titer
                          = n._currentState->tl.begin();
                        titer != n._currentState->tl.end();
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
              transition_ty        &transition = inCommState.getEventTrigger();
              Expr::Detail::ActivationStatus   status = transition.getStatus();
              smoc_root_node                   &n = transition.getActor();
              assert( status.toSymbol() == Expr::Detail::_ENABLED );
                      
              // remove nodes transitions from list
              for ( transitionlist_ty::iterator titer
                      = n._currentState->tl.begin();
                    titer != n._currentState->tl.end();
                    ++titer ){
                inCommState.remove(*titer);
              }

              DEBUG_CODE( std::cerr << "<actor type=\"commstate\" name=\""
                          << n.name() << "\">" << std::endl; )
              transition.execute(&n._currentState, &n); //
              DEBUG_CODE( std::cerr << "</actor>" << std::endl; )
              for ( transitionlist_ty::iterator titer
                      = n._currentState->tl.begin();
                    titer != n._currentState->tl.end();
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
              // "tick()" each block
              smoc_multicast_sr_signal_kind* mc_sig = dynamic_cast<
                class smoc_multicast_sr_signal_kind* >((*iter));

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
            // "tick()" each block
            smoc_multicast_sr_signal_kind* mc_sig = dynamic_cast<
              class smoc_multicast_sr_signal_kind* >((*iter));
          
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

size_t smoc_scheduler_top::countDefinedInports(smoc_root_node &n){
  size_t definedInPorts = 0;
  smoc_port_list ps  = n.getPorts();
  for(smoc_port_list::iterator iter = ps.begin();
      iter != ps.end(); iter++){
    sc_interface *iface = (*iter)->get_interface();
    if( dynamic_cast<class smoc_root_port_in * >(*iter)) {
      smoc_outlet_kind* mc_sig = dynamic_cast<
  class smoc_outlet_kind* >(&(*iface));

      if(NULL != mc_sig){
  if(mc_sig->isDefined()) definedInPorts++;
      }

      assert( NULL != mc_sig );
    }
  }
  return definedInPorts;
}

size_t smoc_scheduler_top::countDefinedOutports(smoc_root_node &n){
  size_t definedOutPorts = 0;
  smoc_port_list ps  = n.getPorts();
  for(smoc_port_list::iterator iter = ps.begin();
      iter != ps.end(); iter++){
    sc_interface *iface = (*iter)->get_interface();
    if( dynamic_cast<class smoc_root_port_out * >(*iter)) {
      smoc_entry_kind* mc_sig = dynamic_cast<
  class smoc_entry_kind* >(&(*iface));

      if(NULL != mc_sig){
  if(mc_sig->isDefined()) definedOutPorts++;
      }

      assert( NULL != mc_sig );

    }
  }
  return definedOutPorts;
}

/*
      for ( smoc_node_list::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        if ( !(*iter)->is_v1_actor ) {
          bool canexec;
          smoc_firing_types::resolved_state_ty **rs =
            &(*iter)->_currentState;
          
          // assert( _ctx.ports_setup.empty() );
          do {
            canexec = false;
            
            for ( transitionlist_ty::iterator titer = (*rs)->tl.begin();
                  titer != (*rs)->tl.end() && !canexec;
                  ++titer ) {
              canexec = titer->isEnabled();
              
              if ( canexec ) {
                titer->execute(rs,*iter);
                again = true;
              }
            }
          } while ( canexec );
        }
      }
    } while (again);
    {
      for ( smoc_node_list::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        if ( !(*iter)->is_v1_actor ) {
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<findBlocked for " << (*iter)->name() << ">" << std::endl;
#endif
          (*iter)->_currentState->findBlocked(ol);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</findBlocked>" << std::endl;
#endif
        }
      }
 */

