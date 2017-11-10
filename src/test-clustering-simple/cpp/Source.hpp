#ifndef _INCLUDED_SOURCE_HPP
#define _INCLUDED_SOURCE_HPP

#include <systemc.h>

#include <systemoc/smoc_moc.hpp>

#include "smoc_synth_std_includes.hpp"

#include <cstdlib>

class Source
: public smoc_actor {
  typedef Source this_type;
public:
  smoc_port_out<void> smoc_port_out_0;
protected:
  unsigned long long SRC_ITER;
  unsigned long long SRC_ITERS;

  smoc_firing_state smoc_firing_state_10;
  smoc_firing_state smoc_firing_state_11;
public:
  Source(sc_module_name name)
    : smoc_actor(name, smoc_firing_state_10) {
    {
      char* init = std::getenv("SRC_ITER");
      SRC_ITER = init ? std::atoll(init) : 0;
    }
    {
      char* init = std::getenv("SRC_ITERS");
      SRC_ITERS = init ? std::atoll(init) : 1000000000;
    }
    smoc_firing_state_10
     = ((SMOC_VAR(this_type::SRC_ITER)<SMOC_VAR(this_type::SRC_ITERS))&&smoc_port_out_0(1,1)) >> SMOC_CALL(this_type::action) >> smoc_firing_state_10
     | (SMOC_VAR(this_type::SRC_ITER)>=SMOC_VAR(this_type::SRC_ITERS)) >> SMOC_CALL(this_type::action_1) >> smoc_firing_state_11
     ;
  }
protected:
  void action() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "Source::action [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "Source::action [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
  ++SRC_ITER;
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "Source::action [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "Source::action [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
  void action_1() {
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "Source::action_1 [enter] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "Source::action_1 [enter]" << std::endl;
  #endif // DEBUG_ACTIONS
  #if defined(DEBUG_ACTIONS) && defined(SYSTEMC_VERSION)
  std::cerr << "Source::action_1 [done] @" << sc_time_stamp() << std::endl;
  #elif defined(DEBUG_ACTIONS)
  std::cerr << "Source::action_1 [done]" << std::endl;
  #endif // DEBUG_ACTIONS
  }
};

#endif // _INCLUDED_SOURCE_HPP

