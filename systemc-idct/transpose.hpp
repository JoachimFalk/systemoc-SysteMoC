#ifndef _INCLUDED_TRANSPOSE_HPP
#define _INCLUDED_TRANSPOSE_HPP

#include <systemc.h>

#include "callib.hpp"

class m_transpose: public sc_module {
public:
  sc_fifo_in<int>  I0, I1, I2, I3, I4, I5, I6, I7;
  sc_fifo_out<int> O0, O1, O2, O3, O4, O5, O6, O7;
private:
  void action0() {
    O0.write(I0.read()); O0.write(I1.read()); O0.write(I2.read()); O0.write(I3.read());
    O0.write(I4.read()); O0.write(I5.read()); O0.write(I6.read()); O0.write(I7.read());
  }
  void action1() {
    O1.write(I0.read()); O1.write(I1.read()); O1.write(I2.read()); O1.write(I3.read());
    O1.write(I4.read()); O1.write(I5.read()); O1.write(I6.read()); O1.write(I7.read());
  }
  void action2() {
    O2.write(I0.read()); O2.write(I1.read()); O2.write(I2.read()); O2.write(I3.read());
    O2.write(I4.read()); O2.write(I5.read()); O2.write(I6.read()); O2.write(I7.read());
  }
  void action3() {
    O3.write(I0.read()); O3.write(I1.read()); O3.write(I2.read()); O3.write(I3.read());
    O3.write(I4.read()); O3.write(I5.read()); O3.write(I6.read()); O3.write(I7.read());
  }
  void action4() {
    O4.write(I0.read()); O4.write(I1.read()); O4.write(I2.read()); O4.write(I3.read());
    O4.write(I4.read()); O4.write(I5.read()); O4.write(I6.read()); O4.write(I7.read());
  }
  void action5() {
    O5.write(I0.read()); O5.write(I1.read()); O5.write(I2.read()); O5.write(I3.read());
    O5.write(I4.read()); O5.write(I5.read()); O5.write(I6.read()); O5.write(I7.read());
  }
  void action6() {
    O6.write(I0.read()); O6.write(I1.read()); O6.write(I2.read()); O6.write(I3.read());
    O6.write(I4.read()); O6.write(I5.read()); O6.write(I6.read()); O6.write(I7.read());
  }
  void action7() {
    O7.write(I0.read()); O7.write(I1.read()); O7.write(I2.read()); O7.write(I3.read());
    O7.write(I4.read()); O7.write(I5.read()); O7.write(I6.read()); O7.write(I7.read());
  }
  void action() {
    while (true) {
      action0();
      action1();
      action2();
      action3();
      action4();
      action5();
      action6();
      action7();
    }
  }
public:
  SC_HAS_PROCESS(m_transpose);

  m_transpose(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};
#endif // _INCLUDED_TRANSPOSE_HPP
