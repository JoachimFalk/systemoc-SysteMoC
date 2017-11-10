#ifndef _INCLUDED_A1_8_HPP
#define _INCLUDED_A1_8_HPP

#include <systemc.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

#include "smoc_synth_std_includes.hpp"



class A1_8
: public smoc_actor {
  typedef A1_8 this_type;
public:
  smoc_port_in<int> smoc_port_in_0;
  smoc_port_in<int> smoc_port_in_1;
  smoc_port_in<int> smoc_port_in_2;
  smoc_port_out<int> smoc_port_out_0;
  smoc_port_out<int> smoc_port_out_1;
  smoc_port_in<int> smoc_port_in_3;
protected:
  

  smoc_firing_state smoc_firing_state_7;
public:
  A1_8(sc_module_name name)
    : smoc_actor(name, smoc_firing_state_7) {
    
    smoc_firing_state_7
     = (smoc_port_in_3(1,1)&&(smoc_port_out_1(1,1)&&(smoc_port_out_0(1,1)&&(smoc_port_in_2(9,9)&&(smoc_port_in_1(1,1)&&smoc_port_in_0(1,1)))))) >> SMOC_CALL(this_type::action) >> smoc_firing_state_7
     ;
  }
protected:
  void action() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_8::action [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_8::action [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_1[i] = 4711;
    }
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_0[i] = 4711;
    }
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_8::action [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_8::action [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
};

#endif // _INCLUDED_A1_8_HPP

