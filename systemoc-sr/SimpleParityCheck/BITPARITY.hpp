#ifndef __INCLUDE_BITPARITY_HPP
#define __INCLUDE_BITPARITY_HPP

#include <systemoc/smoc_moc.hpp>
#include <math.h>
#include "pkg_datatypes.hpp"

/****
MTT: (BITPARITY)
From	To	[BITINPUT](2)	[BITCOUNT = (WORDSIZE-1)](1)	[CLK](0)	
EVEN	ODD	T	F	@T	
ODD	EVEN	T	F	@T	
ODD	EVEN		T	@T	
*/
class BITPARITY: public smoc_actor {
public:
  //input ports
  smoc_port_in<bool>                    BITINPUT_inPort;
  smoc_port_in<int>                     BITCOUNT_inPort;
  smoc_port_in<bool>                    CLK_inPort;
  smoc_port_in<bool>                    CLK_hist_inPort;

  //output ports
  smoc_port_out<PARITY>                 BITPARITY_outPort;

//
  BITPARITY(sc_module_name name)
    : smoc_actor(name, s_init){

    s_init = BITPARITY_outPort(1)                                          >>
        CALL(BITPARITY::func_EVEN)                                         >>
        s_EVEN    ;

    s_EVEN = (
          BITINPUT_inPort(1)                                               &&
          BITCOUNT_inPort(1)                                               &&
          CLK_inPort(1)                                                    &&
          CLK_hist_inPort(1)                                               &&
          GUARD(BITPARITY::c2)                                             &&
          !GUARD(BITPARITY::c1)                                            &&
          GUARD(BITPARITY::c0)                                             &&
          GUARD(BITPARITY::c0_event) )                                     >>
        BITPARITY_outPort(1)                                               >>
        CALL(BITPARITY::func_ODD)                                          >>
        s_ODD
    ;

    s_ODD = (
          BITINPUT_inPort(1)                                               &&
          BITCOUNT_inPort(1)                                               &&
          CLK_inPort(1)                                                    &&
          CLK_hist_inPort(1)                                               &&
          GUARD(BITPARITY::c2)                                             &&
          !GUARD(BITPARITY::c1)                                            &&
          GUARD(BITPARITY::c0)                                             &&
          GUARD(BITPARITY::c0_event) )                                     >>
        BITPARITY_outPort(1)                                               >>
        CALL(BITPARITY::func_EVEN)                                         >>
        s_EVEN

      |(
          BITCOUNT_inPort(1)                                               &&
          CLK_inPort(1)                                                    &&
          CLK_hist_inPort(1)                                               &&
          GUARD(BITPARITY::c1)                                             &&
          GUARD(BITPARITY::c0)                                             &&
          GUARD(BITPARITY::c0_event) )                                     >>
        BITPARITY_outPort(1)                                               >>
        CALL(BITPARITY::func_EVEN)                                         >>
        s_EVEN
    ;
  }
private:
  // BITINPUT
  bool c2() const{
    bool BITINPUT = BITINPUT_inPort[0];
    return (BITINPUT);
  }

  // BITCOUNT = (WORDSIZE-1)
  bool c1() const{
    int BITCOUNT = BITCOUNT_inPort[0];
    return (BITCOUNT == (WORDSIZE-1));
  }

  // CLK
  bool c0() const{
    bool CLK = CLK_inPort[0];
    return (CLK);
  }

  bool c0_event() const{
    bool CLK = CLK_hist_inPort[0];
    bool actual  = this->c0();
    bool hist    = CLK;
    return (hist != actual);
  }

  void func_ODD(){
    BITPARITY_outPort[0] = ODD;
  }

  void func_EVEN(){
    BITPARITY_outPort[0] = EVEN;
  }

  smoc_firing_state s_init;
  smoc_firing_state s_ODD;
  smoc_firing_state s_EVEN;
};

#endif //__INCLUDE_BITPARITY_HPP
