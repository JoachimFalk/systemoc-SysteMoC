/*  Library : Math Operations
 Block : Product, Divide
 Despcription : Multiply and divide scalars and nonscalars or multiply and
 invert matrices.
 */
#ifndef __INCLUDED__DEMUX__HPP__
#define __INCLUDED__DEMUX__HPP__

#include <systemoc/smoc_expr.hpp>

template<typename DATA_TYPE, int PORTS = 1>
class Demux: public smoc_actor {
public:

  smoc_port_in<DATA_TYPE> in;
  smoc_port_out<DATA_TYPE> out[PORTS];

  Demux(sc_module_name name) :
    smoc_actor(name, start)
  {
    Expr::Ex<bool>::type eOut(out[0](1));
    
    for (int i = 1; i < PORTS; i++) {
      eOut = eOut && out[i](1);
    }
    
    start = in(1) >> eOut >> CALL(Demux::process) >> start;
  }
protected:

  void process() {
#ifdef _DEBUG
    cout << "Demux " << name() ;
#endif
     DATA_TYPE tmp = in[0];
     for (int i = 0; i < PORTS; i++) {
       out[i][0] = tmp;
     }
#ifdef _DEBUG
      cout << " out:" << tmp << " ->" << endl;
#endif
  }
  smoc_firing_state start;
};

#endif// __INCLUDED__DEMUX__HPP__
