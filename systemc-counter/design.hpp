// vim: set sw=2 ts=8:

#include <systemc.h>

SC_MODULE(counter) {
 public:
  sc_in_clk                   clk;
  sc_in<bool>                 reset;
  sc_out<bool>                led;
 private:
  void worker_thread( void ) {
    int dutycycle = 0;
    int counter   = 0;

    wait();
    while ( 1 ) {
      wait();
      if ( counter == dutycycle )
	led = 0;
      else if ( counter == 0 )
	led = 1;
      wait();
      dutycycle = counter != period - 1
	      ? dutycycle
	      : ( dutycycle == period ? 0 : dutycycle + 1 );
      counter = counter == period - 1
	      ? 0
	      : counter + 1;
    }
  }
  
 public:
  SC_CTOR(counter) {
    SC_CTHREAD(worker_thread, clk.pos() );
    watching(reset.delayed() == true);
  }
};
