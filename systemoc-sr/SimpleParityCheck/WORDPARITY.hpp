#ifndef __INCLUDE_WORDPARITY_HPP
#define __INCLUDE_WORDPARITY_HPP

#include <systemoc/smoc_moc.hpp>
#include <math.h>
#include "pkg_datatypes.hpp"

/****
DTT: (WORDPARITY)
Value	[BITCOUNT=WORDSIZE-1](0)	
WORDPARITY	F	
BITPARITY	T	
*/
class WORDPARITY: public smoc_actor {
public:
  //input ports
  smoc_port_in<int>                     BITCOUNT_inPort;
  smoc_port_in<PARITY>                  WORDPARITY_inPort;
  smoc_port_in<PARITY>                  BITPARITY_inPort;

  //output ports
  smoc_port_out<PARITY>                 WORDPARITY_outPort;

//
  WORDPARITY(sc_module_name name)
    : smoc_actor(name, s_init){

    s_init = WORDPARITY_outPort(1)                                         >>
        CALL(WORDPARITY::func_writeInitial)                                >>
        s_main;

    s_main = (
          BITCOUNT_inPort(1)                                               &&
          WORDPARITY_inPort(1)                                             &&
          !GUARD(WORDPARITY::c0) )                                         >>
        WORDPARITY_outPort(1)                                              >>
        CALL(WORDPARITY::func_write)                                       >>
        s_main
      |(
          BITCOUNT_inPort(1)                                               &&
          BITPARITY_inPort(1)                                              &&
          GUARD(WORDPARITY::c0) )                                          >>
        WORDPARITY_outPort(1)                                              >>
        CALL(WORDPARITY::func_write_1)                                     >>
        s_main
    ;
  }
private:
  // BITCOUNT=WORDSIZE-1
  bool c0() const{
    int BITCOUNT = BITCOUNT_inPort[0];
    return (BITCOUNT==WORDSIZE-1);
  }

  void func_write_1(){
    int BITCOUNT = BITCOUNT_inPort[0];
    //PARITY WORDPARITY = WORDPARITY_inPort[0]; //FIXME!!
    PARITY BITPARITY = BITPARITY_inPort[0];
    WORDPARITY_outPort[0] = (BITPARITY);
  }

  void func_write(){
    int BITCOUNT = BITCOUNT_inPort[0];
    PARITY WORDPARITY = WORDPARITY_inPort[0];
    PARITY BITPARITY = BITPARITY_inPort[0];
    WORDPARITY_outPort[0] = (WORDPARITY);
  }

  void func_writeInitial(){
    WORDPARITY_outPort[0] = EVEN;
  }

  smoc_firing_state s_init;
  smoc_firing_state s_main;
};

#endif //__INCLUDE_WORDPARITY_HPP
