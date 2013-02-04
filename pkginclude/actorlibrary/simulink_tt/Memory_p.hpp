/*  Library : Discrete
    Block : Memory
    Despcription : The Memory block outputs its input from the previous time step,
                   applyling a one integration step sample-and-hold to its input signal.
*/


#ifndef __INCLUDED__MEMORY_P__HPP__
#define __INCLUDED__MEMORY_P__HPP__


#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class Memory_p: public PeriodicActor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Memory_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq )
    : PeriodicActor(name, start, per, off, _eq),
      previous_in() {


    start = Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(Memory_p::process) >> start
      ;
  }

protected:

  T previous_in;

  void process() {
         this->resetEvent();
	 out[0] = previous_in;
	 previous_in = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__MEMORY_p__HPP__

