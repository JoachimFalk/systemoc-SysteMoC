
#include <cstdlib>
#include <iostream>

template<typename Data, int ReadTokenAtOnce=1, int WriteTokenAtOnce=ReadTokenAtOnce>
class NtoMVec
  : public smoc_actor {
public:
  smoc_port_in<Data> *inPuts;
  smoc_port_out<Data> *outPuts;


private:
  int INPUTS;
  int OUTPUTS;

  void process()  {
    // outPuts[0][0] = 14;//inPuts[0][0];
    cout << name() << ".src("<< inPuts[0] <<")(1)" << endl;
    for(int allOutputs = 0; allOutputs < OUTPUTS; allOutputs++){
      for(int i=0; i<WriteTokenAtOnce; i++){
	outPuts[allOutputs][i]=inPuts[allOutputs%INPUTS][i%ReadTokenAtOnce];
      }
    }

  }

  smoc_firing_state main;
public:
  NtoMVec(sc_module_name name, int inp, int outp)
    : smoc_actor( name, main ) {
    INPUTS=inp;
    OUTPUTS=outp;
    inPuts = new smoc_port_in<Data>[INPUTS];
    outPuts = new smoc_port_out<Data>[OUTPUTS];

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
        CALL(NtoMVec::process)  >> main
      ;
  }
};

