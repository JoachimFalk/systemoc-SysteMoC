// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_moc.hpp>
#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
# include <hscd_scheduler.hpp>
# include <hscd_pggen.hpp>
#endif

template <typename T>
class m_adder: public hscd_fixed_transact_active_node {
  public:
    hscd_port_in<T>  in1;
    hscd_port_in<T>  in2;
    hscd_port_out<T> out;
  private:
    void process() {
      while (true) {
	out[0] = in1[0] + in1[1] + in2[0]; 
	std::cout << name() << " adding " << in1[0] << " + " << in1[1] << " + " << in2[0] << " = " << out[0] << std::endl;
	transact();
      }
    }
  public:
    m_adder( sc_module_name name )
      :hscd_fixed_transact_active_node( name, in1(2) & in2(1) & out(1) ) {}
};

template <typename T>
class m_multiply: public hscd_fixed_transact_active_node {
  public:
    hscd_port_in<T>  in1;
    hscd_port_in<T>  in2;
    hscd_port_out<T> out1;
    hscd_port_out<T> out2;
  private:
    void process() {
      while (true) {
	out1[0] = in1[0] * in2[0];
        out2[0] = out1[0];
	std::cout << name() << " multiplying " << in1[0] << " * " << in2[0] << " = " << out1[0] << std::endl;
	transact();
      }
    }
  public:
    m_multiply( sc_module_name name )
      :hscd_fixed_transact_active_node( name, in1(1) & in2(1) & out1(1) & out2(1) ) {}
};

class m_top2
  : public hscd_sdf_constraintset {
  public:
    hscd_port_in<int>  in1;
    hscd_port_in<int>  in2;
    hscd_port_out<int> out;
    
    m_top2( sc_module_name name )
      : hscd_sdf_constraintset(name)
    {
      m_adder<int>    &adder = registerNode(new m_adder<int>("adder"));
      m_multiply<int> &mult  = registerNode(new m_multiply<int>("multiply"));
      
      connectInterfacePorts( in1, adder.in1 ); // adder.in(in1);
      connectInterfacePorts( in2, mult.in1 );  // mult.in1(in2);
      connectNodePorts( adder.out, mult.in2 );
      connectNodePorts( mult.out2, adder.in2, hscd_fifo<int>() << 13 );
      connectInterfacePorts( out, mult.out1 ); // mult.out(out);
    }
};

class m_source: public hscd_fixed_transact_active_node {
  public:
    hscd_port_out<int> out;
  private:
    void process() {
      int i = 0;
      
      while (true) {
	std::cout << name() << " generating " << i << std::endl;
	out[0] = i++;
	transact();
      }
    }
  public:
    m_source( sc_module_name name )
      :hscd_fixed_transact_active_node( name, out(1) ) {}
};

class m_sink: public hscd_fixed_transact_active_node {
  public:
    hscd_port_in<int> in;
  private:
    void process() {
      while (true) {
	std::cout << name() << " receiving " << in[0] << std::endl;
	transact();
      }
    }
  public:
    m_sink( sc_module_name name )
      :hscd_fixed_transact_active_node( name, in(1) ) {}
};

class m_top
: public hscd_sdf_constraintset {
  public:
    m_top( sc_module_name name )
      : hscd_sdf_constraintset(name) {
      m_top2        &top2 = registerNode(new hscd_sdf_moc<m_top2>("top2"));
      m_source      &src1 = registerNode(new m_source("src1"));
      m_source      &src2 = registerNode(new m_source("src2"));
      m_sink        &sink = registerNode(new m_sink("sink"));
      
      connectNodePorts( src1.out, top2.in1 );
      connectNodePorts( src2.out, top2.in2 );
      connectNodePorts( top2.out, sink.in );
    }
};

int sc_main (int argc, char **argv) {
#ifndef __SCFE__
  try {
#endif
    hscd_sdf_moc<m_top> *top = new hscd_sdf_moc<m_top>("top");
#ifndef __SCFE__
    hscd_top x(top);

    hscd_modes::dump( std::cout, *top );
  
    sc_start(-1);
  } catch (...) {
    std::cout << "exception !" << std::endl;
    throw;
  }
#endif
  return 0;
}
