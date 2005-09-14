template <typename T>
class UpSample: public smoc_actor {
public:
  smoc_port_in<T>  input;
  smoc_port_out<T> output;
private:
  int phase;
  int factor;
  
  void action0() {
    for (int i = 0; i < factor; i++)
      output[i] = i == phase ? input[0] : 0;
  }
  smoc_firing_state start;
public:
  UpSample(sc_module_name name, int factor = 2, int phase = factor - 1 )
    : smoc_actor(name, start), phase(phase), factor(factor) {
    start = (input.getAvailableTokens() >= 1     ) >>
            (out.getAvailableSpace()    >= factor) >>
            call(&UpSample::action0)               >> start;
  }
};
