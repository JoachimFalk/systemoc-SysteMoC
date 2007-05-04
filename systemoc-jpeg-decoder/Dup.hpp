// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUP_HPP
#define INCLUDE_DUP_HPP

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
class Dup: public smoc_actor {

public:
  smoc_port_in<JpegChannel_t> in;
  smoc_port_out<JpegChannel_t> out1;
  smoc_port_out<JpegChannel_t> out2;


private:
  void copy_data() {
    JpegChannel_t input_data = in[0];
    out1[0] = input_data;
    out2[0] = input_data;
  }
  smoc_firing_state start;

public:
  Dup( sc_module_name name )
     : smoc_actor(name,start)
  {
    start = (in(1) && out1(1) && out2(1))
         >> CALL(Dup::copy_data) 
         >> start;
  }

};

#endif // INCLUDE_DUP_HPP
