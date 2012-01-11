/*  Library : Math Operations
    Block : Gain
    Despcription : Multiply input by constant
		   

*/


#ifndef __INCLUDED__GAIN_P__HPP__
#define __INCLUDED__GAIN_P__HPP__

#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>


template<typename DATA_TYPE>
 class Gain_p: public PeriodicActor {
public:
  smoc_port_out<DATA_TYPE>  out;
  smoc_port_in<DATA_TYPE>  in;

  Gain_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq, DATA_TYPE gain )
    : PeriodicActor(name, start, per, off, _eq), gain(gain) {

    start = Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(Gain_p::process) >> start
      ;
  }

protected:
  DATA_TYPE gain;

  void process() {
    this->resetEvent();
	  //std::cout << this->name() << " : " << in[0] << " @ " << sc_time_stamp() << std::endl;
          out[0] = in[0] * gain;
 	  //std::cout << this->name() << " : Action finished" << std::endl;
          //printf("result: %.6f, gain %.6f\n", in[0] * gain, gain );		
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__Gain_P__HPP__
