// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_sr_signal.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

template<typename T>
class ConstSource: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T   output;
  int limitCount;
  void src() {
    cout << name() << ".src("<< output <<")" << endl;
    limitCount++;
    out[0] = output;
    //out[1] = output; //What happens if using signals?? ( same question: "port.getAvalable...()>=2" )
  }
  
  smoc_firing_state start;
public:
  ConstSource(sc_module_name name, SMOC_ACTOR_CPARAM( T, value ), SMOC_ACTOR_CPARAM( int, limit ))
    : smoc_actor(name, start), output( value ), limitCount( 0 ) {
    start = ( (VAR(limitCount)<limit) && out(1)) >> CALL(ConstSource::src) >> start
      ;
  }
};


template<typename T>
class NonStrictAnd: public smoc_actor {
public:
  smoc_port_in<T>     op0;
  smoc_port_in<T>     op1;
  smoc_port_out<bool> out;
private:

  void goAnd(bool t) { //const
    cout << name() << ".goAnd(";
    if(op0.isValid()) cout << op0[0];
    else                 cout << "X";
    cout           << ", ";
    if(op1.isValid()) cout << op1[0];
    else                 cout << "X";
    cout           << ")" << endl;
    if( op0.isValid() && !op0[0] ) out[0] = 0;
    if( op1.isValid() && !op1[0] ) out[0] = 0;
    if( op0.isValid() &&  op1.isValid() ){
      T l = op0[0];
      T r = op1[0];
      cout << "valid: " << l << " " << r << " " << (l && r) << endl;
      out[0] = op0[0] && op1[0];
    }
  }

  void tick(int t1, int t2){
    cout << "## TICK ##" << endl;
  }

  smoc_firing_state start;
public:
  NonStrictAnd(sc_module_name name)
    : smoc_actor(name, start){

    start = (op0(1) && op1(1))                          >>
      out(1)                                            >>
      (SR_GO(NonStrictAnd::goAnd)(true) && SR_TICK(NonStrictAnd::tick)(2)(3))  >> start
      ;
  }
};



template<typename T, typename S>
class TerminalSink: public smoc_actor {
public:
  smoc_port_in<T>  in;

protected:
  S message;
  void terminal() const{
    cout << name() << ".terminal()" << endl;
    std::cout << message << in[0] << std::endl;
  }

  smoc_firing_state start;
public:
  TerminalSink(sc_module_name name, SMOC_ACTOR_CPARAM( S, message ))
    : smoc_actor(name, start), message(message) {
    start = in(1) >> CALL(TerminalSink::terminal) >> start
      ;
  }
};


class NonStrictAndTestBench
  :public smoc_graph{
protected:
  TerminalSink<bool, std::string > snk;
  NonStrictAnd<int> nsAnd;
  NonStrictAnd<bool> nsAnd2;
  ConstSource<int>  src0;
  ConstSource<int>  src1;
  ConstSource<bool>  src2;

public:
  NonStrictAndTestBench(sc_module_name name, int times)
    :smoc_graph(name),
     snk("Sink", std::string("\t") ),
     nsAnd("firstNSAnd"),
     nsAnd2("secondNSAnd"),
     src0("Source1", 1, times),
     src1("Source2", 2, times),
     src2("Source3", 1, times){
    connectNodePorts( src0.out,  nsAnd.op0, smoc_sr_signal<int>()  );
    connectNodePorts( src1.out,  nsAnd.op1, smoc_sr_signal<int>()  );
    connectNodePorts( src2.out,  nsAnd2.op0, smoc_sr_signal<bool>()  );
    connectNodePorts( nsAnd.out,  nsAnd2.op1, smoc_sr_signal<bool>()  );
    connectNodePorts( nsAnd2.out, snk.in,    smoc_sr_signal<bool>() );
  }
};
 

int sc_main (int argc, char **argv) {
  smoc_top_moc<NonStrictAndTestBench> nsa_tb("top", argc-1);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
