// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <smoc_moc.hpp>
#include <smoc_sr_signal.hpp>

#include <cosupport/oneof.hpp>

using namespace CoSupport;

void smoc_scheduler_top::getLeafNodes(
    smoc_node_list &nodes, smoc_graph *node) {
  smoc_node_list n = node->getNodes();
  
  for ( smoc_node_list::iterator iter = n.begin();
        iter != n.end();
        ++iter ) {
    if ( dynamic_cast<smoc_actor *>(*iter) != NULL ) {
      nodes.push_back(*iter);
    }
    if ( dynamic_cast<smoc_graph *>(*iter) != NULL ) {
      getLeafNodes(nodes, dynamic_cast<smoc_graph *>(*iter));
    }
  }
}

void smoc_scheduler_top::schedule(smoc_graph *c) {
  //int guard_success = 0;
  //int guard_fail    = 0;
  {
    smoc_node_list nodes;
    
    getLeafNodes(nodes, c);
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter ) {
      smoc_firing_types::resolved_state_ty *rs = (*iter)->_currentState;
      
      for ( transitionlist_ty::iterator titer = rs->tl.begin();
            titer != rs->tl.end();
            ++titer )
        ol |= *titer;
    }
  }
  do {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<smoc_scheduler_top::schedule>" << std::endl;
#endif
    while (ol) {
      transition_ty           &transition = ol.getEventTrigger();
      transition_ty::status_t  status     = transition.getStatus();
      
      switch (status) {
        case transition_ty::DISABLED: {
          ol.remove(transition);
          break;
        }
        case transition_ty::ENABLED: {                                                
          smoc_root_node &n = transition.getActor();
          
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<actor name=\"" << n.myModule()->name() << "\">" << std::endl;
#endif
          for ( transitionlist_ty::iterator titer = n._currentState->tl.begin();
                titer != n._currentState->tl.end();
                ++titer )
            ol.remove(*titer);
          transition.execute(&n._currentState, &n);
          for ( transitionlist_ty::iterator titer = n._currentState->tl.begin();
                titer != n._currentState->tl.end();
                ++titer )
            ol |= *titer;
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</actor>" << std::endl;
#endif
          break;
        }
        default: {
          assert(status == transition_ty::ENABLED ||
                 status == transition_ty::DISABLED   );
        }
      }
    }
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</smoc_scheduler_top::schedule>" << std::endl;
#endif
//  std::cerr << "guard_success: " << guard_success << std::endl;
//  std::cerr << "guard_fail:    " << guard_fail    << std::endl;
    smoc_wait(ol);
#ifdef SYSTEMOC_DEBUG
    std::cerr << ol << std::endl;
#endif
  } while ( 1 );
}

void smoc_scheduler_top::scheduleSR(smoc_graph *c) {
  smoc_transition_ready_list bottom;    // starting point for each instant
  smoc_transition_ready_list nonStrict; // only partial known non-strict actor possibly must be executed several time to fixate them
  //FIXME(MS): This list is possibly not needed!
  smoc_transition_ready_list defined;   // no more changes in this instant

#ifdef ENABLE_SYSTEMC_VPC
  // Needed for VPC coupling??
  smoc_transition_ready_list inCommState; 
#endif // ENABLE_SYSTEMC_VPC

  smoc_node_list nodes;
  getLeafNodes(nodes, c);  
  const smoc_chan_list cs  = c->getChans();

  do {
    bottom.clear();
    nonStrict.clear();
    defined.clear();
    { // initialize bottom list
      for ( smoc_node_list::const_iterator iter = nodes.begin();
	    iter != nodes.end();
	    ++iter ) {
	smoc_firing_types::resolved_state_ty *rs = (*iter)->_currentState;
	for ( transitionlist_ty::iterator titer = rs->tl.begin();
	      titer != rs->tl.end();
	      ++titer ){
	    bottom |= *titer;
	}
      }
    }
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<smoc_scheduler_top::scheduleSR>" << std::endl;
#endif
#ifdef ENABLE_SYSTEMC_VPC
    while (nonStrict || bottom || inCommState) {
      smoc_transition_ready_list  &fromList = (inCommState)?inCommState:((bottom)?bottom:nonStrict); //select by priority
      //      transition_ty           &transition =
      //	  (inCommState)?inCommState.getEventTrigger():((bottom)?bottom.getEventTrigger():nonStrict.getEventTrigger());
#else
    while (nonStrict || bottom) {
      smoc_transition_ready_list  &fromList = (bottom)?bottom:nonStrict; //select by priority
      //      transition_ty           &transition = (bottom)?bottom.getEventTrigger():nonStrict.getEventTrigger();
#endif // ENABLE_SYSTEMC_VPC
      transition_ty           &transition = fromList.getEventTrigger();
      transition_ty::status_t  status     = transition.getStatus();
      
      switch (status) {
        case transition_ty::DISABLED: {
          fromList.remove(transition);
          break;
        }
        case transition_ty::ENABLED: {                                                
          smoc_root_node &n = transition.getActor();
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<actor name=\"" << n.myModule()->name() << "\">" << std::endl;
#endif
          for ( transitionlist_ty::iterator titer = n._currentState->tl.begin();
                titer != n._currentState->tl.end();
                ++titer ){
	    fromList.remove(*titer);
	  }
	  if(n.isNonStrict()){
	    std::cerr << "<nonstrict name=\"" << n.myModule()->name() << "\"\\>" << std::endl;
	    //enable multiple writes (sr_signal will test if nothing changes)
	    smoc_port_list ps  = n.getPorts();
	    for(smoc_port_list::iterator iter = ps.begin();
		iter != ps.end(); iter++){
	      sc_interface *iface = (*iter)->get_interface();
	      if( dynamic_cast<class smoc_root_port_out * >(*iter)) {
		smoc_sr_signal_kind* sig = dynamic_cast<class smoc_sr_signal_kind* >(&(*iface));
		if(sig) sig->multipleWriteSameValue(true);
		cerr << " (out port) " << iface << endl;;
	      }
	    }
	  }else{
	    std::cerr << "<strict name=\"" << n.myModule()->name() << "\"\\>" << std::endl;
	  }
          transition.execute(&n._currentState, &n);

          for ( transitionlist_ty::iterator titer = n._currentState->tl.begin();
                titer != n._currentState->tl.end();
                ++titer ){
#ifdef ENABLE_SYSTEMC_VPC
	      if(n.inCommState()){
		inCommState |= *titer; // still in communication delay state
	      }else
#endif  // ENABLE_SYSTEMC_VPC
	      {
		if(n.isNonStrict()){//FIXME(MS): not only test actors, but test each transition if possible
		  cerr << "NSNSNSNSNSNSNSNSNSNSNS"<< endl;
		  nonStrict   |= *titer; 
		}else{	      // all those strict actors can only be executed once per instant
		  defined     |= *titer; // add to defined list 
		}
	      }
	  }
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</actor>" << std::endl;
#endif
          break;
        }
        default: {
          assert(status == transition_ty::ENABLED ||
                 status == transition_ty::DISABLED   );
        }
      }
      {

	smoc_transition_ready_list instant = bottom | nonStrict;
#ifdef ENABLE_SYSTEMC_VPC
	instant |= inCommState;
#endif  // ENABLE_SYSTEMC_VPC
	if(instant) smoc_wait(instant); // if no more change instant break loop
      }
    }
    for ( smoc_chan_list::const_iterator iter = cs.begin();
	  iter != cs.end();
	  ++iter ){
      std::cerr << (*iter)->kind() << std::endl;
      // "tick()" each block
      dynamic_cast<smoc_sr_signal_kind*>((*iter))->tick();
    }
      
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
          std::cerr << "<findBlocked for " << (*iter)->myModule()->name() << ">" << std::endl;
#endif
          (*iter)->_currentState->findBlocked(ol);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</findBlocked>" << std::endl;
#endif
        }
      }
 */
