#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

using Expr::field;
using std::endl;
using std::cout;


template <typename Data, int ReadTokenAtOnce=1, int WriteTokenAtOnce=ReadTokenAtOnce>
class generic_communication_node : public smoc_actor {

public:
  smoc_port_in<Data>   in;
  smoc_port_out<Data>  out;

private:
  smoc_firing_state main;

protected: 
  virtual void processCommunication(){

    for(int i=0; i<WriteTokenAtOnce; i++){
      out[i]=in[i%ReadTokenAtOnce];
    }

  }

public:
  generic_communication_node( sc_module_name name ) : smoc_actor( name, main ){
    main =
      in(ReadTokenAtOnce) >> out(WriteTokenAtOnce)            >>
      CALL(generic_communication_node::processCommunication)  >> main;
  }
};
