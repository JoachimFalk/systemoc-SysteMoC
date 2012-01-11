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

  Memory_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq, T _init )
    : PeriodicActor(name, start, per, off, _eq), init(_init)
      //previous_in() 
  {


    start = Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(Memory_p::process) >> start
      ;
  }

protected:

  //T previous_in;

  void process() {
         this->resetEvent();
	 //std::cout << "Memory> storage: " << storage << " @ " << sc_time_stamp() << std::endl;
        if( init != (T)0.0 )
	{
	  out[0] = init;
	  init = (T)0.0;
	  //storage = in[0];
	}
 	else{
         out[0] = in[0];//storage;
	 //storage = 
	}
  }

  smoc_firing_state start;
  T storage;
  T init;
};

#endif // __INCLUDED__MEMORY_p__HPP__

