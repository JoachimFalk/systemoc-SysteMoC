// vim: set sw=2 ts=8:

#include <stdlib.h>
#include <systemc.h>
#include <iostream>

#define period 10
#include "design.hpp"

SC_MODULE(stim) {
 private:
  sc_clock   iclkgen;
  sc_in_clk  iclk;

  void clk_thread( void ) {
    clk = iclk;
  }
  
  void stimgen_thread( void ) {
    reset = 1;
    wait();
    wait();
    reset = 0;
    while ( false )
      ;
  }
  
 public:
  sc_out< bool >	clk;
  sc_out< bool >	reset;

  SC_CTOR(stim)
    : iclkgen("clkgen", 10, SC_NS ), iclk(iclkgen) {
    SC_CTHREAD(stimgen_thread,iclk.pos());
    //SC_THREAD(stimgen_thread);
    //sensitive << clkgen.posedge_event();
    SC_METHOD(clk_thread);
    sensitive << iclk;
  }
};

SC_MODULE(display) {
 public:
  sc_in_clk   clk;
  sc_in<bool> led;
 private:
  int l;
  
  void display_method( void ) {
    std::cout << led;
    if ( ++l == 20  ) {
      std::cout << std::endl;
      l = 0;
    }
  }
  
 public:
  SC_CTOR(display):
    l(0) {
    SC_METHOD(display_method);
    sensitive << clk.pos();
  }
};

int sc_main( int argc, char *argv[] ) {
  counter c( "my_counter" );
  stim    s( "my_stim" );
  display d( "my_display" );
  
  sc_signal<bool> s_clk;
  s.clk( s_clk ); /* out */
  c.clk( s_clk ); /* in */
  d.clk( s_clk ); /* in */

  sc_signal<bool> s_reset;
  s.reset( s_reset ); /* out */
  c.reset( s_reset ); /* in */

  sc_signal<bool> s_led;
  c.led( s_led ); /* out */
  d.led( s_led ); /* in */

  sc_start( -1 );
  //sc_start( 1000, SC_NS );
  
  return 0;
}
