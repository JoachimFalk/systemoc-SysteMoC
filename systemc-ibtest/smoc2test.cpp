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

// InifiniBand Includes
#include "tt_ib.h"
#include <oneof.hpp>

using Expr::field;
using Expr::isType;


typedef oneof<int, char *> test_ty;

class m_source: public smoc_actor {
  public:
    smoc_port_out<test_ty> out;
  private:
    int count;
        
    void process() {
      
      std::cout << name() << " generating token: ";
      out[0] = count % 2;
      std::cout << out[0] << std::endl;

      count++;
    }
    
    smoc_firing_state start;
  
  public:
    m_source( sc_module_name name ) :
      smoc_actor( name, start ),
      count(0)
    {
      start = out(1) >> call(&m_source::process) >> start;
    }
};

class m_dispatcher : public smoc_actor 
{

  
  public:
    smoc_port_in<test_ty> in;
    smoc_port_out<int> out1;
    smoc_port_out<int> out2;

  private:

    int temp;
    
    // FSM states
    smoc_firing_state start;
    smoc_firing_state work;
    
    // methods
    
    void copy() {
      std::cout << name() << ": COPY called: ";
      temp = in[0];
      std::cout  << temp << std::endl;
    }
    
    void process1() {
      std::cout << name() << ": writing input to output channel 0" << std::endl;
      out1[0] = in[0];
    }
    
    void process2() {
      std::cout << name() << ": writing input to output channel 1" << std::endl;
      out2[0] = in[0];
    }

    bool check() const {
      if ( temp > 0 ) return true;
      else return false;
    }
    
  public:
    m_dispatcher( sc_module_name name ) :
      smoc_actor( name, start )
    {
      // use temprorary variable to buffer an input token before it will be transmitted
      /*
      start = (in(1) && field(*in.getValueAt(0), &tt_ib::type) ==  tt_ib::TT_PACKET_INFO) >> call(&m_dispatcher::copy) >> work;
      start = in(1) >> call(&m_dispatcher::copy) >> work;  
      work  = (var(temp) == 0) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (var(temp) == 1) >> out2(1) >> call(&m_dispatcher::process2) >> start;
      */
      
      // example with input message type checking
      /*
      start = (in(1) && (isType<int>(in.getValueAt(0))))) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (in(1) && (in.getValueAt(0) == 1)) >> out2(1) >> call(&m_dispatcher::process2) >> start;
      */
      // atomic transitions checking for 0 or 1 value on input channel
      start = (in(1) && (in.getValueAt(0) == 0)) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (in(1) && (in.getValueAt(0) == 1)) >> out2(1) >> call(&m_dispatcher::process2) >> start;
    }
};


class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
   
    void print() {
      std::cout << name() << ": got token: ";
      std::cout << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  
  public:
    m_sink( sc_module_name name ) :
      smoc_actor( name, start )
    {
      start = in(1) >> call(&m_sink::print) >> start;
    }
};


class m_top
: public smoc_ndf_constraintset {
  public:
    m_top( sc_module_name name )
      : smoc_ndf_constraintset(name)
    {
      m_source      &src  = registerNode(new m_source("src"));
      
      m_sink        &snk1 = registerNode(new m_sink("snk1"));
      m_sink        &snk2 = registerNode(new m_sink("snk2"));
      m_dispatcher  &disp = registerNode(new m_dispatcher("disp"));
      connectNodePorts( src.out, disp.in );
      connectNodePorts( disp.out1, snk1.in );
      connectNodePorts( disp.out2, snk2.in );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<smoc_ndf_moc<m_top> > top("top");
  
  sc_start(-1);
  return 0;
}
