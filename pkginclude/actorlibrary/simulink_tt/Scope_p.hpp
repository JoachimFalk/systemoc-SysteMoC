/*  Library : Sinks
    Block : Scope
    Despcription : display outputs
*/

#ifndef __INCLUDED__SCOPE_P__HPP__
#define __INCLUDED__SCOPE_P__HPP__


#include <stdio.h>

#include <errno.h>
#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_expr.hpp>
//#include <actorlibrary/tt/TT.hpp>

template<typename DATA_TYPE, typename S, int PORTS=1>
 class Scope_p: public smoc_periodic_actor {
public:
  smoc_port_in<DATA_TYPE>   in[PORTS];
  

  
  Scope_p( sc_module_name name, sc_time per, sc_time off, const char * message )
    : smoc_periodic_actor(name, start, per, off), message(message), _init(), step(){

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = //Expr::till( this->getEvent() )  >>  
      eIn                    >> 
      CALL(Scope_p::process) >> start
      ;
  }
protected:

  const char *message; 
  FILE *fp;
  int _init;
  int step;

  void process() {
    //this->resetEvent();
    
	//std::cout << "Scope:<" << this->name() << "> " << " @ " << sc_time_stamp() << message << " { ";
	
if( message != "" ){
	fp = fopen(  message, "a");
	if (fp == NULL) {
		fprintf(stderr, "cannot open %s for writing %s", this->name(), strerror(errno));
		std::cout << "cannot open %s for writing:" << step << "\n\r"; 
		exit(EXIT_FAILURE);
	}

	
	//fprintf(fp, "P2\n");
	for( int allInputs = 0; allInputs < PORTS; allInputs++ ){
		fprintf(fp, "%d %f\n", step++, in[allInputs][0]  );
		//fprintf(fp, "%d\n", step++);

	  
	}
	_init = 1;	
	fclose(fp);
}
else
{


        
	for( int allInputs = 0; allInputs < PORTS; allInputs++ ){
	   //std::cout << in[allInputs][0] <<", "; 
		//fprintf(fp, "P2\n");
		//fprintf(fp, "%d\n", in[allInputs][0]  );
	  
	}
	
	//std::cout<< " }" << std::endl;

}
  }

  smoc_firing_state start;

};


#endif // __INCLUDED__SCOPE_P__HPP__
