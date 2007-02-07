// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_port.hpp>

template<typename T>
class NonStrictAnd: public smoc_actor {
public:
  smoc_port_in<T>     op0;
  smoc_port_in<T>     op1;
  smoc_port_out<bool> out;
private:

  void goAnd(bool t) { //const
    if( op0.isValid() || op1.isValid() ){
      cout << name() << ".goAnd(";
      if(op0.isValid()) cout << op0[0];
      else                 cout << "X";
      cout           << ", ";
      if(op1.isValid()) cout << op1[0];
      else                 cout << "X";
      cout           << ")" << endl;
    }
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
      (SR_GO(NonStrictAnd::goAnd)(true)                 &&
       SR_TICK(NonStrictAnd::tick)(2)(3))               >>
      start
      ;
  }
};
