/*  Library : Math Operations
    Block : Gain
    Despcription : Multiply input by constant
		   

*/


#ifndef __INCLUDED__GAIN_P__HPP__
#define __INCLUDED__GAIN_P__HPP__

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>


template<typename DATA_TYPE>
 class Gain_p: public smoc_periodic_actor {
public:
  smoc_port_out<DATA_TYPE>  out;
  smoc_port_in<DATA_TYPE>  in;

  Gain_p( sc_module_name name, sc_time per, sc_time off, DATA_TYPE gain )
    : smoc_periodic_actor(name, start, per, off), gain(gain) {

    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(Gain_p::process) >> start
      ;
  }

protected:
  DATA_TYPE gain;

  void process() {
    //this->resetEvent();
	  //std::cout << this->name() << " : " << in[0] << " @ " << sc_time_stamp() << std::endl;
	  DATA_TYPE income = in[0]; 
	  DATA_TYPE temp = income * gain;
	  //cout << name() << " received: " << temp << " at [" << sc_time_stamp() << "]\n";
          out[0] = temp;
 	  //std::cout << this->name() << " : Action finished" << std::endl;
          //printf("result: %.6f, gain %.6f\n", in[0] * gain, gain );		
	      	#ifdef EnablePrint
	std::cout << sc_time_stamp() << " " <<  name() << " " << " out: " << temp << " in: " << income << "\n";
	#endif
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__Gain_P__HPP__
