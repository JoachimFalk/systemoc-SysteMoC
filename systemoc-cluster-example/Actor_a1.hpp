// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ACTOR_A1
#define _INCLUDED_ACTOR_A1

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

class Actor_a1
  : public smoc_actor {

public:

  smoc_port_in<int> in1;

  smoc_port_out<int> out1;
  smoc_port_out<int> out2;

private:

  const unsigned long heat_time;

  //waste some CPU memory
  void heat(){
    usleep(heat_time);

    out1[0] = in1[0];
    out2[0] = in1[0];
  }

  smoc_firing_state fsm_main;

public:
  Actor_a1(sc_core::sc_module_name name,
           unsigned long heat_time)
    : smoc_actor(name, fsm_main),
      heat_time(heat_time)
  {

    SMOC_REGISTER_CPARAM(heat_time);

    fsm_main = 
      (in1(1) && out1(1) && out2(1)) >>
      CALL(Actor_a1::heat) >>
      fsm_main;

  }


};

#endif
