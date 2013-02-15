#ifndef __INCLUDED__EVENT_HPP_
#define __INCLUDED__EVENT_HPP_

#include <systemoc/smoc_moc.hpp>

//Abstract class Event
class TT_Event{
public:
  //method notify() used to be called by the EventQueue
  virtual void notify()=0;

  smoc_event& getEvent(){
    return myevent;			
  }

  TT_Event(): myevent(){}
  virtual ~TT_Event() {}

private:
  smoc_event myevent;
};

#endif /*__INCLUDED__EVENT_HPP_*/
