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
  smoc_periodic_actor(sc_module_name name,
                smoc_firing_state & start_state,
                sc_time per,
                sc_time off,
                float jitter=0.0) :
    smoc_actor(name, start_state),
    period_counter(0),
    period(per),
    offset(off),
    nextReleaseTime_(off),
    jitter(jitter),
    reexecute(false)
  {
    // TODO: negative mobility values may result in negative release times
    //nextReleaseTime_ += calculateMobility();
  }

  sc_time calculateMobility() const{
    sc_time mobility = SC_ZERO_TIME;
    if(jitter != 0.0){
      mobility = jitter * ((rand()/(float(RAND_MAX)+1)*2 )- 1) * period;
    }
    return mobility;
  }

  sc_time updateReleaseTime()
  {
    while(nextReleaseTime_ <= sc_time_stamp()){
      period_counter++; // increment first, initial execution is scheduled @ offset
      nextReleaseTime_ = period_counter * period + offset + calculateMobility();
    }
    return nextReleaseTime_;
  }

  // override getNextReleaseTime from ScheduledTask
  sc_time getNextReleaseTime(){
    if(reexecute){
        reexecute = false;
        return sc_time_stamp();
    }else{
        return updateReleaseTime();
    }
  }


sc_time getPeriod(){ return period; }

sc_time getOffset(){ return offset; }

protected:
  void forceReexecution(){
    reexecute = true;
  }

private:
  int period_counter;
  sc_time period;
  sc_time offset;
  sc_time nextReleaseTime_;
  float jitter;
  bool reexecute;

};

#endif // __INCLUDED__SMOC_PERIODIC_ACTOR__HPP__
