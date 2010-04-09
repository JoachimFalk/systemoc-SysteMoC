#include "system.h"

void TOP::initInstances()
{
  i_tb = new tb( "i_tb" );
  i_tb->clk(clk);
  i_tb->rst(rst);
  i_tb->out(a);
  i_tb->in(b);
  
  i_approx_loop = new m_approx_loop( "i_approx_loop" );
  i_approx_loop->clk(clk);
  i_approx_loop->rst(rst);
  i_approx_loop->i1(a);
  i_approx_loop->o1(b);
  
  a.clk(clk);
  a.rst(rst);
  
  b.clk(clk);
  b.rst(rst);
}

void TOP::deleteInstances()
{
  if (i_tb) {
    delete i_tb;
    i_tb = NULL;
  }
  if (i_approx_loop) {
    delete i_approx_loop;
    i_approx_loop = NULL;
  }
}
