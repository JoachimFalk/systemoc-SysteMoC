/*  Library : IO adapter
    Block : Inport, Outport
    Despcription : IO adapter for subsystem's inport and outport
*/


#ifndef __INCLUDED__IOADAPTER__HPP__
#define __INCLUDED__IOADAPTER__HPP__



template<typename T>
 class IOAdapter: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  IOAdapter( sc_module_name name )
    : smoc_actor(name, start){


    start = in(1)              >>
      out(1)                   >>
      CALL(IOAdapter::process) >> start
      ;
  }

protected:

  void process() {
	 out[0] = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__IOADAPTER__HPP__

