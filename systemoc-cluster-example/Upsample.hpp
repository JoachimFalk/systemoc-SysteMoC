// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_UPSAMPLE
#define _INCLUDED_UPSAMPLE

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

class Upsample
  : public smoc_actor {

public:

  smoc_port_in<int> in1;
  smoc_port_out<int> out1;

private:

  const unsigned int factor;

  //action
  void do_upsample(){

    int value = in1[0];

    for(unsigned int i = 0; i<factor; i++)
      out1[i] = value;
  }

  smoc_firing_state fsm_main;

public:
  Upsample(sc_module_name name, unsigned int factor)
    : smoc_actor(name, fsm_main),
      factor(factor)
  {

    SMOC_REGISTER_CPARAM(factor);

    fsm_main = 
      (in1(1) && out1(factor)) >>
      CALL(Upsample::do_upsample) >>
      fsm_main;

  }


};

#endif
