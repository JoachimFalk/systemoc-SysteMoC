// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <sstream>
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

template<class T>
struct ActorFactory
{
  virtual T* construct(const char *name) const = 0;
  virtual ~ActorFactory() {}
};

class m_mod :
  public smoc_actor
{
public:
  smoc_port_in<double> in_src2mod;
  smoc_port_out<double> out_mod2sink;
  
private:
  void copy() {
    std::cout << name() << "> proc. " << in_src2mod[0] << std::endl; 
    out_mod2sink[0] = in_src2mod[0];
  }
  
  smoc_firing_state run;

public:
  m_mod(sc_module_name name) :
    smoc_actor(name, run)
  {
    run =
        in_src2mod(1)
     >> out_mod2sink(1)
     >> CALL(m_mod::copy)
     >> run;
  }
public:
  
  struct factory : public ActorFactory<m_mod>
  {
    factory() {}
    
    m_mod *construct(const char *name) const {
      return new m_mod(name);
    }
  };
  
};

class m_src :
  public smoc_actor
{
public:
  smoc_port_out<double> out_src2mod;

private:
  int i;
  int n;
  
  bool nTimesTrue() const { return n != 0 ;}

  void src() {
    //std::cout << "src: " << i << std::endl;
    out_src2mod[0] = i++;
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
         ( out_src2mod(1) && GUARD(m_src::nTimesTrue) ) 
      >> CALL(m_src::src)
      >> start;
  }
};

class m_sink :
  public smoc_actor
{
public:
  smoc_port_in<double> in_mod2sink;
private:
  int i;
  
  void sink() {
    std::cout << "sink: " << in_mod2sink[0] << "(exp. " << i++ << ")" << std::endl;
  }
  
  smoc_firing_state start;

public:
  m_sink(sc_module_name name) :
    smoc_actor(name, start),
    i(1)
  {
    start = 
         in_mod2sink(1) 
      >> CALL(m_sink::sink) 
      >> start;
  }
};


template<class CHAN_TYPE>
class m_par_port_in
{
public:
  typedef smoc_port_in<CHAN_TYPE> port_type;  
  
private:
  port_type *p_inst;
  int        n_inst;
  
public:
  
  m_par_port_in(int inst) :
    p_inst(new port_type[inst]),
    n_inst(inst)
  {}
  
  ~m_par_port_in() {
    delete[] p_inst;
  }
  
  port_type& operator()(int i) const {
    assert(i >= 0 && i < n_inst);
    return p_inst[i];
  }
  
  int count() const { return n_inst; }
};


template<class CHAN_TYPE>
class m_par_port_out
{
public:
  typedef smoc_port_out<CHAN_TYPE> port_type;

private:
  port_type *p_inst;
  int        n_inst;

public:
  
  m_par_port_out(int inst) :
    p_inst(new port_type[inst]),
    n_inst(inst)
  {}
  
  ~m_par_port_out() {
    delete[] p_inst;
  }
  
  port_type& operator()(int i) const {
    assert(i >= 0 && i < n_inst);
    return p_inst[i];
  }
  
  int count() const { return n_inst; }
};


template<class M>
class m_par_actor
{
private:
  M         **p_inst;
  int         n_inst;
  
public:
  m_par_actor(const char *name_prefix, int inst, const ActorFactory<M> &f) :
    p_inst(new M *[inst]),
    n_inst(inst)
  {
    for(int i=0; i<inst; ++i) {
      std::ostringstream name;
      name << name_prefix << i;
      p_inst[i] = f.construct(name.str().c_str());
    }
  }

  ~m_par_actor() {
    for(int i=0; i<n_inst; ++i) {
      delete p_inst[i];
    }
    delete[] p_inst;
  }
  
  M& operator()(int i) {
    assert(i >= 0 && i < n_inst);
    return *p_inst[i];
  }
  
  int count() const { return n_inst; }
};



class m_mod_dispatcher :
  public smoc_actor
{
public:
  m_par_port_in<double>  in_src2mod;
  m_par_port_out<double> out_src2mod;

  m_par_port_in<double>  in_mod2sink;
  m_par_port_out<double> out_mod2sink;
  
private:  

  std::queue<int> control;
  
  void copy_src2mod(smoc_port_in<double> &in, smoc_port_out<double> &out, int i, int o) {
    std::cout << "DISPATCHER> src(" << i << ") -> mod(" << o << ")" << std::endl;
    
    control.push(o); // Remember which instance received token
    
    out[0] = in[0];
  }
  
  bool guard_mod2sink(int i) const {
    return i == control.front();
  }
  
  void copy_mod2sink(smoc_port_in<double> &in, smoc_port_out<double> &out, int i, int o) {
    std::cout << "DISPATCHER> mod("<< i << ") -> sink("<< o << ")" << std::endl;
    
    control.pop(); // Remove control info
    
    out[0] = in[0];
  }
  
  smoc_firing_state run;
  
public:

  m_mod_dispatcher(sc_module_name name, int mod_count, int src_count, int sink_count) :
    smoc_actor(name, run),
    in_src2mod(src_count),
    out_src2mod(mod_count),
    in_mod2sink(mod_count),
    out_mod2sink(sink_count)
  {
    smoc_transition_list stl;
    
    for(int i=0; i<src_count; ++i) {
      for(int o=0; o<mod_count; ++o) {
	
	stl |= in_src2mod(i)(1)
	    >> out_src2mod(o)(1)
	    >> CALL(m_mod_dispatcher::copy_src2mod)(in_src2mod(i))(out_src2mod(o))(i)(o)
	    >> run;
      }
    }

    for(int i=0; i<mod_count; ++i) {
      for(int o=0; o<sink_count; ++o) {

	stl |= (in_mod2sink(i)(1) && GUARD(m_mod_dispatcher::guard_mod2sink)(i))
	    >> out_mod2sink(o)(1)
	    >> CALL(m_mod_dispatcher::copy_mod2sink)(in_mod2sink(i))(out_mod2sink(o))(i)(o)
	    >> run;
      }
    }

    run = stl;
  }
  
};


class m_top : 
  public smoc_graph
{
  private:
    m_src               src;
    m_sink              sink;
    
    m_mod_dispatcher    disp;
    m_par_actor<m_mod>  mod;
    
  public:
    m_top( sc_module_name name, int mod_inst ) :
      smoc_graph(name),
      src("src", 50),
      sink("sink"),
      disp("disp", mod_inst, 1, 1),
      mod("mod", mod_inst, m_mod::factory())
    {
      connectNodePorts(src.out_src2mod, disp.in_src2mod(0));
      
      for(int i=0; i<mod_inst; ++i) {
	connectNodePorts(disp.out_src2mod(i), mod(i).in_src2mod);
	connectNodePorts(mod(i).out_mod2sink, disp.in_mod2sink(i));
      }
      
      connectNodePorts(disp.out_mod2sink(0), sink.in_mod2sink);  
    }
};

int sc_main (int argc, char **argv) {
  
  int mod_inst;
  
#define MOD_INST "--mod_inst"
  if(argc > 2 && 0 == strncmp(argv[1], MOD_INST, sizeof(MOD_INST))) {
    mod_inst = atoi(argv[2]);
    assert(mod_inst > 0);
  } else {
    std::cerr << "Usage: ./simulation-sqr --mod_inst <n> [--generate-problemgraph]" << std::endl;
    return 1;
  }
#undef MOD_INST
  
  smoc_top_moc<m_top> top("top", mod_inst);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 3 && 0 == strncmp(argv[3], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
