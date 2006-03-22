#ifndef _INCLUDED_COL2BLOCK_HPP
#define _INCLUDED_COL2BLOCK_HPP

#include <systemc.h>

#include "callib.hpp"

class m_col2block: public sc_module {
public:
  sc_fifo_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  sc_fifo_out<int> b;
private:
  void action() {
    while (true) {
      for ( int i = 0; i < 8; i++ )
        b.write(R0.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R1.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R2.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R3.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R4.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R5.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R6.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R7.read());
    }
  }
public:
  SC_HAS_PROCESS(m_col2block);
 
  m_col2block(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};

#endif // _INCLUDED_COL2BLOCK_HPP
