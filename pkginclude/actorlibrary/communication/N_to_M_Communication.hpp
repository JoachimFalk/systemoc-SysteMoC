
#include <cstdlib>
#include <iostream>

template<typename Data, int INPUTS=1, int OUTPUTS=1, int ReadTokenAtOnce=1, int WriteTokenAtOnce=ReadTokenAtOnce>
class NtoM
  : public smoc_actor {
public:
  smoc_port_in<Data> inPuts[INPUTS];
  smoc_port_out<Data> outPuts[OUTPUTS];


private:
  void process()  {
    // outPuts[0][0] = 14;//inPuts[0][0];
    for(int allOutputs = 0; allOutputs < OUTPUTS; allOutputs++){
      for(int i=0; i<WriteTokenAtOnce; i++){
	outPuts[allOutputs][i]=inPuts[allOutputs%OUTPUTS][i%ReadTokenAtOnce];
      }
    }

  }

  smoc_firing_state main;
public:
  NtoM(sc_module_name name)
    : smoc_actor( name, main ) {

    Expr::Ex<bool >::type eIn(inPuts[0](ReadTokenAtOnce) );
    for(int allInputs = 1; allInputs < INPUTS; allInputs++){
      eIn = eIn && inPuts[allInputs](ReadTokenAtOnce);
    }

    Expr::Ex<bool >::type eOut(outPuts[0](WriteTokenAtOnce) );
    for(int allOutputs = 1; allOutputs < OUTPUTS; allOutputs++){
      eOut = eOut && outPuts[allOutputs](WriteTokenAtOnce);
    }

    main =
        eIn                  >>
        eOut                 >>
        CALL(NtoM::process)  >> main
      ;
  }
};

