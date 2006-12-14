// vim: set sw=2 ts=8:

#include <iostream>
#include <string>
#include <queue>
#include <cassert>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include <par_port.hpp>
#include <par_actor.hpp>
#include <par_actor_factory.hpp>

#include <oneof.hpp>

using CoSupport::par_port_out;
using CoSupport::par_port_in;
using CoSupport::par_actor;
using CoSupport::par_actor_factory;
using Expr::isType;

struct msg_add {};
struct msg_neg {};

ostream& operator<<(ostream& os, const msg_add& msg) {
  return os << "msg_add" << std::endl;
}

ostream& operator<<(ostream& os, const msg_neg& msg) {
  return os << "msg_neg" << std::endl;
}

typedef CoSupport::oneof<msg_add, msg_neg, double> ct_src2mod;

/**
 * m_mod performs arithmetic operations based on input tokens
 * and delivers results to sink
 */
class m_mod :
  public smoc_actor
{
public:
  smoc_port_in<ct_src2mod> in_src2mod;
  smoc_port_out<double>    out_mod2sink;
  
private:
  
  void add() {
    assert(isType<double>(in_src2mod[1]));
    assert(isType<double>(in_src2mod[2]));
    double a = in_src2mod[1];
    double b = in_src2mod[2];
    std::cout << name() << "> proc. ADD(" << a << ", " << b << ")" << std::endl; 
    out_mod2sink[0] = a + b;
  }

  void neg() {
    assert(isType<double>(in_src2mod[1]));
    double a = in_src2mod[1];
    std::cout << name() << "> NEG(" << a << ")" << std::endl;
    out_mod2sink[0] = -a;
  }
  
  smoc_firing_state run;

public:
  m_mod(sc_module_name name) :
    smoc_actor(name, run)
  {
    run =
        (in_src2mod(3) && isType<msg_add>(in_src2mod.getValueAt(0)))
     >> out_mod2sink(1)
     >> CALL(m_mod::add)
     >> run
   |     (in_src2mod(2) && isType<msg_neg>(in_src2mod.getValueAt(0)))
     >> out_mod2sink(1)
     >> CALL(m_mod::neg)
     >> run
   ;
  }
  
public:
  
  struct factory : public par_actor_factory<m_mod>
  {
    factory(const std::string& prefix) :
      par_actor_factory<m_mod>(prefix)
    {}
    
    void construct_helper(m_mod* p, const std::string& name)
    { new (p) m_mod(name.c_str()); }
  };
};

/**
 * src generates arithmetic operations and delivers them
 * to the processing actor
 */
class m_src :
  public smoc_actor
{
public:
  smoc_port_out<ct_src2mod> out_src2mod;

private:
  int i;
  int n;
  
  bool check_add() const {
    return (i < n) && ((i % 2) == 0);
  }

  bool check_neg() const {
    return (i < n) && ((i % 2) == 1);
  }
  
  void gen_add() {
    double a = i / 2;
    double b = i * 2;
    
    std::cout << name() << "> gen. ADD(" << a << ", " << b << ")" << std::endl;
    
    out_src2mod[0] = msg_add();
    out_src2mod[1] = a;
    out_src2mod[2] = b;
    
    ++i;
  }

  void gen_neg() {
    std::cout << name() << "> gen. NEG(" << i << ")" << std::endl;
    
    out_src2mod[0] = msg_neg();
    out_src2mod[1] = i;
    
    ++i;    
  }  
  
  smoc_firing_state start;

public:
  m_src(sc_module_name name, int times) :
    smoc_actor(name, start),
    i(0),
    n(times)
  { 
    start = 
         (out_src2mod(3) && GUARD(m_src::check_add)) 
      >> CALL(m_src::gen_add)
      >> start
    |    (out_src2mod(2) && GUARD(m_src::check_neg))
      >> CALL(m_src::gen_neg)
      >> start
    ;           
  }
};

/**
 * sink simply prints out received numbers
 */
class m_sink :
  public smoc_actor
{
public:
  smoc_port_in<double> in_mod2sink;
private:
  
  void sink() {
    std::cout << name() << ">" << in_mod2sink[0] << std::endl;
  }
  
  smoc_firing_state start;

public:
  m_sink(sc_module_name name) :
    smoc_actor(name, start)
  {
    start = 
         in_mod2sink(1) 
      >> CALL(m_sink::sink) 
      >> start;
  }
};

/**
 * dispatcher manages parallization
 */
class m_mod_dispatcher :
  public smoc_actor
{
public:
  smoc_port_in<ct_src2mod>     in_src2mod;
  par_port_out<ct_src2mod>::ty out_src2mod;
  
  par_port_in<double>::ty      in_mod2sink;
  smoc_port_out<double>        out_mod2sink;
  
private:
  
  void copy_src2mod(smoc_port_out<ct_src2mod> &out_src2mod, size_t o, size_t count) {
    for(size_t i=0; i<count; ++i) {
      std::cout << "DISPATCHER> src -> mod(" << o << ")" << std::endl;
      out_src2mod[i] = in_src2mod[i];
    }
  }
  
  void copy_mod2sink(smoc_port_in<double> &in_mod2sink, size_t i) {
    std::cout << "DISPATCHER> mod("<< i << ") -> sink" << std::endl;
    out_mod2sink[0] = in_mod2sink[0];
  }
  
  smoc_firing_state run;
  
public:

  m_mod_dispatcher(sc_module_name name, size_t mod_count) :
    smoc_actor(name, run),
    out_src2mod(mod_count),
    in_mod2sink(mod_count)
  {
    smoc_transition_list stl;
    
    for(size_t o=0; o<mod_count; ++o) {
      
      // if operator is add, copy 3 tokens at once
      stl |=
           (in_src2mod(3) && isType<msg_add>(in_src2mod.getValueAt(0)))
        >> out_src2mod(o)(3)
        >> CALL(m_mod_dispatcher::copy_src2mod)(out_src2mod(o))(o)(3)
	>> run;
      // if operator is neg, copy 2 tokens at once
      stl |=
           (in_src2mod(2) && isType<msg_neg>(in_src2mod.getValueAt(0)))
        >> out_src2mod(o)(2)
        >> CALL(m_mod_dispatcher::copy_src2mod)(out_src2mod(o))(o)(2)
        >> run;
    }

    for(size_t i=0; i<mod_count; ++i) {
      
      // fifo to sink is not important in this example
      stl |=
           in_mod2sink(i)(1)
        >> out_mod2sink(1)
	>> CALL(m_mod_dispatcher::copy_mod2sink)(in_mod2sink(i))(i)
	>> run;
    }

    run = stl;
  }
  
};

/**
 * convenience graph for encapsulating the par_actor and
 * the appropriate dispatcher
 */
class m_mod_graph :
  public smoc_graph
{
public:
  smoc_port_in<ct_src2mod> in_src2mod;
  smoc_port_out<double>    out_mod2sink;
  
private:
  par_actor<m_mod>    mod;
  m_mod_dispatcher    disp;
    
public:
  m_mod_graph( sc_module_name name ) :
    smoc_graph(name),
    mod(m_mod::factory("mod")),
    disp("disp", mod.count())
  {
    connectInterfacePorts(in_src2mod, disp.in_src2mod);
     
    for(size_t i=0; i<mod.count(); ++i) {
      connectNodePorts<3>(disp.out_src2mod(i), mod(i).in_src2mod);
      connectNodePorts<1>(mod(i).out_mod2sink, disp.in_mod2sink(i));
    }

    connectInterfacePorts(out_mod2sink, disp.out_mod2sink);
  }
};

class m_top : 
  public smoc_graph
{
  private:
    m_src       src;
    m_sink      sink;
    m_mod_graph mod;
    
  public:
    m_top( sc_module_name name ) :
      smoc_graph(name),
      src("src", 50),
      sink("sink"),
      mod("mod_graph")
    {
      connectNodePorts<3>(src.out_src2mod, mod.in_src2mod);   
      connectNodePorts<1>(mod.out_mod2sink, sink.in_mod2sink);  
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  sc_start(-1);
  return 0;
}
