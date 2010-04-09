#ifndef TB_H_INCLUDED
#define TB_H_INCLUDED

#include <systemc.h>
#include <esc.h>
#include "hscd_floating.h"
#include "smoc_fifo.h"

SC_MODULE(tb)
{
 public:
  sc_in< bool >                clk;
  sc_out< bool >               rst;
  
  smoc_fifo< hscd_float >::out out;
  smoc_fifo< hscd_float >::in  in;
  
  SC_CTOR(tb) : clk("clk"), rst("rst"), out("out"), in("in") {
    SC_CTHREAD(source, clk.neg());
    SC_CTHREAD(sink, clk.neg());
    watching(rst.delayed() == true);
    
    out.clk(clk);
    out.rst(rst);
    
    in.clk(clk);
    in.rst(rst);
  }

 private:

  void source();
  void sink();
};

#endif // TB_H_INCLUDED
