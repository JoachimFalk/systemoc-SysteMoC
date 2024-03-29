/*  Library : Math Operations
    Block : Sum, Add, Subtract
    Despcription : Add or subtract inputs
*/


#ifndef __INCLUDED__CONSTANT_P_HPP__
#define __INCLUDED__CONSTANT_P_HPP__

#include <cstdlib>
#include <iostream>


#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>





template<typename DATA_TYPE>
 class Constant_p: public smoc_periodic_actor {
public:
  smoc_port_out<DATA_TYPE>  out;
  static DATA_TYPE ste;


  Constant_p( sc_module_name name, sc_time per, sc_time off, DATA_TYPE constValue )
    : smoc_periodic_actor(name, start, per, off), constValue(constValue) {

    start = //Expr::till( this->getEvent() )  >>
      out(1)     >>
      CALL(Constant_p::process) >> start
      ;
  }



protected:
  DATA_TYPE constValue;

  void process() {
    //this->resetEvent();
      //std::cout << "Constant> send: " << constValue << " @ " << sc_time_stamp() << std::endl;
     
      out[0] = constValue;

      //FIXME: just for testing!!!
     // constValue += 0.1;
      
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__CONSTANT_p__HPP__
