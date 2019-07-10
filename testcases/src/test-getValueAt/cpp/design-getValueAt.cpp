// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class m_source: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  size_t i;

  void process() {
    std::cout << name() << ": generating " << i << std::endl;
    out[0] = i++;
  }

  smoc_firing_state start;
public:
  m_source( sc_core::sc_module_name name, size_t iter)
    : smoc_actor( name, start ), i(0) {
    start =
        (out(1) && SMOC_VAR(i) < iter) >>
        SMOC_CALL(m_source::process) >> start
      ;
  }
};

class m_sink: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
  void process() {
    std::cout << name() << ": receiving " << in[0] << std::endl;
  }

  smoc_firing_state start;
public:
  m_sink( sc_core::sc_module_name name )
    :smoc_actor( name, start ) {
    start =
        (in(1) && in.getValueAt(0) == 0) >>
        SMOC_CALL(m_sink::process) >> start
      |
        in(1) >>
        SMOC_CALL(m_sink::process) >> start
      ;
  }
};

class m_top: public smoc_graph {
public:
  m_top( sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name)
  {
    m_source &src1 = registerNode(new m_source("src1", iter));
    m_sink   &sink = registerNode(new m_sink("sink"));
    connectNodePorts(src1.out, sink.in, smoc_fifo<int>(2));
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);

  smoc_top_moc<m_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
