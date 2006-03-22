#ifndef _INCLUDED_MIN_DUPLEX_HPP
#define _INCLUDED_MIN_DUPLES_HPP

#include <systemc.h>

class m_MIN_duplex: public sc_module {
public:
  sc_fifo_in<int> I;
  sc_fifo_out<int> O0;
  sc_fifo_out<int> O1;
  sc_fifo_out<int> O2;
  sc_fifo_out<int> O3;
  sc_fifo_out<int> O4;
  sc_fifo_out<int> O5;
  sc_fifo_out<int> O6;
  sc_fifo_out<int> O7;

private:
  
  void action() {
    while (true) {
      int in = I.read();
      O0.write(in);
      O1.write(in);
      O2.write(in);
      O3.write(in);
      O4.write(in);
      O5.write(in);
      O6.write(in);
      O7.write(in);
    }
  }
public:
  SC_HAS_PROCESS(m_MIN_duplex);
 
  m_MIN_duplex(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};

#endif // _INCLUDED_MIN_DUPLEX_HPP
