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
#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/ClockI.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#ifdef SYSTEMOC_ENABLE_MAESTRO
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
#else //!defined(SYSTEMOC_ENABLE_MAESTRO)
smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s)
	: smoc_root_node(name, smoc_root_node::NODE_TYPE_ACTOR, s)
{
}

smoc_actor::smoc_actor(smoc_hierarchical_state &s)
	: smoc_root_node(sc_gen_unique_name("smoc_actor"), smoc_root_node::NODE_TYPE_ACTOR, s)
{
}
#endif //!defined(SYSTEMOC_ENABLE_MAESTRO)

#ifdef SYSTEMOC_ENABLE_MAESTRO
void smoc_actor::initMMactor()
{
  MM::MMAPI* api = MM::MMAPI::getInstance();
  
  api->addActor(*this);
}

bool smoc_actor::canExecute()
{
    //smoc_actor* sActor =static_cast<smoc_actor*>(this);
    bool canFire = this->canFire();

#ifdef SYSTEMOC_ENABLE_VPC
    canFire = canFire && getActive;
#endif //SYSTEMOC_ENABLE_VPC

    return canFire;
}

bool smoc_actor::testCanExecute()
{
	//smoc_actor* sActor =static_cast<smoc_actor*>(this);
	bool canFire = this->testCanFire();

	return canFire;
}

bool smoc_actor::isScheduled()
{
	return this->scheduled;
}

void smoc_actor::setScheduled(bool set)
{
	scheduled = set;
}

void smoc_actor::getCurrentTransition(MetaMap::Transition*& activeTransition)
{
    //smoc_actor* sActor =static_cast<smoc_actor*>(this);
    activeTransition = (MetaMap::Transition*) this->ct;
}

void smoc_actor::registerTransitionReadyListener(MetaMap::TransitionReadyListener& listener)
{
    //For all states
    RuntimeStateSet states = this->getFiringFSM()->getStates();

    for(RuntimeStateSet::iterator si= states.begin(); si != states.end(); si++)
      {
        //For all state transitions
        for(list<RuntimeTransition>::iterator ti= (*si)->getTransitions().begin(); ti != (*si)->getTransitions().end(); ti++)
          {
            (*ti).registerTransitionReadyListener(listener);
          }
      }

}

#ifdef MAESTRO_ENABLE_POLYPHONIC
void smoc_actor::registerThreadDoneListener(MetaMap::ThreadDoneListener& listener)
{
	//For all states
	RuntimeStateSet states = this->getFiringFSM()->getStates();

	for (RuntimeStateSet::iterator si = states.begin(); si != states.end(); si++)
	{
		//For all state transitions
		for (list<RuntimeTransition>::iterator ti = (*si)->getTransitions().begin(); ti != (*si)->getTransitions().end(); ti++)
		{
			(*ti).registerThreadDoneListener(listener);
		}
	}

}
#endif


void smoc_actor::execute()
{
  //notify actor activity to runtime manager
  MetaMap::SMoCActor::execute();
    
  //execute and set next transition if there is any transition ready
  this->schedule();
}

void smoc_actor::wait(double v, sc_time_unit tu )
{
#ifdef MAESTRO_ENABLE_POLYPHONIC
	this->waitListener->notifyWillWaitTime(*this);
#endif
  sc_module::wait(v,tu);
#ifdef MAESTRO_ENABLE_POLYPHONIC
  this->waitListener->notifyTimeEllapsedAndAwaken(*this);
#endif
}

void smoc_actor::wait(sc_time sct )
{
#ifdef MAESTRO_ENABLE_POLYPHONIC
	this->waitListener->notifyWillWaitTime(*this);
#endif
    sc_module::wait(sct);
#ifdef MAESTRO_ENABLE_POLYPHONIC
	this->waitListener->notifyTimeEllapsedAndAwaken(*this);
#endif
}

void smoc_actor::wait(sc_event& waitEvent)
{
	sc_module::wait(waitEvent);
}

void smoc_actor::wait()
{
  sc_module::wait();
}

void smoc_actor::sleep(sc_event& wakeUpevent)
{
  wait(wakeUpevent);
}

double smoc_actor::getLocalTimeDiff()
{
	sc_time localTime = localClock->computeTimeStamp();
	sc_time globalTime = sc_time_stamp();

	return globalTime.to_double() - localTime.to_double();

}

void smoc_actor::localClockWait(sc_time sct)
{
	sc_time totalTimeToWait;

	//ClockI* clock = SMoCActor::trace->getClock();

	//double localTime = clock->computeTimeStamp().to_double();

	double freqFactor = localClock->getFreqFactor();

	//double offset = clock->getOffset().to_double() * clock->getOffsetSign();

	//double shift = clock->getShift().to_double() * clock->getShiftSign();

	double totalTime = (sct.to_double() /*- shift - offset*/)*freqFactor;

#ifdef MAESTRO_ENABLE_POLYPHONIC
	this->waitListener->notifyWillWaitTime(*this);
#endif
	sc_module::wait(totalTime,SC_PS);
#ifdef MAESTRO_ENABLE_POLYPHONIC
	this->waitListener->notifyTimeEllapsedAndAwaken(*this);
#endif
}

void smoc_actor::localClockWait(double v, sc_time_unit tu)
{
	localClockWait(sc_time(v, tu));
}

#endif //defined(SYSTEMOC_ENABLE_MAESTRO)

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
