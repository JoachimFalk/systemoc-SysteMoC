// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUPLEX_VOTER_HPP
#define INCLUDE_DUPLEX_VOTER_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include "smoc_synth_std_includes.hpp"

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

///Duplication of a token
class DuplexVoter: public smoc_actor {

public:
  smoc_port_in<codeword_t> in1;
  smoc_port_in<codeword_t> in2;
  smoc_port_in<int> inTimeOut;
  smoc_port_out<codeword_t> out;
  smoc_port_out<int> outTimeOut;

  DuplexVoter( sc_module_name name );

private:
  void forward_data(); 
  void failed_action();

  smoc_firing_state start;
  smoc_firing_state failed;
  
};

#endif // INCLUDE_DUPLEX_VOTER_HPP
