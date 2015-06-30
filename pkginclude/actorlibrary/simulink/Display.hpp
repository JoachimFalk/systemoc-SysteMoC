/*  Library : Sinks
 Block : Scope
 Despcription : display outputs
 */

#ifndef __INCLUDED__DISPLAY__HPP__
#define __INCLUDED__DISPLAY__HPP__

#include <cstdlib>
#include <iostream>

#include <vector>
#include <CoSupport/Streams/stl_output_for_vector.hpp>

#include <systemoc/smoc_expr.hpp>
#include <systemoc/smoc_actor.hpp>

template<typename DATA_TYPE, int PORTS = 1>
class Display: public smoc_actor {
public:
  smoc_port_in<DATA_TYPE> in[PORTS];

  Display(sc_module_name name);
protected:

  void process();

  smoc_firing_state start;
};

template<typename DATA_TYPE, int PORTS>
Display<DATA_TYPE, PORTS>::Display(sc_module_name name)
  : smoc_actor(name, start)
{
  Expr::Ex<bool>::type eIn(in[0](1));

  for (int i = 1; i < PORTS; i++) {
    eIn = eIn && in[i](1);
  }
  start = eIn >> CALL(Display::process) >> start;
}

template<typename DATA_TYPE, int PORTS>
void Display<DATA_TYPE, PORTS>::process() {
  bool first = true;
  std::cout << this->name() << "> {";
  for (int allInputs = 0; allInputs < PORTS; allInputs++) {
    if (first) {
      std::cout << in[allInputs][0];
      first = false;
    } else
      std::cout << ", " << in[allInputs][0];
  }
  std::cout << "}" << std::endl;
}

#endif // __INCLUDED__DISPLAY__HPP__
