
#include <systemc.h>
#include <smoc_event.hpp>


class B: public sc_module{
public:
  static smoc_event or1;
  static smoc_event or2;
  static smoc_event and1;
  static smoc_event and2;
  static smoc_event and3;

  SC_CTOR(B){
    SC_THREAD(proc);
  }
  void proc(){
    while(1){
      smoc_event_and_list ol = or1 & or2;
      //      cerr << "size: "<< ol.size()<< endl;
      ol &= and1;
      //      cerr << "size: "<< ol.size()<< endl;
      smoc_wait(ol);
      smoc_reset(ol);
      cerr << "B called at: "<< sc_simulation_time() <<endl;
    }
  }
};

class C: public sc_module{
public:
  static smoc_event or1;
  static smoc_event or2;
  static smoc_event and1;
  static smoc_event and2;
  static smoc_event and3;

  SC_CTOR(C){
    SC_THREAD(proc);
  }
  void proc(){
    while(1){
      smoc_event_or_list ol=or1 | or2;
      smoc_wait(ol);
      smoc_reset(ol);
      cerr << "C called at: "<< sc_simulation_time() <<endl;
    }
  }
};

smoc_event B::or1;
smoc_event B::or2;
smoc_event B::and1;
smoc_event B::and2;
smoc_event B::and3;

smoc_event C::or1;
smoc_event C::or2;
smoc_event C::and1;
smoc_event C::and2;
smoc_event C::and3;

class A: public sc_module{
public:
  SC_CTOR(A){
    SC_THREAD(proc);
  }
  void proc(){
    wait(1,SC_NS);
    smoc_notify(B::or1);
    wait(1,SC_NS);
    smoc_notify(B::or2);
    wait(1,SC_NS);
    smoc_notify(B::and1);
    wait(1,SC_NS);
    smoc_notify(B::and2);
    smoc_notify(C::or1);
    wait(1,SC_NS);
    smoc_notify(B::and3);
    wait(1,SC_NS);
    smoc_notify(B::or2);
    wait(1,SC_NS);
    smoc_notify(B::or1);
    wait(1,SC_NS);
    smoc_notify(B::and2);
    wait(1,SC_NS);
    smoc_notify(B::and1);
    smoc_notify(C::or2);
  }
};


int sc_main(int ac,char *av[])
{
  A a("a");
  B b("b");
  C c("c");
  sc_start();
  return 0;
}
