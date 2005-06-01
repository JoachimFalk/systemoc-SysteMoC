#include <systemc.h>
#include <smoc_sr.hpp>
#include <cstdlib>
#include <iostream>

// ----------------------------------------------------------------------------
//  Application code follows
// ----------------------------------------------------------------------------

enum srevent_ty { EV_A, EV_B, EV_C, EV_D };

SC_MODULE(fsmAB) {
 public:
  smoc_sr_in<srevent_ty>  x;
  smoc_sr_out<srevent_ty> y;
 private:
  enum { ST_A, ST_B } state;
  
  void worker_method( void ) {
    if ( !x.event() ) {
      std::cout << "fsmAB State Init (True/-) => A" << std::endl;
      state = ST_A;
      return;
    }
    if ( !x.event() ) { return; /* no init */ }
    switch (state) {
    case ST_A:
      std::cout << "fsmAB State A";
      if ( x == EV_A ) {
	y = EV_B; state = ST_B; std::cout << " (A/B)=> B";
      }
      std::cout << std::endl;
      break;
    case ST_B:
      std::cout << "fsmAB State B";
      if ( x == EV_C ) {
	y = EV_D; state = ST_A; std::cout << " (C/D)=> A";
      }
      std::cout << std::endl;
      break;
    }
  }
 public:
  SC_CTOR(fsmAB) {
    SC_METHOD(worker_method);
    sensitive << x;
  }
};

SC_MODULE(fsmCD) {
 public:
  smoc_sr_in<srevent_ty>  x;
  smoc_sr_out<srevent_ty> y;
 private:
  enum { ST_C, ST_D } state;
  
  void worker_method( void ) {
    if ( !x.event() ) {
      std::cout << "fsmCD State Init (True/A) => C" << std::endl;
      y = EV_A; state = ST_C;
      return;
    }
    switch (state) {
    case ST_C:
      std::cout << "fsmCD State C";
      if ( x == EV_B ) {
	y = EV_C; state = ST_D; std::cout << " (B/C)=> D";
      }
      std::cout << std::endl;
      break;
    case ST_D:
      std::cout << "fsmCD State D";
      if ( x == EV_D ) {
	y = EV_A; state = ST_C; std::cout << " (D/A)=> C";
      }
      std::cout << std::endl;
      break;
    }
  }
 public:
  SC_CTOR(fsmCD) {
    SC_METHOD(worker_method);
    sensitive << x;
  }
};

int sc_main( int argc, char *argv[] ) {
  smoc_sr<srevent_ty> sAB2CD;
  smoc_sr<srevent_ty> sCD2AB;
  
  //sc_clock       clkgen( "clkgen", sc_time( 1000, SC_NS ) );
  fsmAB     fsm1( "fsm1" );
  fsmCD     fsm2( "fsm2" );
  
  fsm1.y(sAB2CD); fsm2.x(sAB2CD);
  fsm1.x(sCD2AB); fsm2.y(sCD2AB);
  
  sc_start( -1 );
  //sc_start( 1000, SC_NS );
  
  return 0;
}
