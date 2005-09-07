// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

template <typename T>
class src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = (out.getAvailableSpace() >= 1) >>
            call(&m_h_src::src)            >> start;
  }
};

template <typename T>
class SimpleFIR: public smoc_actor {
  // code as shown before
}

template <typename T>
class sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in.getAvailableTokens() >= 1) >>
            call(&m_h_sink::sink)          >> start;
  }
};

class top: public smoc_graph {
protected:
  src<double>         s;
  SimpleFIR<double>   f;
  sink<double>        d;
  
  smoc_fifo<double>   f1;
  smoc_fifo<double>   f2;
public:
  static std::vector<double> gentaps() {
    std::vector<double> retval;
    
    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }
  
  top( sc_module_name name )
    : smoc_graph(name),
      s("s"),
      f("f", gentaps()),
      d("d") {
    s.out(f1); f.input(f1); // s.out -> f.input
    f.output(f2); d.in(f2); // f.output -> d.in
  }
};

int sc_main (int argc, char **argv) {
  smoc_top<top> t("t");
  
  sc_start(-1);
  return 0;
}
