// vim: set sw=2 ts=8:

#include <systemc.h>
#include <cstdlib>
#include <iostream>

typedef sc_uint<16> tokentype_ty;

SC_MODULE(design_mul2) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      out.write( in.read() * 2 );
    }
  }
  
 public:
  SC_CTOR(design_mul2) {
    SC_THREAD(worker_thread);
  }
};
  
SC_MODULE(design_expand) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out1;
  sc_fifo_out< tokentype_ty > out2;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      tokentype_ty t = in.read();
      out1.write(t);
      out2.write(t);
    }
  }
  
 public:
  SC_CTOR(design_expand) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_sub1) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      out.write( in.read() - 1 );
    }
  }
  
 public:
  SC_CTOR(design_sub1) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_pot2) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      tokentype_ty t = in.read();
      out.write( t*t );
    }
  }
  
 public:
  SC_CTOR(design_pot2) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_add3) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      out.write( in.read() + 3 );
    }
  }
  
 public:
  SC_CTOR(design_add3) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_mul3) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      out.write( in.read() * 3 );
    }
  }
  
 public:
  SC_CTOR(design_mul3) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_xaddy) {
 public:
  sc_in_clk		      clk;
  sc_fifo_in< tokentype_ty >  inx;
  sc_fifo_in< tokentype_ty >  iny;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      // consume tokens
      tokentype_ty t1 = inx.read();
      tokentype_ty t2 = iny.read();
      
      // do some work
      tokentype_ty t  = t1+t2;
      // simulate time needed for adding values
      wait(clk.posedge_event());
      
      // produce tokens
      out.write( t1 + t2 );
    }
  }
  
 public:
  SC_CTOR(design_xaddy) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(design_top) {
 public:
  sc_in_clk		 clk;
  sc_fifo_in< tokentype_ty >  in;
  sc_fifo_out< tokentype_ty > out;
 private:
  sc_fifo< tokentype_ty > f1, f2, f3, f4, f5, f6, f7, f8;
  
  design_mul2    *i_mul2;
  design_expand  *i_expand;
  design_sub1    *i_sub1;
  design_pot2    *i1_pot2;
  design_add3    *i_add3;
  design_mul3 *i_mul3;
  design_pot2    *i2_pot2;
  design_xaddy     *i_xaddy;

/*
  void schedule( void ) {
    sc_event x;
    
    while ( 1 ) {
      i_mul2->activate(x);
      wait(x);
      i_expand->activate(x);
      wait(x);
      i_sub1->activate(x);
      wait(x);
      i1_pot2->activate(x);
      wait(x);
      i_add3->activate(x);
      wait(x);
      i_mul3->activate(x);
      wait(x);
      i2_pot2->activate(x);
      wait(x);
      i_xaddy->activate(x);
      wait(x);
    }
  }*/
  
 public:
  SC_CTOR(design_top) {
    i_mul2    = new  design_mul2( "i_mul2" );
    i_expand  = new design_expand( "i_expand" );
    i_sub1    = new design_sub1( "i_sub1" );
    i1_pot2   = new design_pot2( "i1_pot2" );
    i_add3    = new design_add3( "i_add3" );
    i_mul3 = new design_mul3( "i_mul3" );
    i2_pot2   = new design_pot2( "i2_pot2" );
    i_xaddy     = new design_xaddy( "i_xaddy" );
    i_mul2->clk(clk);
    i_expand->clk(clk);
    i_sub1->clk(clk);
    i1_pot2->clk(clk);
    i_add3->clk(clk);
    i_mul3->clk(clk);
    i2_pot2->clk(clk);
    i_xaddy->clk(clk);
    
    i_mul2->in(in);
    i_mul2->out(f1); i_expand->in(f1);
    
    i_expand->out1(f2); i_sub1->in(f2);
    i_sub1->out(f3); i1_pot2->in(f3);
    i1_pot2->out(f4); i_xaddy->inx(f4);
    
    i_expand->out2(f5); i_add3->in(f5);
    i_add3->out(f6); i_mul3->in(f6);
    i_mul3->out(f7); i2_pot2->in(f7);
    i2_pot2->out(f8); i_xaddy->iny(f8);
    
    i_xaddy->out(out);
    
    //SC_CTHREAD(worker_thread,clk.pos());
    //SC_THREAD(schedule);
    //sensitive << clkgen.posedge_event();
    //SC_METHOD(clk_thread);
    //sensitive << clk;
  }
};

SC_MODULE(token_source) {
 public:
  sc_in_clk		 clk;
  sc_fifo_out< tokentype_ty > out;
  
 private:
  void worker_thread( void ) {
    tokentype_ty x = 0;
    
    while ( 1 ) {
      out.write( x++ );
    }
  }
  
 public:
  SC_CTOR(token_source) {
    SC_THREAD(worker_thread);
  }
};

SC_MODULE(token_sink) {
 public:
  sc_in_clk		 clk;
  sc_fifo_in< tokentype_ty > in;
  
 private:
  void worker_thread( void ) {
    while ( 1 ) {
      std::cout << in.read() << std::endl;
    }
  }
  
 public:
  SC_CTOR(token_sink) {
    SC_THREAD(worker_thread);
    // SC_CTHREAD(worker_thread,clk.pos());
  }
};

int sc_main( int argc, char *argv[] ) {
  sc_clock clkgen("clkgen", 10, SC_NS);
  sc_fifo< tokentype_ty >  fsrc( 16 ), fsink( 16 );
  
  design_top   top  ( "design_top" );
  token_source tsrc ( "token_source" );
  token_sink   tsink( "token_sink" );
  
  tsrc.clk( clkgen );
  top.clk( clkgen );
  tsink.clk( clkgen );

  tsrc.out(fsrc);
  top.in(fsrc);
  top.out(fsink);
  tsink.in(fsink);
  
  sc_start( -1 );
  //sc_start( 1000, SC_NS );
  
  return 0;
}
