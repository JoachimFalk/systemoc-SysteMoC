#ifndef __INCLUDED__TT__HPP__
#define __INCLUDED__TT__HPP__

#include <CoSupport/compatibility-glue/nullptr.h>

#include "Event.hpp"

#include <queue>

/*struct used to store an event with a certain release-time*/
struct TimeEventPair{
  TimeEventPair(sc_core::sc_time time,  Event *event)
    : time(time), event(event) {}
  sc_core::sc_time time;
  Event *event;
};


/*struct used for comparison
 * needed by the priority_queue*/
struct eventCompare{
  bool operator()(const TimeEventPair& tep1,
                  const TimeEventPair& tep2) const
  {
    sc_core::sc_time p1=tep1.time;
    sc_core::sc_time p2=tep2.time;
    if (p1 >= p2)
      return true;
    else
      return false;
  }
};

/*Class EventQueue stores Events and notifies them 
 * if their releasetime has been arrived */
class EventQueue : public sc_core::sc_module{
  SC_HAS_PROCESS(EventQueue);	
	
public:		
  //SystemC-Prozess, der die queue topped und die notwendige Zeit wartet.
  void waiter();
		
  EventQueue(sc_core::sc_module_name name): sc_core::sc_module(name), current(nullptr) {
    SC_THREAD(waiter);
  };

  // register an event with its next releasetime in the EventQueue
  void registerEvent(Event* event, sc_core::sc_time time);	

private:
  TimeEventPair* current;
  sc_core::sc_event wait_interrupt;
  typedef std::priority_queue <TimeEventPair,
                               std::vector<TimeEventPair>,
                               eventCompare>                  TimedQueue;

  TimedQueue pqueue;

};





/*class PeriodicActor 
 * used to generate an event periodically.
 * the period, its offset 
 * and a pointer to the managing EventQueue is passed to the Constructor*/
class PeriodicActor: public smoc_actor, public Event{
  int counter;
	
public:
  //constructor sets the period, offset and EventQueue
  PeriodicActor(sc_core::sc_module_name name,
                smoc_firing_state & start_state,
                sc_core::sc_time per,
                sc_core::sc_time off,
                EventQueue* eventQueue,
                float mobility_fact=0.0) :
    smoc_actor(name, start_state),
    ::Event(),
    counter(0),
    period(per),
    offset(off),
    eq(eventQueue),
    mobility_factor(mobility_fact)
  {
    //and register it at the EventQueue
    eq->registerEvent( this, offset );
  }

  //inherited method, is called by the EventQueue
  void notify();

  void resetEvent(){
    smoc_reset( this->getEvent() );
  }
	
protected:
  sc_core::sc_time period;
  sc_core::sc_time offset;
  EventQueue* eq;
  float mobility_factor;
};

#endif // __INCLUDED__TT__HPP__
