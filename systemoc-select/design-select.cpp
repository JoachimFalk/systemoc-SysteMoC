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
    std::cout << name() << ": " << i << std::endl;
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

class m_h_srcbool: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  bool i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i ? 1 : 0; i = !i;
  }
  smoc_firing_state start;
public:
  m_h_srcbool(sc_module_name name)
    : smoc_actor(name, start),
      i(false) {
    start = out(1) >> call(&m_h_srcbool::src) >> start;
  }
};

template <typename T>
class Select: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { 
    std::cout << "action0" << std::endl;
    Output[0] = Data0[0];
  }
  void action1() {
    std::cout << "action1" << std::endl;
    Output[0] = Data1[0];
  }
  smoc_firing_state start;
  smoc_firing_state atChannel0, atChannel1;
public:
  Select(sc_module_name name, int initialChannel = 0)
    : smoc_actor(name, start) {
    atChannel0
      = (Control(1) && Data0(1) &&
         Control.getValueAt(0) == 0)  >>
        Output(1)                     >>
        call(&Select::action0)        >> atChannel0
      | (Control(1) && Data1(1) &&
         Control.getValueAt(0) == 1)  >>
        Output(1)                     >>
        call(&Select::action1)        >> atChannel1
      | Data0(1)                      >>
        Output(1)                     >>
        call(&Select::action0)        >> atChannel0;
    
    atChannel1
      = (Control(1) && Data0(1) &&
         Control.getValueAt(0) == 0)  >>
        Output(1)                     >>
        call(&Select::action0)        >> atChannel0
      | (Control(1) && Data1(1) &&
         Control.getValueAt(0) == 1)  >>
        Output(1)                     >>
        call(&Select::action1)        >> atChannel1
      | Data0(1)                      >>
        Output(1)                     >>
        call(&Select::action1)        >> atChannel1;
    
    start = initialChannel == 0
      ? atChannel0
      : atChannel1;
  }
};

template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << name() << ": " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> call(&m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_srcbool         srcbool;
  m_h_src<double>     src1, src2;
  Select<double>      select;
  m_h_sink<double>    sink;
public:
  m_h_top( sc_module_name name )
    : smoc_graph(name),
      srcbool("srcbool"),
      src1("src1"), src2("src2"),
      select("select"),
      sink("sink") {
    connectNodePorts(srcbool.out, select.Control);
    connectNodePorts(src1.out, select.Data0);
    connectNodePorts(src2.out, select.Data1);
    connectNodePorts(select.Output, sink.in);
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
