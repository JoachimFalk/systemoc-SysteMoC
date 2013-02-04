/*  Library : Math Operations
    Block : Sum, Add, Subtract
    Despcription : Add or subtract inputs
*/


#ifndef __INCLUDED__CONSTANT_P_HPP__
#define __INCLUDED__CONSTANT_P_HPP__

#include <cstdlib>
#include <iostream>


#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>




template<typename DATA_TYPE>
 class Constant_p: public PeriodicActor {
public:
  smoc_port_out<DATA_TYPE>  out;

  Constant_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq, DATA_TYPE constValue )
    : PeriodicActor(name, start, per, off, _eq), constValue(constValue) {

    start = Expr::till( this->getEvent() )  >>
      out(1)     >>
      CALL(Constant_p::process) >> start
      ;
  }

protected:
  DATA_TYPE constValue;

  void process() {
    this->resetEvent();
      //std::cout << "Constant> I am alive!" << std::endl;
      out[0] = constValue;
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__CONSTANT_p__HPP__
