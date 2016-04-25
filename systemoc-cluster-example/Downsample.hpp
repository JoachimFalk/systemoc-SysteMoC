// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_DOWNSAMPLE
#define _INCLUDED_DOWNSAMPLE

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

class Downsample
  : public smoc_actor {

public:

  smoc_port_in<int> in1;
  smoc_port_out<int> out1;

private:

  const unsigned int factor;

  //action
  void do_downsample(){

    int value = in1[0];
    out1[0] = value;
  }

  smoc_firing_state fsm_main;

public:
  Downsample(sc_core::sc_module_name name, unsigned int factor)
    : smoc_actor(name, fsm_main),
      factor(factor)
  {

    SMOC_REGISTER_CPARAM(factor);


    fsm_main = 
      (in1(factor) && out1(1)) >>
      CALL(Downsample::do_downsample) >>
      fsm_main;

  }


};

#endif
