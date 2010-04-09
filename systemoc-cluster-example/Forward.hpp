// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_FORWARD
#define _INCLUDED_FORWARD

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

class Forward
  : public smoc_actor {

public:

  smoc_port_in<int> in1;

  smoc_port_out<int> out1;

private:

  const unsigned long heat_time;

  //waste some CPU memory
  void heat(){
    usleep(heat_time);

    out1[0] = in1[0];
  }

  smoc_firing_state fsm_main;

public:
  Forward(sc_module_name name,
          unsigned long heat_time)
    : smoc_actor(name, fsm_main),
      heat_time(heat_time)
  {
    
    SMOC_REGISTER_CPARAM(heat_time);    

    fsm_main = 
      (in1(1) && out1(1)) >>
      CALL(Forward::heat) >>
      fsm_main;

  }


};

#endif
