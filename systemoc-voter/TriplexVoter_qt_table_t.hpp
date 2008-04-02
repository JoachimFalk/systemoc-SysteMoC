// vim: set sw=2 ts=8:

#ifndef INCLUDE_TRIPLEX_VOTER_HPP
#define INCLUDE_TRIPLEX_VOTER_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "channels.hpp"

///Duplication of a token
class TriplexVoter: public smoc_actor {

public:
  smoc_port_in<qt_table_t> in1;
  smoc_port_in<qt_table_t> in2;
  smoc_port_in<qt_table_t> in3;
  smoc_port_in<int> inTimeOut;
  smoc_port_out<qt_table_t> out;
  smoc_port_out<int> outTimeOut;

  TriplexVoter( sc_module_name name );

private:
  void forward_in1(); 
  void forward_in2(); 
  void failed_action();

  smoc_firing_state start;
  smoc_firing_state timeOut;
  smoc_firing_state failed;
  
};

#endif // INCLUDE_TRIPLEX_VOTER_HPP
