// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

template<typename T>
class ConstSource: public smoc_actor {
public:
  smoc_port_out<T> out;

  ConstSource(sc_module_name name, T value, int limit )
    : smoc_actor(name, start),
      output( value ),
      limitCount( 0 )
  {
    SMOC_REGISTER_CPARAM(value);
    SMOC_REGISTER_CPARAM(limit);

    start = ( (VAR(limitCount)<limit) && out(1)) >>
      CALL(ConstSource::src)                     >>
      start
      ;
  }
private:
  T   output;
  int limitCount;
  void src() {
    cout << name() << ".src("<< output <<")" << endl;
    limitCount++;
    out[0] = output;
    //out[1] = output; //What happens if using signals??
    //                  ( same question: "port.getAvailable...()>=2" )
  }
  
  smoc_firing_state start;
};

