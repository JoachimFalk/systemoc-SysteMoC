/*  Library : Math Operations
    Block : Gain
    Despcription : Multiply input by constant
		   

*/


#ifndef __INCLUDED__GAIN_HPP__
#define __INCLUDED__GAIN_HPP__




template<typename DATA_TYPE>
 class Gain: public smoc_actor {
public:
  smoc_port_in<DATA_TYPE>  in;
  smoc_port_out<DATA_TYPE>  out;

  Gain( sc_module_name name, DATA_TYPE gain )
    : smoc_actor(name, start), gain(gain)
  {
    SMOC_REGISTER_CPARAM(gain);

    start = in(1) >> out(1)     >>
      CALL(Gain::process) >> start
      ;
  }

protected:
  DATA_TYPE gain;

  void process() {
      out[0] = in[0] * gain;
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__Gain_HPP__
