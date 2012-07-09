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

#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph_type.hpp>

smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s)
  : smoc_root_node(name, s)
{
#ifdef SYSTEMOC_ENABLE_METAMAP
  this->setName(this->name());
  initMMactor();
#endif
}

#ifdef SYSTEMOC_ENABLE_METAMAP
smoc_actor::smoc_actor(string name, smoc_hierarchical_state &s)
  : smoc_root_node(name, s), MetaMap::Actor(name)
{
  initMMactor();
}
#endif


smoc_actor::smoc_actor(smoc_firing_state &s)
  : smoc_root_node((string) sc_gen_unique_name("smoc_actor"), s)
{
#ifdef SYSTEMOC_ENABLE_METAMAP
  this->setName(this->name());
  initMMactor();
#endif
}

#ifdef SYSTEMOC_ENABLE_METAMAP
void smoc_actor::initMMactor()
{
  /////////////////
  //rrr
  MM::MMAPI api = MM::MMAPI::getInstance();
  MetaMap::Actor* a = static_cast<MetaMap::Actor*>(this);
  api.addActor(*a);
  ////////////////
}

bool smoc_actor::canExecute()
{
    smoc_actor* sActor =static_cast<smoc_actor*>(this);
    bool canFire = sActor->canFire();

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
    //SysteMoC Actor
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

void smoc_actor::registerSleepingListener(MetaMap::SleepingListener& sListener){
      //SysteMoC Actor
      smoc_actor* sActor =dynamic_cast<smoc_actor*>(this);

      sActor->sleepingListener = &sListener;
}

void smoc_actor::execute()
{
    //std::cerr << "SysteMoC::Scheduling::execute" << std::endl;
    //assert(dynamic_cast<smoc_actor*>(actor) != NULL);
    MetaMap::Actor::execute();
    dynamic_cast<smoc_actor*>(this)->schedule();
}

void smoc_actor::wait( double v, sc_time_unit tu ){

  //this->sleepingListener->notifySleeping(*this);
  this->sleepingListener->notifyWillWakeUp(this->getName(),v);
  sc_module::wait(v,tu);

  this->sleepingListener->notifyAwaken(*this);
}

void smoc_actor::wait( sc_time sct )
{
  //this->sleepingListener->notifySleeping(*this);
    this->sleepingListener->notifyWillWakeUp(this->getName(),sct.to_double());
    sc_module::wait(sct);

    this->sleepingListener->notifyAwaken(*this);
}

void smoc_actor::wait(){

  this->sleepingListener->notifySleeping(*this);

  sc_module::wait();

  this->sleepingListener->notifyAwaken(*this);
}

void smoc_actor::sleep(){

//  this->sleepingListener->notifySleeping(*this);
  wait();
}

#endif

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
