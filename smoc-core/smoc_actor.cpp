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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph_type.hpp>

#ifdef SYSTEMOC_ENABLE_MAESTROMM
smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s, unsigned int thread_stack_size, bool useLogFile)
  : smoc_root_node(name, smoc_root_node::NODE_TYPE_ACTOR, s),
    SMoCActor(thread_stack_size)
{
  this->setName(this->name());
  this->instanceLogger(this->name(), useLogFile);
  initMMactor();
}

smoc_actor::smoc_actor(smoc_hierarchical_state &s, unsigned int thread_stack_size, bool useLogFile)
  : smoc_root_node(sc_gen_unique_name("smoc_actor"), smoc_root_node::NODE_TYPE_ACTOR, s),
SMoCActor(thread_stack_size)
{
  this->setName(this->name());
  this->instanceLogger(this->name(), useLogFile);
  initMMactor();
}
#else //!defined(SYSTEMOC_ENABLE_MAESTROMM)
smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s)
	: smoc_root_node(name, smoc_root_node::NODE_TYPE_ACTOR, s)
{
}

smoc_actor::smoc_actor(smoc_hierarchical_state &s)
	: smoc_root_node(sc_gen_unique_name("smoc_actor"), smoc_root_node::NODE_TYPE_ACTOR, s)
{
}
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM)

#ifdef SYSTEMOC_ENABLE_MAESTROMM
void smoc_actor::initMMactor()
{
  MM::MMAPI* api = MM::MMAPI::getInstance();
  MetaMap::SMoCActor* a = static_cast<MetaMap::SMoCActor*>(this);
  api->addActor(*a);
}

bool smoc_actor::canExecute()
{
    smoc_actor* sActor =static_cast<smoc_actor*>(this);
    bool canFire = sActor->canFire();

#ifdef SYSTEMOC_ENABLE_VPC
    canFire = canFire && getActive;
#endif //SYSTEMOC_ENABLE_VPC

    return canFire;
}

void smoc_actor::getCurrentTransition(MetaMap::Transition & activeTransition)
{
    smoc_actor* sActor =static_cast<smoc_actor*>(this);
    MetaMap::Transition* transition = (MetaMap::Transition*) sActor->ct;
    activeTransition = *transition;
}

void smoc_actor::registerTransitionReadyListener(MetaMap::TransitionReadyListener& listener)
{
    //smoc Actor
    smoc_actor* sActor =dynamic_cast<smoc_actor*>(this);
    //cout << "R: " << sActor->getName() << " this: " <<  listener.tname << endl;
    //For all states
    RuntimeStateSet states = sActor->getFiringFSM()->getStates();
    for(RuntimeStateSet::iterator si= states.begin(); si != states.end(); si++)
      {
        //For all state transitions
        list<RuntimeTransition> transitions = (*si)->getTransitions();
        for(list<RuntimeTransition>::iterator ti= transitions.begin(); ti != transitions.end(); ti++)
          {
            (*ti).registerTransitionReadyListener(listener);
          }
      }

}

void smoc_actor::execute()
{
    //std::cerr << "smoc::Scheduling::execute" << std::endl;
    //assert(dynamic_cast<smoc_actor*>(actor) != nullptr);
    MetaMap::SMoCActor::execute();
    dynamic_cast<smoc_actor*>(this)->schedule();
}

void smoc_actor::sleep()
{
  wait();
}

#endif //defined(SYSTEMOC_ENABLE_MAESTROMM)

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_actor::finaliseVpcLink() {
  smoc_sysc_port_list ports = getPorts();
  for (smoc_sysc_port_list::iterator iter = ports.begin();
      iter != ports.end(); ++iter)
    (*iter)->finaliseVpcLink(this->name());
}
#endif //SYSTEMOC_ENABLE_VPC

void smoc_actor::setActivation(bool activation){
  smoc_root_node::setActivation(activation);
  //std::cerr << this->name()
  //    << ": smoc_actor::setActivation(" << activation << ")" << std::endl;
#ifdef SYSTEMOC_ENABLE_VPC
  this->notifyActivation(activation);
#endif //SYSTEMOC_ENABLE_VPC


}
