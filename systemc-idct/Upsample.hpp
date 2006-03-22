#ifndef _INCLUDED_UPSAMPLE_HPP
#define _INCLUDED_UPSAMPLE_HPP

#include <systemc.h>

#include "callib.hpp"

class m_Upsample: public sc_module {
public:
  sc_fifo_in<int>  I;
  sc_fifo_out<int> O;
private:
  const int factor;
  
  void action() {
    while (true) {
      int mem = I.read();
      
      for (int state = 0; state < factor; ++state)
        O.write(mem);
    }
  }
public:
  SC_HAS_PROCESS(m_Upsample);

  m_Upsample(sc_module_name name, int factor)
    : sc_module(name),
      factor(factor) {
    SC_THREAD(action);
  }
};

#endif // _INCLUDED_UPSAMPLE_HPP
