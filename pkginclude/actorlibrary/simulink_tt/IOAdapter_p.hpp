/*  Library : IO adapter
    Block : Inport, Outport
    Despcription : IO adapter for subsystem's inport and outport
*/


#ifndef __INCLUDED__IOADAPTER_P__HPP__
#define __INCLUDED__IOADAPTER_P__HPP__


#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>


template<typename T>
 class IOAdapter_p: public PeriodicActor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  IOAdapter_p( sc_module_name name, sc_time per, sc_time off, EventQueue* eventQueue )
    : PeriodicActor(name, start, per, off, eventQueue ){


    start = in(1)              >>
      out(1)                   >>
      CALL(IOAdapter_p::process) >> start
      ;
  }

protected:

  void process() {
	 out[0] = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__IOADAPTER_P__HPP__

