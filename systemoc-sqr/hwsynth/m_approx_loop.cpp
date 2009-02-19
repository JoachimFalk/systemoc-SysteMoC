#include "m_approx_loop.h"

m_approx_loop::m_approx_loop(sc_module_name name) : sc_module(name),
                                                    clk("clk"),
                                                    rst("rst"),
                                                    i1("i1"),
                                                    o1("o1"),
                                                    sqrloop_o1__approx_i1("sqrloop_o1__approx_i1"),
                                                    approx_o1__dup_i1("approx_o1__dup_i1"),
                                                    dup_o1__approx_i2("dup_o1__approx_i2"),
                                                    dup_o2__sqrloop_i2("dup_o2__sqrloop_i2")
{
  sqrloop = new m_sqrloop("sqrloop");
  sqrloop->clk(clk);
  sqrloop->rst(rst);
  sqrloop->i1(i1);
  sqrloop->i2(dup_o2__sqrloop_i2);
  sqrloop->o1(sqrloop_o1__approx_i1);
  sqrloop->o2(o1);
  
  approx = new m_approx("approx");
  approx->clk(clk);
  approx->rst(rst);
  approx->i1(sqrloop_o1__approx_i1);
  approx->i2(dup_o1__approx_i2);
  approx->o1(approx_o1__dup_i1);
  
  dup = new m_dup("dup");
  dup->clk(clk);
  dup->rst(rst);
  dup->i1(approx_o1__dup_i1);
  dup->o1(dup_o1__approx_i2);
  dup->o2(dup_o2__sqrloop_i2);
  
  i1.clk(clk);
  i1.rst(rst);
        
  o1.clk(clk);
  o1.rst(rst);

  sqrloop_o1__approx_i1.clk(clk);
  sqrloop_o1__approx_i1.rst(rst);
  
  approx_o1__dup_i1.clk(clk);
  approx_o1__dup_i1.rst(rst);
  approx_o1__dup_i1 << 2;
  
  dup_o1__approx_i2.clk(clk);
  dup_o1__approx_i2.rst(rst);
  
  dup_o2__sqrloop_i2.clk(clk);
  dup_o2__sqrloop_i2.rst(rst);
}
