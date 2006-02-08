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

#include <cosupport/oneof.hpp>

using namespace CoSupport;

void smoc_scheduler_sdf::schedule() {
  assert("FIXME sdf scheduler unfinished !" == NULL);
}

void smoc_scheduler_sdf::analyse() {
  s = smoc_activation_pattern(Expr::literal(true), true) >>
      call(&smoc_scheduler_sdf::schedule) >> s;
}

smoc_scheduler_sdf::smoc_scheduler_sdf( cset_ty *c )
  : smoc_root_node(s), c(c) { analyse(); }

const smoc_firing_state &smoc_scheduler_ndf::schedule() {
  assert( 0 );
/*
  
  bool           again;
  smoc_node_list nodes   = c->getNodes();

  // FIXME: Big hack !!!
  smoc_ctx       _oldctx = _ctx;
  _ctx.ports_setup.clear();
  
#ifdef SYSTEMOC_DEBUG
  std::cout << "<smoc_scheduler_ndf::schedule>" << std::endl;
#endif
  // FIXME: Big hack !!!
  // _ctx.hierarchy = myModule();
  do {
    again = false;
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      if ( !(*iter)->is_v1_actor )
        again |= (*iter)->currentState().tryExecute();
//      wait(SC_ZERO_TIME);
  } while (again);
  {
    smoc_transition_list      tl;
    smoc_root_port_bool_list  l;
    
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      if ( !(*iter)->is_v1_actor )
        (*iter)->currentState().findBlocked(l);
#ifdef SYSTEMOC_DEBUG
    std::cout << "CREATE TRANSITIONS: " << l << std::endl;
#endif
    for ( smoc_root_port_bool_list::const_iterator iter = l.begin();
          iter != l.end();
          ++iter ) {
      tl |= smoc_activation_pattern(Expr::vguard(*iter), true) >>
	      smoc_interface_action(smoc_func_diverge(this,&smoc_scheduler_ndf::schedule));
    }
    s = tl;
  }
#ifdef SYSTEMOC_DEBUG
  std::cout << "</smoc_scheduler_ndf::schedule>" << std::endl;
#endif
  // FIXME: Big hack !!!
  _ctx = _oldctx;
  return s;
*/
}

smoc_scheduler_ndf::smoc_scheduler_ndf( cset_ty *c )
  : smoc_root_node( smoc_activation_pattern(Expr::literal(true), true) >>
		    smoc_interface_action(smoc_func_diverge(this,&smoc_scheduler_ndf::schedule))),
  c(c) {
#ifdef SYSTEMOC_DEBUG
  std::cout << "smoc_scheduler_ndf" << std::endl;
#endif
}

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
  bool           again;
  smoc_node_list nodes;

  int guard_success = 0;
  int guard_fail    = 0;
  
  getLeafNodes(nodes, c);
  do {
    // FIXME: Big hack !!!
    smoc_ctx       _oldctx = _ctx;
    // _ctx.ports_setup.clear();
    
#ifdef SYSTEMOC_DEBUG
    std::cout << "<smoc_scheduler_top::schedule>" << std::endl;
#endif
    // FIXME: Big hack !!!
    // _ctx.hierarchy = c->myModule();
    do {
      again = false;
      for ( smoc_node_list::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        if ( !(*iter)->is_v1_actor ) {
          bool canexec;
          smoc_firing_types::resolved_state_ty **rs =
            &(*iter)->currentState().rs;
          
          // assert( _ctx.ports_setup.empty() );
          do {
            canexec = false;
            
            for ( transitionlist_ty::iterator titer = (*rs)->tl.begin();
                  titer != (*rs)->tl.end() && !canexec;
                  ++titer ) {
              canexec =
                titer->knownSatisfiable().getStatus() ==
                smoc_root_port_bool::IS_ENABLED;
              
              assert( isType<NILTYPE>(titer->f) ||
                      isType<smoc_func_diverge>(titer->f) ||
                      isType<smoc_func_branch>(titer->f) ||
                      isType<smoc_func_call>(titer->f) );
              if ( canexec ) {
                titer->execute(rs,*iter);
                again = true;
              }
              /*
              for ( smoc_port_list::iterator iter =  _ctx.ports_setup.begin();
                    iter != _ctx.ports_setup.end();
                    ++iter )
                (*iter)->reset();
              _ctx.ports_setup.clear();
               */
            }
          } while ( canexec );
        }
      }
    } while (again);
    {
      smoc_event_or_list        ol;
      
      for ( smoc_node_list::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        if ( !(*iter)->is_v1_actor ) {
          (*iter)->currentState().findBlocked(ol);
        }
      }
#ifdef SYSTEMOC_DEBUG
      std::cout << "</smoc_scheduler_top::schedule>" << std::endl;
#endif
      // FIXME: Big hack !!!
      _ctx = _oldctx;
//    std::cout << "guard_success: " << guard_success << std::endl;
//    std::cout << "guard_fail:    " << guard_fail    << std::endl;
      smoc_wait(ol);
#ifdef SYSTEMOC_DEBUG
      std::cout << ol << std::endl;
#endif
    }
  } while ( 1 );
}
