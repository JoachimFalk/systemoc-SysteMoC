#ifndef _INCLUDED_BLOCK2ROW_HPP
#define _INCLUDED_BLOCK2ROW_HPP

#include <systemc.h>

#include "callib.hpp"

class m_block2row: public sc_module {
public:
  sc_fifo_in<int>  b;
  sc_fifo_out<int> C0, C1, C2, C3, C4, C5, C6, C7;
private:
  void action() {
    while (true) {
      C0.write(b.read());
      C1.write(b.read());
      C2.write(b.read());
      C3.write(b.read());
      C4.write(b.read());
      C5.write(b.read());
      C6.write(b.read());
      C7.write(b.read());
    }
  }
public:
  SC_HAS_PROCESS(m_block2row);
  
  m_block2row(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};
#endif // _INCLUDED_BLOCK2ROW_HPP
