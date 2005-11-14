
#include <smoc_moc.hpp>

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
              diverge(&smoc_scheduler_ndf::schedule);
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
                    diverge(&smoc_scheduler_ndf::schedule) ),
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
  
  getLeafNodes(nodes, c);
  do {
    // FIXME: Big hack !!!
    smoc_ctx       _oldctx = _ctx;
    _ctx.ports_setup.clear();
    
#ifdef SYSTEMOC_DEBUG
    std::cout << "<smoc_scheduler_top::schedule>" << std::endl;
#endif
    // FIXME: Big hack !!!
    // _ctx.hierarchy = c->myModule();
    do {
      again = false;
      for ( smoc_node_list::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter )
        if ( !(*iter)->is_v1_actor )
          again |= (*iter)->currentState().tryExecute();
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
      smoc_wait(ol);
#ifdef SYSTEMOC_DEBUG
      for ( smoc_event_or_list::iterator iter = ol.begin();
            iter != ol.end();
            ++iter )
        std::cout << **iter << std::endl;
#endif
    }
  } while ( 1 );
}
