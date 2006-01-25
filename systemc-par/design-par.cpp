// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

class m_mod :
  public smoc_actor
{
public:
  smoc_port_in<double> in;
  smoc_port_out<double> out;

private:
  void copy() {
    std::cout << name() << "> proc. " << in[0] << std::endl; 
    out[0] = in[0];
  }
  
  smoc_firing_state run;

public:
  m_mod(sc_module_name name) :
    smoc_actor(name, run)
  {
    run =
        in(1)
     >> out(1)
     >> CALL(m_mod::copy)
     >> run;
  }
};

class m_src :
  public smoc_actor
{
public:
  smoc_port_out<double> out;

private:
  int i;
  int n;
  
  bool nTimesTrue() const { return n != 0 ;}

  void src() {
    //std::cout << "src: " << i << std::endl;
    out[0] = i++;
    if(n>0) n--;
  }
  
  smoc_firing_state start;

public:
  m_src(sc_module_name name, int times) :
    smoc_actor(name, start),
    i(1),
    n(times)
  { 
    start = 
         ( out(1) && GUARD(m_src::nTimesTrue) ) 
      >> CALL(m_src::src)
      >> start;
  }
};

class m_sink :
  public smoc_actor
{
public:
  smoc_port_in<double> in;
private:
  int i;
  
  void sink() {
    std::cout << "sink: " << in[0] << "(exp. " << i++ << ")" << std::endl;
  }
  
  smoc_firing_state start;

public:
  m_sink(sc_module_name name) :
    smoc_actor(name, start),
    i(1)
  {
    start = 
         in(1) 
      >> CALL(m_sink::sink) 
      >> start;
  }
};


template<class CHAN_TYPE_IN, class CHAN_TYPE_OUT>
class m_disp_base
{
private:
  smoc_port_in<CHAN_TYPE_IN> *p_in;
  smoc_port_out<CHAN_TYPE_OUT> *p_out;
  
  int n_in;
  int n_out;
public:
  m_disp_base(int in, int out) :
    p_in(new smoc_port_in<CHAN_TYPE_IN>[in]),
    p_out(new smoc_port_out<CHAN_TYPE_OUT>[out]),
    n_in(in),
    n_out(out)
  {}
  
  ~m_disp_base() {
    delete[] p_in;
    delete[] p_out;
  }

  smoc_port_in<CHAN_TYPE_IN>& in(int i) {
    assert(i >= 0 && i < n_in);
    return p_in[i];
  }

  smoc_port_out<CHAN_TYPE_OUT>& out(int i) {
    assert(i >= 0 && i < n_out);
    return p_out[i];
  }
  
  int in_count() const { return n_in; }
  int out_count() const { return n_out; }
};


class m_d_src2mod :
  public smoc_actor,
  public m_disp_base<double,double>
{ 
public:
  smoc_port_out<int> sm2ms;
  
private:
  void copy(int i, int o) { std::cout << "SRC2MOD> " << i << " -> " << o << std::endl; out(o)[0] = in(i)[0]; sm2ms[0] = o; }
  
  smoc_firing_state run;
  
public:
  m_d_src2mod(sc_module_name name, int _in, int _out) :
    smoc_actor(name, run),
    m_disp_base<double,double>(_in, _out)
  {
    smoc_transition_list stl;
    
    for(int i=0; i<in_count(); ++i) {
      for(int o=0; o<out_count(); ++o) {
	stl |= in(i)(1)
            >> (out(o)(1) && sm2ms(1))
	    >> CALL(m_d_src2mod::copy)(i)(o)
	    >> run;
      }
    }
    
    run = stl;
  }
};

class m_d_mod2sink :
  public smoc_actor,
  public m_disp_base<double,double>
{
public:
  smoc_port_in<int> sm2ms;
  
private:
  void copy(int i, int o) { out(o)[0] = in(i)[0]; }
  bool my_guard(int i) const { return sm2ms[0] == i; }

  smoc_firing_state run;
  
public:
  m_d_mod2sink(sc_module_name name, int _in, int _out) :
    smoc_actor(name, run),
    m_disp_base<double,double>(_in, _out)
  {
    smoc_transition_list stl;

    for(int i=0; i<in_count(); ++i) {
      for(int o=0; o<out_count(); ++o) {
	
	stl |= (in(i)(1) && sm2ms(1) && GUARD(m_d_mod2sink::my_guard)(i))
            >> out(o)(1)
            >> CALL(m_d_mod2sink::copy)(i)(o)
            >> run;
      }
    }
    
    run = stl;
  }  
};

#define MOD_INST 3

class m_top : 
  public smoc_graph
{
  private:
    m_src             src;
    m_sink            sink;
    m_d_src2mod       src2mod;
    m_d_mod2sink      mod2sink;
    
    m_mod             *mod[MOD_INST];
    
  public:
    m_top( sc_module_name name ) :
      smoc_graph(name),
      src("src", 50),
      sink("sink"),
      src2mod("src2mod", 1, MOD_INST),
      mod2sink("mod2sink", MOD_INST, 1)
    {
      connectNodePorts(src.out, src2mod.in(0));
      
      for(int i=0; i<MOD_INST; ++i) {
	std::ostringstream name;
	name << "mod" << i;
	mod[i] = new m_mod(name.str().c_str());
	connectNodePorts(src2mod.out(i), mod[i]->in);
	connectNodePorts(mod[i]->out, mod2sink.in(i));
      }
      
      connectNodePorts(mod2sink.out(0), sink.in);
      
      connectNodePorts(src2mod.sm2ms, mod2sink.sm2ms, smoc_fifo<int>(MOD_INST) );
    }    
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
