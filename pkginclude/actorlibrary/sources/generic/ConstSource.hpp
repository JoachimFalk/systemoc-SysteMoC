#include <cstdlib>
#include <iostream>

template<typename T, int OUTPUTS=1, int WriteTokenAtOnce=1>
class ConstSource: public smoc_actor {
public:
  smoc_port_out<T> outPuts[OUTPUTS];
private:
  T   output;
  int limitCount;
  void src() {
    limitCount++;
    for(int allOutputs = 0; allOutputs < OUTPUTS; allOutputs++){
      for(int i=0; i<WriteTokenAtOnce; i++){
	outPuts[allOutputs][i] = output;
      }
    }
  }
  
  smoc_firing_state main;
public:
  ConstSource(sc_module_name name, SMOC_ACTOR_CPARAM( T, value ), SMOC_ACTOR_CPARAM( int, limit ))
    : smoc_actor(name, main), output( value ), limitCount( 0 ) {

    Expr::Ex<bool >::type eOut(outPuts[0](WriteTokenAtOnce) );
    for(int allOutputs = 1; allOutputs < OUTPUTS; allOutputs++){
      eOut = eOut && outPuts[allOutputs](WriteTokenAtOnce);
    }

    main = 
	( (VAR(limitCount)<limit) && eOut )  >>
	CALL(ConstSource::src)               >> main
      ;
  }
};
