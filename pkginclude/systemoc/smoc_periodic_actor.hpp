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
    jitter(jitter)
  {
  }

sc_time getNextReleasetime(){
  period_counter++;
  sc_time mobility = sc_time(0,SC_US);
  if(jitter != 0.0){ 
    mobility = jitter * ((rand()/(float(RAND_MAX)+1)*2 )- 1) * period;
  }
  return (period_counter * period + offset + mobility );
}

sc_time getPeriod(){ return period; }

sc_time getOffset(){ return offset; }

private:
  int period_counter;
  sc_time period;
  sc_time offset;
  float jitter;

};

#endif // __INCLUDED__SMOC_PERIODIC_ACTOR__HPP__
