template <typename T> // actor type parameter T
class FIR: public smoc_actor {
public:
  smoc_port_in<T>       input;
  smoc_port_out<T>      output;
private:
  const int             decimation;
  const int             decimationPhase;
  const int             interpolation;
  // taps parameter unmodifiable after actor instantiation
  const std::vector<T>  taps;
  
  T                     zero;
  int                   phaseLength;
  T                     data;     
  int                   mostRecent;
  T                     tapItem;
  T                     dataItem;
  
  void initialize() {
    zero = zero(taps[0]);
    phaseLength = (taps.size() / interpolation);
    if ((taps.size() % interpolation) != 0) {
      phaseLength = phaseLength + 1;
    }
    // create data array
    data = T(phaseLength);
    // initialize data array with zero
    for ( int i = 0; i <= phaseLength -1; i++) {
      data[i] = 0;
    }
    mostRecent = phaseLength;
  }
  
  // start state of the firing rules state machine
  smoc_firing_state start;
  
  // action function for the firing rules state machine
  void action0() {
    // action code here
  }
public:
  FIR(sc_module_name name, // name of actor
      int decimation,
      int decimationPhase,
      int interpolation,
      const std::vector<T> &taps
  ) : smoc_actor( name, start ),
      // make local copy of parameters
      decimation(decimation),
      decimationPhase(decimationPhase),
      interpolation(interpolation),
      taps(taps)
  {
    initialize();
//  action [a] repeat decimation ==> [b] repeat interpolation
    start = (input.getAvailableTokens() >= decimation) >>
            (output.getAvailableSpace() >= interpolation) >>
            call(&FIR::action0) >> start;
  }
};
