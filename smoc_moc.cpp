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

  //int guard_success = 0;
  //int guard_fail    = 0;
  
  getLeafNodes(nodes, c);
  do {
    //// FIXME: Big hack !!!
    //smoc_ctx       _oldctx = _ctx;
    // _ctx.ports_setup.clear();
    
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<smoc_scheduler_top::schedule>" << std::endl;
#endif
    // FIXME: Big hack !!!
    // _ctx.hierarchy = c->myModule();
    /*
    for ( smoc_node_list::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter ) {
      smoc_firing_types::resolved_state_ty *rs = (*iter)->_currentState;
      
      for ( transitionlist_ty::iterator titer = rs->tl.begin();
            titer != rs->tl.end();
            ++titer )
        ol |= titer->ap.al;
    }*/
    do {
      again = false;
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
#ifdef SYSTEMOC_DEBUG
      std::cerr << "</smoc_scheduler_top::schedule>" << std::endl;
#endif
      //// FIXME: Big hack !!!
      //_ctx = _oldctx;
//    std::cerr << "guard_success: " << guard_success << std::endl;
//    std::cerr << "guard_fail:    " << guard_fail    << std::endl;
      smoc_wait(ol);
      ol.clear();
#ifdef SYSTEMOC_DEBUG
      std::cerr << ol << std::endl;
#endif
    }
  } while ( 1 );
}
