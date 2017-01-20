/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef __INCLUDED__SMOC_PERIODIC_ACTOR__HPP__
#define __INCLUDED__SMOC_PERIODIC_ACTOR__HPP__

#include <systemoc/smoc_moc.hpp>
#include "smoc_actor.hpp"

/*class smoc_periodic_actor 
 * used to generate an event periodically.
 * the period, its offset 
 * and a pointer to the managing EventQueue is passed to the Constructor*/
class smoc_periodic_actor: public smoc_actor{

	
public:
  //constructor sets the period, offset and EventQueue
  smoc_periodic_actor(sc_core::sc_module_name name,
                smoc_firing_state & start_state,
                sc_core::sc_time per,
                sc_core::sc_time off,
                float jitter=0.0) :
    smoc_actor(name, start_state),
    period_counter(0),
    period(per),
    offset(off),
    nextReleaseTime_(off),
    jitter(jitter),
    reexecute(false),
    periodicActorActive(true)
  {
    // TODO: negative mobility values may result in negative release times
    //nextReleaseTime_ += calculateMobility();
  }

  sc_core::sc_time calculateMobility() const{
    sc_core::sc_time mobility = sc_core::SC_ZERO_TIME;
    if(jitter != 0.0){
      mobility = jitter * ((rand()/(float(RAND_MAX)+1)*2 )- 1) * period;
    }
    return mobility;
  }

  sc_core::sc_time updateReleaseTime()
  {
    while(nextReleaseTime_ <= sc_core::sc_time_stamp()){
      period_counter++; // increment first, initial execution is scheduled @ offset
      nextReleaseTime_ = period_counter * period + offset + calculateMobility();
    }
    return nextReleaseTime_;
  }

  // override getNextReleaseTime from ScheduledTask
  sc_core::sc_time getNextReleaseTime(){
    if(reexecute){
        reexecute = false;
        return sc_core::sc_time_stamp();
    }else{
    	if(periodicActorActive){
    		return updateReleaseTime();
    	}else{
    		periodicActorActive=true;
    		return sc_core::SC_ZERO_TIME;
    	}
    }
  }


  sc_core::sc_time getPeriod(){ return period; }

  sc_core::sc_time getOffset(){ return offset; }

protected:
  void forceReexecution(){
    reexecute = true;
  }

  void stopPeriodicActorExecution(){
	  periodicActorActive = false;
    }

  void restartPeriodicActorExecution(){
  	  periodicActorActive = true;
    }

private:
  int period_counter;
  sc_core::sc_time period;
  sc_core::sc_time offset;
  sc_core::sc_time nextReleaseTime_;
  float jitter;
  bool reexecute;
  bool periodicActorActive;

};

#endif // __INCLUDED__SMOC_PERIODIC_ACTOR__HPP__
