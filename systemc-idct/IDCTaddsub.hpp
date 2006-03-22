#ifndef _INCLUDED_IDCTADDSUB_HPP
#define _INCLUDED_IDCTADDSUB_HPP

#include <systemc.h>

class m_IDCTaddsub: public sc_module {
public:
  sc_fifo_in<int> I1;
  sc_fifo_in<int> I2;
  sc_fifo_out<int> O1;
  sc_fifo_out<int> O2;
private:
  const int  G;
  const int  OS;
  const int  ATTEN;
  
  void action0() {
    while (true) {
      int i1 = I1.read();
      int i2 = I2.read();
      O1.write(cal_rshift(G * (i1 + i2) + OS, ATTEN));
      O2.write(cal_rshift(G * (i1 - i2) + OS, ATTEN));
    }
  }
public:
  SC_HAS_PROCESS(m_IDCTaddsub);
  
  m_IDCTaddsub(sc_module_name name, int G, int OS, int ATTEN)
    : sc_module(name),
      G(G), OS(OS), ATTEN(ATTEN) {
    SC_THREAD(action0);
  }
};

#endif // _INCLUDED_IDCTADDSUB_HPP
