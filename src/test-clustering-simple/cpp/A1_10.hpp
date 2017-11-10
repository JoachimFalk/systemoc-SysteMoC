#ifndef _INCLUDED_A1_10_HPP
#define _INCLUDED_A1_10_HPP

#include <systemc.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

#include "smoc_synth_std_includes.hpp"

class A1_10
: public smoc_actor {
  typedef A1_10 this_type;
public:
  smoc_port_in<int> smoc_port_in_0;
  smoc_port_out<int> smoc_port_out_0;
  smoc_port_in<int> smoc_port_in_1;
  smoc_port_out<int> smoc_port_out_1;
  smoc_port_out<int> smoc_port_out_2;
protected:
  

  smoc_firing_state smoc_firing_state_9;
public:
  A1_10(sc_module_name name)
    : smoc_actor(name, smoc_firing_state_9) {
    
    smoc_firing_state_9
     = (smoc_port_out_2(5,5)&&(smoc_port_out_1(1,1)&&(smoc_port_in_1(5,5)&&(smoc_port_out_0(5,5)&&smoc_port_in_0(5,5))))) >> SMOC_CALL(this_type::action) >> smoc_firing_state_9
     ;
  }
protected:
  void action() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_10::action [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_10::action [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
    for (size_t i = 0; i < 5; ++i) {
      smoc_port_out_2[i] = 4711;
    }
    for (size_t i = 0; i < 1; ++i) {
      smoc_port_out_1[i] = 4711;
    }
    for (size_t i = 0; i < 5; ++i) {
      smoc_port_out_0[i] = 4711;
    }
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_10::action [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_10::action [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
};

#endif // _INCLUDED_A1_10_HPP

