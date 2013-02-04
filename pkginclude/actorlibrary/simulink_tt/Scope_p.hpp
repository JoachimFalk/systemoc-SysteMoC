/*  Library : Sinks
    Block : Scope
    Despcription : display outputs
*/

#ifndef __INCLUDED__SCOPE_P__HPP__
#define __INCLUDED__SCOPE_P__HPP__

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>

template<typename DATA_TYPE, typename S, int PORTS=1>
 class Scope_p: public PeriodicActor {
public:
  smoc_port_in<DATA_TYPE>   in[PORTS];
  

  
  Scope_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq, S message )
    : PeriodicActor(name, start, per, off, _eq), message(message){

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = Expr::till( this->getEvent() )  >>
      eIn                    >> 
      CALL(Scope_p::process) >> start
      ;
  }
protected:

  S message; 


  void process() {
    this->resetEvent();
    
	std::cout << "Scope:<" << this->name() << "> " << message << " {";
        for( int allInputs = 0; allInputs < PORTS; allInputs++ ){
	  std::cout << in[allInputs][0] <<", "; 
	  
	}
	std::cout<< " }" << std::endl;
  }

  smoc_firing_state start;

};


#endif // __INCLUDED__SCOPE_P__HPP__
