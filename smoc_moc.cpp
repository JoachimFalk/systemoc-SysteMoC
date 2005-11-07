
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
  bool           again;
  smoc_node_list nodes   = c->getNodes();

  // FIXME: Big hack !!!
  smoc_ctx       _oldctx = _ctx;
  _ctx.ports_setup.clear();
  
#ifdef SYSTEMOC_DEBUG
  std::cout << "<smoc_scheduler_ndf::schedule>" << std::endl;
#endif
  // FIXME: Big hack !!!
  _ctx.hierarchy = myModule();
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
}

smoc_scheduler_ndf::smoc_scheduler_ndf( cset_ty *c )
  : smoc_root_node( smoc_activation_pattern(Expr::literal(true), true) >>
                    diverge(&smoc_scheduler_ndf::schedule) ),
  c(c) {
#ifdef SYSTEMOC_DEBUG
  std::cout << "smoc_scheduler_ndf" << std::endl;
#endif
}

void smoc_scheduler_top::schedule(smoc_graph *c) {
  do {
    bool executed = c->currentState().tryExecute();
    assert( executed == true );
    {
#ifdef SYSTEMOC_DEBUG
      std::cout << "in top scheduler !!!" << std::endl;
#endif
      smoc_root_port_bool_list l;
      
      smoc_event_or_list ol;
      c->currentState().findBlocked(l);
      for ( smoc_root_port_bool_list::iterator iter = l.begin();
            iter != l.end();
            ++iter ) {
        smoc_root_port_bool::reqs_ty &reqs = iter->reqs;
        
       /* 
        smoc_event_and_list al;
        for ( smoc_root_port_bool::reqs_ty::iterator riter = reqs.begin();
              riter != reqs.end();
              ++riter )
          al &= *static_cast<smoc_event *>(*riter);
        ol |= al;*/
#ifdef SYSTEMOC_DEBUG
        std::cout << reqs << std::endl;
#endif
        assert( reqs.size() <=  1 );
        if ( !reqs.empty() ) {
          ol |= *static_cast<smoc_event *>(*reqs.begin());
        }
      }
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

