#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>

//BISTDATA
class Bistdata: public smoc_actor {
public:
  smoc_port_out<int> out;
private:

  int testcost;

  smoc_firing_state start;
  smoc_firing_state end;

  void report(void){
	  std::cout<<name()<<" produced "<<testcost<<" Bytes!"<<std::endl;
  }

public:
  Bistdata(sc_module_name name, int testcost)
    : smoc_actor(name, start),testcost(testcost){

    start =	out(testcost)
    		>>CALL(Bistdata::report)
    		>> end;
  }
};
