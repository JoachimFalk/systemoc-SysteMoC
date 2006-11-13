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

#include <smoc_root_port.hpp>
#include <smoc_root_node.hpp>
// #include <systemc/kernel/sc_object_manager.h>
#include <typeinfo>
#include <smoc_firing_rules.hpp>
#include <hscd_tdsim_TraceLog.hpp>

#include <typeinfo>

smoc_root_node::smoc_root_node(smoc_firing_state &s)
  :
#ifndef NDEBUG
//  _finalizeCalled(false),
#endif
    _initialState(s),
    is_v1_actor(false),
#ifdef ENABLE_SYSTEMC_VPC
# ifndef NDEBUG
    vpc_event_lat(NULL),
# endif
    commstate(
      smoc_transition(
        smoc_activation_pattern(Expr::till(vpc_event_dii)),
        smoc_func_diverge(this,&smoc_root_node::_communicate))),
#endif // ENABLE_SYSTEMC_VPC
    _guard(NULL)
  {
#ifdef ENABLE_SYSTEMC_VPC
    commstate.finalise(this);
#endif // ENABLE_SYSTEMC_VPC
    local_constr_args.insert(
        local_constr_args.end(),
        global_constr_args.begin(),
        global_constr_args.end());
    global_constr_args.clear();
    /*
    while(!smoc_root_node::global_constr_args.empty()){
      local_arg_vector.push_back(smoc_root_node::global_arg_stack.top());
      smoc_root_node::global_arg_stack.pop();
    }*/
  }
 
  
std::vector<std::pair<std::string, std::string> >smoc_root_node::global_constr_args; 

 
#ifdef ENABLE_SYSTEMC_VPC
const smoc_firing_state &smoc_root_node::_communicate() {
# ifdef SYSTEMOC_DEBUG
  std::cerr << "    <call actor=" << myModule()->name()
	    << " func=smoc_root_node::communicate>" << std::endl;
  std::cerr << "    <communication type=\"execute\"/>" << std::endl;
# endif
  
# ifdef SYSTEMOC_TRACE
   TraceLog.traceStartDeferredCommunication(myModule()->name());
# endif
  
  assert(vpc_event_dii && vpc_event_lat != NULL);
  
  {
    smoc_ref_event_p foo(vpc_event_lat);
    Expr::evalTo<Expr::CommExec>(*_guard, foo);
    if (!*foo) {
      // latency event not signaled
      struct _: public smoc_event_listener {
        smoc_ref_event_p foo;
        
        bool signaled(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_::signaled(...)" << std::endl;
# endif
          assert(_e == &*foo);
          assert(*_e);
          foo = NULL;
          return false;
        }
        void eventDestroyed(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_:: eventDestroyed(...)" << std::endl;
# endif
          delete this;
        }
        
        _(const smoc_ref_event_p &foo): foo(foo) {};
        
        virtual ~_() {}
      };
      foo->addListener(new _(foo));
    }
  }
  
#ifndef NDEBUG
  vpc_event_lat = NULL;
#endif
  
#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndDeferredCommunication(myModule()->name());
#endif
  
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </call>"<< std::endl;
#endif
  return nextState;
}
#endif // ENABLE_SYSTEMC_VPC

void smoc_root_node::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << myModule()->name() << ": finalise" << std::endl;
#endif
  _currentState = _initialState.finalise(this);
  
  smoc_port_list ports = getPorts();
  
  for (smoc_port_list::iterator iter = ports.begin();
       iter != ports.end();
       ++iter)
    (*iter)->finalise(this);
}

const smoc_port_list smoc_root_node::getPorts() const {
  smoc_port_list   ports;
  const sc_module *m = myModule();
  
  // std::cerr << "=== getPorts ===" << this << std::endl;
  for ( 
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
#endif
        iter != m->get_child_objects().end();
        ++iter ) {
    smoc_root_port *port = dynamic_cast<smoc_root_port *>(*iter);
    
    if ( port != NULL )
      ports.push_back(port);
  }
  return ports;
}

const smoc_firing_states smoc_root_node::getFiringStates() const { 
  smoc_firing_states states;
  const sc_module *m = myModule();
    for ( 
#if SYSTEMC_VERSION < 20050714
      sc_pvector<sc_object*>::const_iterator iter =
          m->get_child_objects().begin();
#else
      std::vector<sc_object*>::const_iterator iter =
          m->get_child_objects().begin();
#endif
      iter != m->get_child_objects().end();
      ++iter ) {
    smoc_firing_state *state = dynamic_cast<smoc_firing_state*>(*iter);
    
    if ( state != NULL )
      states.push_back(state);
  }
  return states;

}

void smoc_root_node::pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
  {}

void smoc_root_node::assemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module          *m  = myModule();
  //const smoc_firing_states  fs = getFiringStates();
  const smoc_port_list      ps = getPorts();
  
  if ( !ps.empty() ) {
    pgw << "<process name=\"" << m->name() << "\" "
                    "type=\"actor\" "
                    "id=\"" << pgw.getId(this) << "\">" << std::endl;
    {
      pgw.indentUp();
      //**********************************PORTS************************************
      for ( smoc_port_list::const_iterator iter = ps.begin();
            iter != ps.end();
            ++iter )
        pgw << "<port name=\"" << (*iter)->name() << "\" "
                     "type=\"" << ((*iter)->isInput() ? "in" : "out") << "\" "
                     "id=\"" << pgw.getId(*iter) << "\"/>" << std::endl;
      //**************************FSM-STATES  && ACTOR-TAG*************************
      assembleActor(pgw);
      //***************************CONTAINED PROBLEMGRAPH**************************
      pgAssemble(pgw, this);
      pgw.indentDown();
    }
    pgw << "</process>" << std::endl;
  } else {
    pgAssemble(pgw, this);
  }
}

/*FIXME: function constructs dump of different tag-levels. necessary, because
  smoc_graph (inherits from smoc_root_node) must not construct this output.
  So it reimplements this virtual function.*/
void smoc_root_node::assembleActor(smoc_modes::PGWriter &pgw ) const {
  const smoc_firing_states  fs = getFiringStates();
  const sc_module          *m  = myModule();
  //*********************************FSM-STATES********************************
  for ( smoc_firing_states::const_iterator iter = fs.begin();
          iter != fs.end();
          ++iter )
        pgw << "<stateDeclaration state=\"" << pgw.getId(&(*iter)->getResolvedState())
            << "\"/>" << std::endl;
        //*******************************ACTOR CLASS*********************************
        pgw << "<actor actorClass=\"" << typeid(*m).name() << "\">" << std::endl;
        {
          pgw.indentUp();
          //***************************CONSTRUCTORPARAMETERS***************************
          for (unsigned int i = 0; i < local_constr_args.size(); i++) {
            std::pair<std::string, std::string> parameterpair = local_constr_args[i];
            pgw << "<constructorParameter "
                   "type=\""  << parameterpair.first  << "\" "
                   "value=\"" << parameterpair.second << "\"/>" << std::endl;
          }
        //************************************FSM************************************
        assembleFSM(pgw);
        pgw.indentDown();
      }
      pgw << "</actor>" << std::endl;
}

void smoc_root_node::assembleFSM( smoc_modes::PGWriter &pgw ) const {
  pgw << "<fsm startstate=\"" << pgw.getId(&_initialState.getResolvedState()) << "\">" << std::endl;
  {
    pgw.indentUp();
    //******************************FSMSTATES************************************ 
    const smoc_firing_rules               &fsmRules  = _initialState.getFiringRules(); 
    const smoc_firing_types::statelist_ty &fsmStates = fsmRules.getFSMStates(); 
    
    for (smoc_firing_rules::statelist_ty::const_iterator fsmiter =fsmStates.begin(); 
         fsmiter != fsmStates.end(); 
         ++fsmiter) {
      pgw << "<state id=\"" << pgw.getId(*fsmiter)<< "\">" << std::endl;
      {
        pgw.indentUp();
        //**************TRANTIONS********************
        const smoc_firing_types::transitionlist_ty &cTraSt = (*fsmiter)->tl;
        
        for ( smoc_firing_types::transitionlist_ty::const_iterator titer = cTraSt.begin(); 
              titer != cTraSt.end(); 
              ++titer ) {
          const smoc_firing_types::statelist_ty &cToNState = titer->sl;
          
          assert( cToNState.size() <= 1 );
          if ( cToNState.size() == 1 ) {
            pgw << "<transition nextstate=\"" << pgw.getId(*cToNState.begin()) << "\" " << std::flush;
            if (CoSupport::isType<smoc_func_call>(titer->f)) {
              pgw << "action=\"" << static_cast<const smoc_func_call &>(titer->f).getFuncName() << "\">" << std::endl;
            } else {
              pgw << "action=\"\">" << std::endl;
            }
            titer->guardAssemble(pgw);
            pgw << "</transition>" << std::endl;
          } else {
            pgw << "<transition nextState=\"FIXME!!!\"/>" << std::endl;
          }
        }
        //***************/TRANTIONS*****************
        pgw.indentDown();
      }
      pgw << "</state>" << std::endl;
    }
    //*********************************/FSMSTATES*************************************
    pgw.indentDown();
  }
  pgw << "</fsm>" << std::endl;
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

void Expr::Detail::registerParam(const ArgInfo &argInfo) {
  smoc_root_node::global_constr_args.push_back(argInfo);
}
