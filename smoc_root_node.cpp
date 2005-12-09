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

#include <smoc_root_port.hpp>
#include <smoc_root_node.hpp>
// #include <systemc/kernel/sc_object_manager.h>
#include <smoc_firing_rules.hpp>
#include <hscd_tdsim_TraceLog.hpp>

smoc_root_node::smoc_root_node(const smoc_firing_state &s)
  :
#ifndef NDEBUG
    _finalizeCalled(false),
#endif
    _currentState(s),
    _initialState(_currentState),
    is_v1_actor(false)
#ifdef ENABLE_SYSTEMC_VPC
  , commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
	       smoc_interface_action(smoc_func_diverge(
						       this,&smoc_root_node::communicate)) ) 
#endif // ENABLE_SYSTEMC_VPC
  {}
smoc_root_node::smoc_root_node(smoc_firing_state &s)
  :
#ifndef NDEBUG
    _finalizeCalled(false),
#endif
    _initialState(s),
    is_v1_actor(false)
#ifdef ENABLE_SYSTEMC_VPC
  , commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
	       smoc_interface_action(smoc_func_diverge(
						       this,&smoc_root_node::communicate)) )
#endif // ENABLE_SYSTEMC_VPC
  {}

#ifdef ENABLE_SYSTEMC_VPC
const smoc_firing_state &smoc_root_node::communicate() {
    
# ifdef SYSTEMOC_DEBUG
  std::cout << "  <call actor=" << myModule()->name()
	    << " func=smoc_root_node::communicate>" << std::endl;
  std::cout << "    <communication type=\"execute\"/>" << std::endl;
# endif

#ifdef SYSTEMOC_TRACE
   TraceLog.traceStartDeferredCommunication(myModule()->name());
#endif
 
  assert( vpc_event );
    
  for ( smoc_port_list::iterator iter = ports_setup.begin();
	iter != ports_setup.end();
	++iter ) {
    (*iter)->commExec();
    (*iter)->reset();
  }
  ports_setup.clear();

#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndDeferredCommunication(myModule()->name());
#endif

# ifdef SYSTEMOC_DEBUG
  std::cout << "  </call>"<< std::endl;
# endif
  return nextState;
}
#endif // ENABLE_SYSTEMC_VPC

void smoc_root_node::finalise() {
#ifndef NDEBUG
  // PARANOIA
  // std::cout << myModule()->name() << ": finalise" << std::endl;
  assert( !_finalizeCalled );
  _finalizeCalled = true;
#endif
  if ( &_initialState != &_currentState ) {
    _currentState = _initialState;
  }
  _currentState.finalise(this);
}

const smoc_port_list smoc_root_node::getPorts() const {
  smoc_port_list   ports;
  const sc_module *m = myModule();
  
  // std::cout << "=== getPorts ===" << this << std::endl;
  for ( sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
        iter != m->get_child_objects().end();
        ++iter ) {
    smoc_root_port *port = dynamic_cast<smoc_root_port *>(*iter);
    
    if ( port != NULL )
      ports.push_back(port);
  }
  return ports;
}

void smoc_root_node::pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
  {}

void smoc_root_node::assemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module     *m  = myModule();
  const smoc_port_list ps = getPorts();
  
  if ( !ps.empty() ) {
    pgw << "<process name=\"" << m->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
    pgw.indentUp();
    for ( smoc_port_list::const_iterator iter = ps.begin();
          iter != ps.end();
          ++iter )
      pgw << "<port name=\"" << (*iter)->name() << "\" "
          << "type=\"" << ((*iter)->isInput() ? "in" : "out") << "\" "
          << "id=\"" << pgw.getId(*iter) << "\"/>" << std::endl;
    pgw << "<fsm startstate=\"" << pgw.getId(&_initialState.getResolvedState()) << "\">" << std::endl;
    pgw.indentUp();
  
    
   //*****************************FSMSTATES************************************ 
    const smoc_firing_rules               &fsmRules  = _initialState.getFiringRules(); 
    const smoc_firing_types::statelist_ty &fsmStates = fsmRules.getFSMStates(); 
    for (smoc_firing_rules::statelist_ty::const_iterator fsmiter =fsmStates.begin(); 
        fsmiter != fsmStates.end(); 
           ++fsmiter) {
      pgw << "<state id=\"" << pgw.getId(*fsmiter)<< "\">" << std::endl;
      pgw.indentUp();
       
      //**************TRANTIONS********************
      const smoc_firing_types::transitionlist_ty &cTraSt = (*fsmiter)->tl;
      
      for ( smoc_firing_types::transitionlist_ty::const_iterator titer = cTraSt.begin(); 
            titer != cTraSt.end(); 
            ++titer ) {
        const smoc_firing_types::statelist_ty &cToNState = titer->sl;

        assert( cToNState.size() <= 1 );

        if ( cToNState.size() == 1 ) {
          pgw << "<transition nextstate=\"" << pgw.getId(*cToNState.begin()) << "\" "
                << "action=\"" << static_cast<const smoc_func_call &>(titer->f).getFuncName() << "\">" << std::endl;
          titer->ap.guardAssemble(pgw);
          pgw << "</transition>" << std::endl;
        } else {
          pgw << "<transition FIXME !!!/>" << std::endl;
        }
      }
      //***************/TRANTIONS*****************
   
      pgw.indentDown();
      pgw << "</state>" << std::endl;
    }
    //*********************************/FSMSTATES*************************************
    
    pgw.indentDown();
    pgw << "</fsm>" << std::endl;
  }
  pgAssemble( pgw, this );
  if ( !ps.empty() ) {
    pgw.indentDown();
    pgw << "</process>" << std::endl;
  }
}

std::ostream &smoc_root_node::dumpActor(std::ostream &o) {
  o << "actor: " << myModule()->name() << std::endl;
  smoc_port_list ps = getPorts();
  o << "  ports:" << std::endl;
  for ( smoc_port_list::const_iterator iter = ps.begin();
        iter != ps.end();
        ++iter ) {
    o << "  " << *iter << std::endl;
  }
  return o;
}
