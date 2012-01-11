/*  Library : Discrete
    Block : Memory
    Despcription : The Memory block outputs its input from the previous time step,
                   applyling a one integration step sample-and-hold to its input signal.
*/


#ifndef __INCLUDED__MEMORY__HPP__
#define __INCLUDED__MEMORY__HPP__




template<typename T>
 class Memory: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Memory( sc_module_name name )
    : smoc_actor(name, start),
      previous_in() {


    start = in(1)              >>
      out(1)                   >>
      CALL(Memory::process) >> start
      ;
  }

protected:

  T previous_in;

  void process() {
	 out[0] = previous_in;
	 previous_in = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__MEMORY__HPP__

