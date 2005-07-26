
#include <callib.hpp>

class m_Upsample: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_out<int> O;
private:
  const int factor;
  int       state;
  int       mem;
  
  void action0() {
    mem = I[0];
    state = state + 1;
    O[0] = I[0];
  }
  void action1() {
    state = state + 1;
    if (state == factor) {
      state = 0;
    }
    O[0] = mem;
  }
  
  smoc_firing_state start;

public:
  m_Upsample(sc_module_name name, int factor)
    : smoc_actor(name, start),
      factor(factor), state(0) {
    start = (I.getAvailableTokens() >= 1 &&
             var(state) == 0)                 >>
            (O.getAvailableSpace() >= 1)      >>
            call(&m_Upsample::action0)        >> start
          | (var(state)  > 0)                 >>
            (O.getAvailableSpace() >= 1)      >>
            call(&m_Upsample::action1)        >> start;
  }
};
