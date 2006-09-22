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

using namespace std; 

class Src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  int i;
  
  void src() {
#ifndef NDEBUG
    cout << "src: " << i << std::endl;
#endif
    out[0] = i++;
  }
  
  smoc_firing_state start;
public:
  Src(sc_module_name name, SMOC_ACTOR_CPARAM(int,from))
    : smoc_actor(name, start), i(from) {
    start =
        (VAR(i) <= 1000000)       >>
        out(1)                    >>
        CALL(Src::src)            >> start
      ;
  }
};

// Definition of the SqrLoop actor class
class SqrLoop
  // All actor classes must be derived
  // from the smoc_actor base class
  : public smoc_actor {
public:
  // Declaration of input and output ports
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  // Declaration of the actor functionality
  // via member variables and methods
  double tmp_i1;
  
  // action functions triggered by the
  // FSM declared in the constructor
  void copyStore()  { o1[0] = tmp_i1 = i1[0];  }
  void copyInput()  { o1[0] = tmp_i1;          }
  void copyApprox() { o2[0] = i2[0];           }
  
  // guard  functions used by the
  // FSM declared in the constructor
  bool check() const {
#ifndef NDEBUG
    cout << "check: " << tmp_i1 << ", " << i2[0] << std::endl;
#endif
    return fabs(tmp_i1 - i2[0]*i2[0]) < 0.0001;
  }
  
  // Declaration of firing states for the FSM
  smoc_firing_state start;
  smoc_firing_state loop;
public:
  // Constructor responsible for declaring the
  // communication FSM and initializing the actor
  SqrLoop(sc_module_name name)
    : smoc_actor( name, start ) {
    start =
        i1(1)                               >>
        o1(1)                               >>
        CALL(SqrLoop::copyStore)            >> loop
      ;
    loop  =
        (i2(1) &&  GUARD(SqrLoop::check))   >>
        o2(1)                               >>
        CALL(SqrLoop::copyApprox)           >> start
      | (i2(1) && !GUARD(SqrLoop::check))   >>
        o1(1)                               >>
        CALL(SqrLoop::copyInput)            >> loop
      ;
  }
};

class Approx: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  // Square root successive approximation step of Newton
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
  
  smoc_firing_state start;
public:
  Approx(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        (i1(1) && i2(1))         >>
        o1(1)                    >>
        CALL(Approx::approx)     >> start
      ;
  }
};

class Dup: public smoc_actor {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;

private:
  void dup() { 
    double in = i1[0];
    o1[0] = in;
    o2[0] = in;
  }
  
  smoc_firing_state start;
public:
  Dup(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        i1(1)                    >>
        (o1(1) && o2(1))         >>
        CALL(Dup::dup)           >> start
      ;
  }
};

class Sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) {
#ifndef NDEBUG
    cout << "sink: " << in[0] << std::endl;
#endif  
  }
  
  smoc_firing_state start;
public:
  Sink(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        in(1)             >>
        CALL(Sink::sink)  >>
	start
      ;
  }
};

class SqrRoot
: public smoc_graph {
public:
protected:
  Src      src;
  SqrLoop  sqrloop;
  Approx   approx;
  Dup      dup;
  Sink     sink;
public:
  SqrRoot( sc_module_name name )
    : smoc_graph(name),
      src("a1", 1),
      sqrloop("a2"),
      approx("a3"),
      dup("a4"),
      sink("a5") {
    connectNodePorts(src.out,    sqrloop.i1);
    connectNodePorts(sqrloop.o1, approx.i1);
#ifndef KASCPAR_PARSING
    connectNodePorts(approx.o1,  dup.i1,
                     smoc_fifo<double>(1));
    connectNodePorts(dup.o1,     approx.i2,
                     smoc_fifo<double>() << 2 );
#endif
    connectNodePorts(dup.o2,     sqrloop.i2);
    connectNodePorts(sqrloop.o2, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<SqrRoot> sqrroot("sqrroot");
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, sqrroot);
  } else {
    sc_start(-1);
  }
#undef GENERATE
#endif
  return 0;
}
