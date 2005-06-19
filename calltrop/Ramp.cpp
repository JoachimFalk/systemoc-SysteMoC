template <typename T, typename X>
class Ramp: public smoc_actor {
public:
  smoc_port_in<X>  In
  smoc_port_out<T> Out;
private:
  T init;
  T step;
  T state;
  
  void action0() {
    Out[0] = state;
    state = state + step;
  }
  smoc_firing_state start;
public:
  Ramp(sc_module_name name, T init = 0, T step = 1)
    : smoc_actor(name, start), init(init), step(step) {
    start = (In.getAvailableTokens() >= 1) >>
            (Out.getAvailableSpace() >= 1) >>
            call(&Ramp::action0)           >> start;
  }
};


