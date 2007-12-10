#include <cstdlib>
#include <iostream>

template<typename T, typename S, int INPUTS=1, int ReadTokenAtOnce=1>
class TerminalSink: public smoc_actor {
public:
  smoc_port_in<T>  inPuts[INPUTS];

protected:
  S message;
  void terminal() const{
    std::cout << message << "{";
    for(int allInputs = 0; allInputs < INPUTS; allInputs++){
      std::cout << " (";
      for(int i=0; i<ReadTokenAtOnce; i++){
	std::cout << inPuts[allInputs][i];
	if(i+1<ReadTokenAtOnce) std::cout << ", ";
      }
      std::cout << ")";
    }
    std::cout << " }"<< std::endl;
  }

  smoc_firing_state start;
public:
  TerminalSink(sc_module_name name, SMOC_ACTOR_CPARAM( S, message ))
    : smoc_actor(name, start), message(message) {

    Expr::Ex<bool >::type eIn(inPuts[0](ReadTokenAtOnce) );
    for(int allInputs = 1; allInputs < INPUTS; allInputs++){
      eIn = eIn && inPuts[allInputs](ReadTokenAtOnce);
    }

    start = eIn                    >> 
      CALL(TerminalSink::terminal) >> start
      ;
  }
};


