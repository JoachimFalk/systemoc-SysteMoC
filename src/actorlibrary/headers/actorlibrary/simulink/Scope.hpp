/*  Library : Sinks
    Block : Scope
    Despcription : display outputs
*/

#ifndef __INCLUDED__SCOPE__HPP__
#define __INCLUDED__SCOPE__HPP__

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_expr.hpp>

template<typename DATA_TYPE, typename S, int PORTS=1>
 class Scope: public smoc_actor {
public:
  smoc_port_in<DATA_TYPE>   in[PORTS];
  

  
  Scope( sc_module_name name, S message )
    : smoc_actor(name, start), message(message){

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn                    >> 
      CALL(Scope::process) >> start
      ;
  }
protected:

  S message; 


  void process() {

	std::cout << this->name() << "> " << message << " {";
        for( int allInputs = 0; allInputs < PORTS; allInputs++ ){
	  std::cout<<" (" << in[allInputs][0] <<", "; 
	  
	}
	std::cout<< " }" << std::endl;
  }

  smoc_firing_state start;

};


#endif // __INCLUDED__SCOPE__HPP__
