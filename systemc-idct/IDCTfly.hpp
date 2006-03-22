#ifndef _INCLUDED_IDCTFLY_HPP
#define _INCLUDED_IDCTFLY_HPP

#include <systemc.h>

class m_IDCTfly: public sc_module {
public:
  sc_fifo_in<int> I1;
  sc_fifo_in<int> I2;
  sc_fifo_out<int> O1;
  sc_fifo_out<int> O2;
private:
  const int  W0;
  const int  OS;
  const int  W1;
  const int  W2;
  const int  ATTEN;
  
  void action0() {
    while (true) {
      int i1 = I1.read();
      int i2 = I2.read();
      int t = (W0 * (i1 + i2)) + OS;
      O1.write(cal_rshift(t + (i1 * W1), ATTEN));
      O2.write(cal_rshift(t + (i2 * W2), ATTEN));
    }
  }
public:
  SC_HAS_PROCESS(m_IDCTfly);
  
  m_IDCTfly(sc_module_name name, int W0, int OS, int W1, int W2, int ATTEN)
    : sc_module(name),
      W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
    SC_THREAD(action0);
  }
};

#endif // _INCLUDED_IDCTFLY_HPP
