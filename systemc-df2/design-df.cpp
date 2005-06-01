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

class m_h_src: public smoc_fixed_transact_passive_node {
public:
  smoc_port_out<double> out;
private:
  int i;
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0] = i++;
  }
public:
  m_h_src(sc_module_name name)
    : smoc_fixed_transact_passive_node(name,
        out(1) >> call(&m_h_src::src) ),
      i(1) {}
};

class m_h_approx: public smoc_fixed_transact_passive_node {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
public:
  m_h_approx(sc_module_name name)
    : smoc_fixed_transact_passive_node(name,
        (i1(1) & i2(1) & o1(1)) >> call(&m_h_approx::approx) ) {}
};

class m_h_dup: public smoc_fixed_transact_passive_node {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;
private:
  void dup(void) { o1[0] = o2[0] = i1[0]; }
public:
  m_h_dup(sc_module_name name)
    : smoc_fixed_transact_passive_node(name,
        (i1(1) & o1(1) & o2(1)) >> call(&m_h_dup::dup) ) {}
};

class m_h_sqrloop: public smoc_transact_passive_node {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  smoc_firing_state start;
  smoc_firing_state ok, again, write;
  
  // action function from FSM defined in constructor
  const smoc_firing_state &check() {
    std::cout << "check: " << i1[0] << ", " << i2[0] << std::endl;
    o1[0] = i1[0]; o2[0] = i2[0];
    // runtime decission of successor state for FSM defined in constructor
    return (fabs(i1[0] - i2[0]*i2[0]) < 0.0001)
      ? ok 
      : write;
  }
public:
  m_h_sqrloop(sc_module_name name)
    : smoc_transact_passive_node( name, start ) {
// state               guards             action       function      successor states
    start = Transact( (i1(1) & i2(1)) >> branch(&m_h_sqrloop::check, ok | write ) );
    ok    = Transact( (o1(1) & o2(1)) >>                             start);
    write = Transact(  o1(1)          >>                             again);
    again = Transact(  i2(1)          >> branch(&m_h_sqrloop::check, ok | write ) );
  }
};

class m_h_sink: public smoc_fixed_transact_passive_node {
public:
  smoc_port_in<double> in;
private:
  int i;
  
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
public:
  m_h_sink(sc_module_name name)
    : smoc_fixed_transact_passive_node(name,
        in(1) >> call(&m_h_sink::sink) ) {}
};

class m_h_approx_loop
: public smoc_ddf_constraintset {
  public:
    smoc_port_in<double>  i1;
    smoc_port_out<double> o1;
  protected:
    m_h_sqrloop sqrloop;
    m_h_approx  approx;
    m_h_dup     dup;
  public:
    m_h_approx_loop( sc_module_name name )
      : smoc_ddf_constraintset(name),
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

class m_h_top
: public smoc_ddf_constraintset {
  public:
  protected:
    m_h_src                         src;
    smoc_ddf_moc<m_h_approx_loop>   al;
    m_h_sink                        sink;
  public:
    m_h_top( sc_module_name name )
      : smoc_ddf_constraintset(name),
        src("src"),
        al("al"),
        sink("sink") {
      connectNodePorts(src.out, al.i1);
      connectNodePorts(al.o1, sink.in);
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<smoc_ddf_moc<m_h_top> > top("top");
  
  sc_start(-1);
  return 0;
}
