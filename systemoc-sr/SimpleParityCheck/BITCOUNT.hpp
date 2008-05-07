#ifndef __INCLUDE_BITCOUNT_HPP
#define __INCLUDE_BITCOUNT_HPP

#include <systemoc/smoc_moc.hpp>
#include <math.h>
#include "pkg_datatypes.hpp"

/****
DTT: (BITCOUNT)
Value	[BITCOUNT = (WORDSIZE-1)](1)	[CLK](0)	
0	T	@T	
BITCOUNT+1	F	@T	
*/
class BITCOUNT: public smoc_actor {
public:
  //input ports
  smoc_port_in<int>                     BITCOUNT_inPort;
  smoc_port_in<bool>                    CLK_inPort;
  smoc_port_in<bool>                    CLK_hist_inPort;

  //output ports
  smoc_port_out<int>                    BITCOUNT_outPort;

//
  BITCOUNT(sc_module_name name)
    : smoc_actor(name, s_init){

    s_init = BITCOUNT_outPort(1)                                           >>
        CALL(BITCOUNT::func_writeInitial)                                  >>
        s_main;

    s_main = (
          BITCOUNT_inPort(1)                                               &&
          CLK_inPort(1)                                                    &&
          CLK_hist_inPort(1)                                               &&
          GUARD(BITCOUNT::c1)                                              &&
          GUARD(BITCOUNT::c0)                                              &&
          GUARD(BITCOUNT::c0_event) )                                      >>
        BITCOUNT_outPort(1)                                                >>
        CALL(BITCOUNT::func_write)                                         >>
        s_main
      |(
          BITCOUNT_inPort(1)                                               &&
          CLK_inPort(1)                                                    &&
          CLK_hist_inPort(1)                                               &&
          !GUARD(BITCOUNT::c1)                                             &&
          GUARD(BITCOUNT::c0)                                              &&
          GUARD(BITCOUNT::c0_event) )                                      >>
        BITCOUNT_outPort(1)                                                >>
        CALL(BITCOUNT::func_write_1)                                       >>
        s_main
    ;
  }
private:
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

  void func_write_1(){
    int BITCOUNT = BITCOUNT_inPort[0];
    bool CLK = CLK_inPort[0];
    BITCOUNT_outPort[0] = (BITCOUNT+1);
  }

  void func_write(){
    int BITCOUNT = BITCOUNT_inPort[0];
    bool CLK = CLK_inPort[0];
    BITCOUNT_outPort[0] = (0);
  }

  void func_writeInitial(){
    BITCOUNT_outPort[0] = 0;
  }

  smoc_firing_state s_init;
  smoc_firing_state s_main;
};

#endif //__INCLUDE_BITCOUNT_HPP
