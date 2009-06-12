// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <typeinfo>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_root_node.hpp>
#include <systemoc/detail/hscd_tdsim_TraceLog.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <smoc/smoc_simulation_ctx.hpp>

using namespace SysteMoC::Detail;

smoc_root_node::smoc_root_node(sc_module_name name, smoc_hierarchical_state &s/*, bool regObj*/)
  : sc_module(name),
#if defined(SYSTEMOC_ENABLE_DEBUG)
//  _finalizeCalled(false),
#endif
    initialState(s),
    _non_strict(false)
#ifdef SYSTEMOC_ENABLE_VPC
    ,diiEvent(new smoc_ref_event())
//# if defined(SYSTEMOC_ENABLE_DEBUG)
//    vpc_event_lat(NULL),
//# endif
#endif // SYSTEMOC_ENABLE_VPC
//  _guard(NULL)
{
#ifdef SYSTEMOC_ENABLE_VPC
  commstate = new RuntimeState();
  commstate->getTransitions().push_back(
      RuntimeTransition(
        this,
        Expr::till(*diiEvent),
        smoc_func_diverge(this, &smoc_root_node::_communicate)));
#endif // SYSTEMOC_ENABLE_VPC

//if(regObj) idPool.regObj(this);
}
 
#ifdef SYSTEMOC_ENABLE_VPC
RuntimeState* smoc_root_node::_communicate() {
  assert(diiEvent != NULL && *diiEvent); // && vpc_event_lat != NULL
  return nextState;
}
#endif // SYSTEMOC_ENABLE_VPC

void smoc_root_node::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_node::finalise() begin, name == " << this->name() << std::endl;
#endif
  
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS  
  
  // finalise ports before FSM (ActivationPattern needs port nodes)
  smoc_sysc_port_list ports = getPorts();
  for (smoc_sysc_port_list::iterator iter = ports.begin();
      iter != ports.end(); ++iter)
    (*iter)->finalise();
  
  getFiringFSM()->finalise(this, initialState.getImpl());
  currentState = getFiringFSM()->getInitialState();
  
  //std::cerr << "smoc_root_node::finalise() name == " << this->name() << std::endl
  //          << "  FiringFSM: " << currentState->getFiringFSM()
  //          << "; #leafStates: " << currentState->getFiringFSM()->getLeafStates().size()
  //          << std::endl;
  
  //check for non strict transitions
  const RuntimeStateSet& states = getFiringFSM()->getStates(); 
  
  for (RuntimeStateSet::const_iterator sIter = states.begin(); 
       sIter != states.end();
       ++sIter) {
    const RuntimeTransitionList& tl = (*sIter)->getTransitions();
    
    for (RuntimeTransitionList::const_iterator tIter = tl.begin();
         tIter != tl.end();
         ++tIter) {
      if (boost::get<smoc_sr_func_pair>(&tIter->getAction())) {
#ifdef SYSTEMOC_DEBUG
        std::cout << "found non strict SR block: " << this->name() << endl;
#endif
        _non_strict = true;
      }
    }
  }
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_node::finalise() end, name == " << this->name() << std::endl;
#endif
}

smoc_sysc_port_list smoc_root_node::getPorts() const {
  smoc_sysc_port_list ret;
  
  for( 
#if SYSTEMC_VERSION < 20050714
    sc_pvector<sc_object*>::const_iterator iter =
#else
    std::vector<sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(smoc_sysc_port* p = dynamic_cast<smoc_sysc_port*>(*iter))
      ret.push_back(p);
  }
  return ret;
}

/*
RuntimeStateList smoc_root_node::getStates() const { 
  RuntimeStateList ret;

  for( 
#if SYSTEMC_VERSION < 20050714
    sc_pvector<sc_object*>::const_iterator iter =
#else
    std::vector<sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(RuntimeState* s = dynamic_cast<RuntimeState*>(*iter))
      ret.push_back(s);
  }
  return ret;
}
*/

std::ostream &smoc_root_node::dumpActor(std::ostream &o) {
  o << "actor: " << this->name() << std::endl;
  smoc_sysc_port_list ps = getPorts();
  o << "  ports:" << std::endl;
  for ( smoc_sysc_port_list::const_iterator iter = ps.begin();
        iter != ps.end();
        ++iter ) {
    o << "  " << *iter << std::endl;
  }
  return o;
}

bool smoc_root_node::inCommState() const{
#ifdef SYSTEMOC_ENABLE_VPC
    return currentState == getCommState();
#else // SYSTEMOC_ENABLE_VPC
    return false;
#endif // SYSTEMOC_ENABLE_VPC
}

bool smoc_root_node::isNonStrict() const{
  return _non_strict;
}

void smoc_root_node::addCurOutTransitions(smoc_transition_ready_list& ol) const {
  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol |= *tIter;
  }
}

void smoc_root_node::delCurOutTransitions(smoc_transition_ready_list& ol) const {
  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol.remove(*tIter);
  }
}

smoc_root_node::~smoc_root_node() {
//idPool.unregObj(this);
#ifdef SYSTEMOC_ENABLE_VPC
  delete commstate;
#endif // SYSTEMOC_ENABLE_VPC
}
