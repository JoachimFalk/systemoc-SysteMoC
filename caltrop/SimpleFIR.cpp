template <typename T> // actor type parameter T
class SimpleFIR: public smoc_actor {
public:
  smoc_port_in<T>  input;
  smoc_port_out<T> output;
private:
  // taps parameter unmodifiable after actor instantiation
  const std::vector<T> taps;
  // state information of the actor functionality
  std::vector<T>       data;
  
  // states of the firing rules state machine
  smoc_firing_state start;
  
  // action function for the firing rules state machine
  void action0() {
    // action [a] ==> [b] 
    T &a(input[0]);
    T &b(output[0]);
    
    // T b := collect(zero(), plus, combine(multiply, taps, data))
    b = 0;
    for ( unsigned int i = 0; i < taps.size(); ++i )
      b += taps[i] * data[i];
    // data := [a] + [data[i] : for Integer i in Integers(0, #taps-2)];
    data.pop_back(); data.insert(data.begin(), a);
  }
public:
  SimpleFIR(
      sc_module_name name,        // name of actor
      const std::vector<T> &taps  // the taps are the coefficients, starting
                                  // with the one for the most recent data item 
  ) : smoc_actor( name, start ),
      taps(taps),                 // make local copy of taps parameter
      data(taps.size(), 0)        // initialize data with zero
  {
//  action [x] ==> [y]
    start = (input.getAvailableTokens() >= 1) >>
            (output.getAvailableSpace() >= 1) >>
            call(&SimpleFIR::action0) >> start;
  }
};
