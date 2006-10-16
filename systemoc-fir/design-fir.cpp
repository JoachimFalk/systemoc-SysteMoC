// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

template <typename T>
class m_h_src: public smoc_actor {
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
  m_h_src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = out(1) >> call(&m_h_src::src) >> start;
  }
};

template <typename T> // actor type parameter T
class m_h_fir: public smoc_actor {
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
  smoc_firing_state write;
  
  // action function for the firing rules state machine
  void dofir() {
    // action [a] ==> [b] 
    const T &a = input[0];
    
    // T b := collect(zero(), plus, combine(multiply, taps, data))
    T b = 0;
    for ( unsigned int i = 0; i < taps.size(); ++i )
      b += taps[i] * data[i];
    // data := [a] + [data[i] : for Integer i in Integers(0, #taps-2)];
    data.pop_back(); data.insert(data.begin(), a);
  }
public:
  m_h_fir(
      sc_module_name name,        // name of actor
      const std::vector<T> &taps  // the taps are the coefficients, starting
                                  // with the one for the most recent data item 
  ) : smoc_actor( name, start ),
      taps(taps),                 // make local copy of taps parameter
      data(taps.size(), 0)        // initialize data with zero
  {
//  action [x] ==> [y]

    start = input(1) >>
            output(1) >>
            call(&m_h_fir::dofir)             >> start;
  }
};

template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> call(&m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<double>  src;
  m_h_fir<double>  fir;
  m_h_sink<double> sink;
public:
  static std::vector<double> gentaps() {
    std::vector<double> retval;

    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }
  
  m_h_top( sc_module_name name )
    : smoc_graph(name),
      src("src"),
      fir("fir", gentaps()),
      sink("sink") {
    connectNodePorts(src.out   , fir.input);
    connectNodePorts(fir.output, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
