// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <vector>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "smoc_synth_std_includes.hpp"


using namespace std;

//template <class T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  double i;
  
  void src() {
#ifndef NDEBUG
    cout << "src: " << i << std::endl;
#endif
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = (out.getAvailableSpace() >= 1) >> (VAR(i) < 10000000) >> CALL(m_h_src::src) >> start;
  }
};


//template <class T> // actor type parameter T
class m_h_fir: public smoc_actor {
public:
  smoc_port_in<double>  input;
  smoc_port_out<double> output;
private:
  // taps parameter unmodifiable after actor instantiation
  /*const*/ vector<double> taps;
  // state information of the actor functionality
  vector<double>       data;
  
  // states of the firing rules state machine
  smoc_firing_state start;
  smoc_firing_state write;
  
  // action function for the firing rules state machine
  void dofir() {
    // action [a] ==> [b] 
    const double &a = input[0];
    
    // T b := collect(zero(), plus, combine(multiply, taps, data))
    double b = 0;
    for ( unsigned int i = 0; i < taps.size(); ++i )
      b += taps[i] * data[i];
    output[0] = b;
    // data := [a] + [data[i] : for Integer i in Integers(0, #taps-2)];
    data.pop_back(); data.insert(data.begin(), a);
  }
public:
  m_h_fir(
      sc_module_name name//,        // name of actor
      //const vector<double> &taps  // the taps are the coefficients, starting
                                    // with the one for the most recent data item 
  ) : smoc_actor( name, start )     //,
      //taps(taps),	            // make local copy of taps parameter
      //data(taps.size(), 0)        // initialize data with zero
  {
//  action [x] ==> [y]
    taps = initializer::get_fir_params();
    data = initializer::get_fir_data();
    start = (input.getAvailableTokens() >= 1) >>
            (output.getAvailableSpace() >= 1) >>
            CALL(m_h_fir::dofir)              >> start;
  }
};

//template <class T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  int i;
  
  void sink(void) {
#ifndef NDEBUG
    cout << "sink: " << in[0] << std::endl;
#endif
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in.getAvailableTokens() >= 1) >> CALL(m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src  src;
  m_h_fir  fir;
  m_h_sink sink;
public:
  /*static vector<double> gentaps() {
    vector<double> retval;

    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }*/
  
  m_h_top( sc_module_name name )
    : smoc_graph(name),
      src("src"),
      fir("fir"/*, gentaps()*/),
      sink("sink") {
    connectNodePorts(src.out   , fir.input);
    connectNodePorts(fir.output, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
#endif
  return 0;
}
