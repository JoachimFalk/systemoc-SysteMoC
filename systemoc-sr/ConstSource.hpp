// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>

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
    //out[1] = output; //What happens if using signals??
    //                  ( same question: "port.getAvalable...()>=2" )
  }
  
  smoc_firing_state start;
public:
  ConstSource(sc_module_name name,
	      SMOC_ACTOR_CPARAM( T, value ),
	      SMOC_ACTOR_CPARAM( int, limit ))
    : smoc_actor(name, start),
      output( value ),
      limitCount( 0 ) {
    start = ( (VAR(limitCount)<limit) && out(1)) >>
      CALL(ConstSource::src)                     >>
      start
      ;
  }
};

