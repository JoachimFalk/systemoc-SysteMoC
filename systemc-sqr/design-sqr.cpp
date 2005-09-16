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

class m_src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  int i;
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0] = i++;
  }
  
  smoc_firing_state start;
public:
  m_src(sc_module_name name)
    : smoc_actor(name, start), i(1)
    { start = out(1)            >>
              call(&m_src::src) >> start; }
};

class m_approx: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
  
  smoc_firing_state start;
public:
  m_approx(sc_module_name name)
    : smoc_actor(name, start)
    { start = (i1(1) && i2(1)) >> o1(1) >>
              call(&m_approx::approx)   >> start; }
};

class m_dup: public smoc_actor {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;
private:
  void dup(void) { o1[0] = o2[0] = i1[0]; }
  
  smoc_firing_state start;
public:
  m_dup(sc_module_name name)
    : smoc_actor(name, start)
    { start = i1(1) >> (o1(1) && o2(1)) >>
              call(&m_dup::dup)         >> start; }
};

class m_sqrloop: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  double tmp_i1;
  
  // action functions for FSM defined in constructor
  void store() { tmp_i1 = i1[0]; }
  void copy1() { o1[0] = tmp_i1; }
  void copy2() { o1[0] = tmp_i1; o2[0] = i2[0]; }
  
  // guard  functions for FSM defined in constructor
  bool check() const {
    std::cout << "check: " << tmp_i1 << ", " << i2[0] << std::endl;
    return fabs(tmp_i1 - i2[0]*i2[0]) < 0.0001;
  }
  
  smoc_firing_state start;
  smoc_firing_state loop;
public:
  m_sqrloop(sc_module_name name)
    : smoc_actor( name, start ) {
    start = i1(1)                                 >>
            call(&m_sqrloop::store)               >> loop;
    loop  = (i2(1) &&  guard(&m_sqrloop::check))  >>
            (o1(1) && o2(1))                      >>
            call(&m_sqrloop::copy2)               >> start
          | (i2(1) && !guard(&m_sqrloop::check))  >>
            o1(1)                                 >>
            call(&m_sqrloop::copy1)               >> loop;
  }
};

class m_sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_sink(sc_module_name name)
    : smoc_actor(name, start)
    { start = in(1) >> call(&m_sink::sink) >> start; }
};

class m_approx_loop
: public smoc_graph {
  public:
    smoc_port_in<double>  i1;
    smoc_port_out<double> o1;
  protected:
    m_sqrloop sqrloop;
    m_approx  approx;
    m_dup     dup;
  public:
    m_approx_loop( sc_module_name name )
      : smoc_graph(name),
        sqrloop("sqrloop"),
        approx("approx"),
        dup("dup") {
      sqrloop.i1(i1);
      connectNodePorts(sqrloop.o1, approx.i1);
      connectNodePorts(approx.o1, dup.i1, smoc_fifo<double>() << 2 );
      connectNodePorts(dup.o1, approx.i2);
      connectNodePorts(dup.o2, sqrloop.i2);
      sqrloop.o2(o1);
    }
};

class m_top
: public smoc_graph {
  public:
  protected:
    m_src           src;
    m_approx_loop   al;
    m_sink          sink;
  public:
    m_top( sc_module_name name )
      : smoc_graph(name),
        src("src"),
        al("al"),
        sink("sink") {
      connectNodePorts(src.out, al.i1);
      connectNodePorts(al.o1, sink.in);
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
