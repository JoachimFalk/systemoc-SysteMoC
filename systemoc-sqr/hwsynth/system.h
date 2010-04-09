#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <systemc.h>
#include "tb.h"
#include "m_approx_loop.h"

#include "hscd_floating.h"
#include "smoc_fifo.h"

SC_MODULE(TOP)
{
  sc_clock                clk;
  sc_signal< bool >       rst;
  
  smoc_fifo< hscd_float > a;
  smoc_fifo< hscd_float > b;
  
  tb            *i_tb;
  m_approx_loop *i_approx_loop;
  
  void initInstances();
  void deleteInstances();
  
  SC_CTOR(TOP) : clk( "clk", CLOCK_PERIOD, 0.50, 0, false ),
                 rst( "rst" ), a( "a" ), b( "b" )
  {
    initInstances();
  }
  
  ~TOP()
  {
    deleteInstances();
  }
};

#endif // SYSTEM_H_INCLUDED
