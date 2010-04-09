#ifndef M_APPROX_LOOP_H
#define M_APPROX_LOOP_H

#include <systemc.h>
#include <esc.h>
#include "cynthhl.h"

#include "hscd_floating.h"
#include "smoc_fifo.h"

#include "m_sqrloop.h"
#include "m_approx.h"
#include "m_dup.h"

SC_MODULE(m_approx_loop)
{
  public:
    sc_in< bool >                clk;
    sc_in< bool >                rst;
    
    smoc_fifo< hscd_float >::in  i1;
    smoc_fifo< hscd_float >::out o1;
    
    smoc_fifo<hscd_float> sqrloop_o1__approx_i1;
    smoc_fifo<hscd_float> approx_o1__dup_i1;
    smoc_fifo<hscd_float> dup_o1__approx_i2;
    smoc_fifo<hscd_float> dup_o2__sqrloop_i2;
    
    m_sqrloop *sqrloop;
    m_approx  *approx;
    m_dup     *dup;
    
    SC_CTOR(m_approx_loop);
};

#endif // M_APPROX_LOOP_H
