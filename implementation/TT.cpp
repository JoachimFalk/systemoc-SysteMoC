#include <actorlibrary/tt/TT.hpp>

float randfloat(void){
  return (rand()/(float(RAND_MAX)+1)*2 )- 1;
}

void PeriodicActor::notify(){
  //notify signal
  //event->notify();
  smoc_notify(getEvent());
  //count the cycles
  counter++;
  sc_time mobility = sc_time(0,SC_US);
  if(mobility_factor != 0.0){ 
    mobility = mobility_factor * randfloat() * period;
    //cerr<<"WAIT-Time / Mobility: "<< mobility <<" @ "<<sc_time_stamp()<< endl;
  }
  //re-register Event at the EventQueue
  eq->registerEvent(this, sc_time_stamp() + period + mobility );
	
}

void EventQueue::registerEvent(Event* event, sc_time time){
  //create a new element and add it to the queue
  TimeEventPair tep(time, event);
  pqueue.push(tep);	
  //is it earlier to release then the current event?
  if(current!=NULL && time < current->time){
    // 		cout<<"Spezialfall! "<<endl;
    wait_interrupt.notify();
  }

	
}

//systemC-thread, has to notify the signals at the correct time
void EventQueue::waiter(){
  while(true){
    if(pqueue.size()){
      //get the top element and let's wait until its releasetime has be arrived
      TimeEventPair pair = pqueue.top();
      current=&pair;
      //			pqueue.pop();
      sc_time toWait=current->time-sc_time_stamp();

      // if not, something very strange happend
      assert(toWait >= sc_time(0,SC_NS));

      wait(toWait, wait_interrupt);
			
      while(current->time == sc_time_stamp()){
        //now we are at the releasetime, so let's notify the event
        pqueue.pop();
        current->event->notify();
        pair = pqueue.top();
        current=&pair;
      }
			
    }else {
      current= NULL;
      wait();
    }
  }
	
}
