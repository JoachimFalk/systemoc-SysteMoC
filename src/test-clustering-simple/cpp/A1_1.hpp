#ifndef _INCLUDED_A1_1_HPP
#define _INCLUDED_A1_1_HPP

#include <systemc.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

#include "smoc_synth_std_includes.hpp"



class A1_1
: public smoc_actor {
  typedef A1_1 this_type;
public:
  smoc_port_out<int> smoc_port_out_0;
  smoc_port_in<void> smoc_port_in_0;
protected:
  

  smoc_firing_state smoc_firing_state_0;
public:
  A1_1(sc_module_name name)
    : smoc_actor(name, smoc_firing_state_0) {
    
    smoc_firing_state_0
     = (smoc_port_in_0(1,1)&&smoc_port_out_0(6,6)) >> SMOC_CALL(this_type::action) >> smoc_firing_state_0
     ;
  }
protected:
  void action() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_1::action [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_1::action [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
    for (size_t i = 0; i < 6; ++i) {
      smoc_port_out_0[i] = 4711;
    }
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "A1_1::action [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "A1_1::action [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
};

#endif // _INCLUDED_A1_1_HPP

