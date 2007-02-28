// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_port.hpp>


template<typename T, typename S>
class TerminalSink: public smoc_actor {
public:
  smoc_port_in<T>  in;

protected:
  S message;
  void terminal() const{
    cout << name() << ".terminal()" << endl;
    std::cout << message << in[0] << std::endl;
  }

  smoc_firing_state start;
public:
  TerminalSink(sc_module_name name, SMOC_ACTOR_CPARAM( S, message ))
    : smoc_actor(name, start), message(message) {
    start = in(1) >> CALL(TerminalSink::terminal) >> start
      ;
  }
};

