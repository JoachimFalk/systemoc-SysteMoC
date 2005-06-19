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
    start = (out.getAvailableSpace() >= 1) >> call(&m_h_src::src) >> start;
  }
};

template <typename T>
class Select: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { Output[0] = Data0[0] ; }
  void action1() { Output[0] = Data1[0] ; }
  smoc_firing_state start;
public:
  Select(sc_module_name name, int initialChannel = 0)
    : smoc_actor(name, start) {
    smoc_firing_state atChannel0, atChannel1;
    smoc_firing_state test;

    atChannel0
      = (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         (Control.getValueAt(0) & 1 == false)  ) >>
        (Output.getAvailableSpace()   >= 1 ) >>
        call(&Select::action0)               >> atChannel0
      | (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         (Control.getValueAt(0) & 1 == true )  ) >>
        (Output.getAvailableSpace()   >= 1 ) >> 
        call(&Select::action1)               >> atChannel1
      | (Data0.getAvailableTokens()   >= 1 ) >>
        (Output.getAvailableSpace()   >= 1 ) >>
         call(&Select::action0)              >> atChannel0;

    atChannel1
      = (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         (Control.getValueAt(0) & 1 == true )  ) >>
        (Output.getAvailableSpace()   >= 1 ) >>
        call(&Select::action1)               >> atChannel1
      | (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         (Control.getValueAt(0) & 1 == false)  ) >>
        (Output.getAvailableSpace()   >= 1 ) >> 
        call(&Select::action0)               >> atChannel0
      | (Data1.getAvailableTokens()   >= 1 ) >>
        (Output.getAvailableSpace()   >= 1 ) >>
         call(&Select::action1)              >> atChannel1;
    
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
  
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in.getAvailableTokens() >= 1) >> call(&m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_ndf_constraintset {
protected:
  m_h_src<int>        srcbool;
  m_h_src<double>     src1, src2;
  Select<double>      select;
  m_h_sink<double>    sink;
public:
  m_h_top( sc_module_name name )
    : smoc_ndf_constraintset(name),
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
  smoc_top_moc<smoc_ndf_moc<m_h_top> > top("top");
  
  sc_start(-1);
  return 0;
}
