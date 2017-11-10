#ifndef _INCLUDED_A1_5_HPP
#define _INCLUDED_A1_5_HPP

#include <systemc.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

#include "smoc_synth_std_includes.hpp"



class A1_5
: public smoc_actor {
  typedef A1_5 this_type;
public:
  smoc_port_out<int> smoc_port_out_0;
  smoc_port_out<int> smoc_port_out_1;
  smoc_port_out<int> smoc_port_out_2;
  smoc_port_in<int> smoc_port_in_0;
  smoc_port_out<int> smoc_port_out_3;
  smoc_port_in<int> smoc_port_in_1;
protected:
  

  smoc_firing_state smoc_firing_state_4;
public:
  A1_5(sc_module_name name)
    : smoc_actor(name, smoc_firing_state_4) {
    
    smoc_firing_state_4
     = (smoc_port_in_1(1,1)&&(smoc_port_out_3(1,1)&&(smoc_port_in_0(1,1)&&(smoc_port_out_2(1,1)&&(smoc_port_out_1(1,1)&&smoc_port_out_0(1,1)))))) >> SMOC_CALL(this_type::action) >> smoc_firing_state_4
     ;
  }
protected:
  void action() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_5::action [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_5::action [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_2[i] = 4711;
    }
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_1[i] = 4711;
    }
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_0[i] = 4711;
    }
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_3[i] = 4711;
    }
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_5::action [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_5::action [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
};

#endif // _INCLUDED_A1_5_HPP

