#ifndef _INCLUDED_IDCTCLIP_HPP
#define _INCLUDED_IDCTCLIP_HPP

#include <systemc.h>

class m_IDCTclip: public sc_module {
public:
  sc_fifo_in<int>  I;
  sc_fifo_in<int>  MIN;
  sc_fifo_out<int> O;
private:
  const int MAX;
  
  int bound(int a, int x, int b) {
    return x < a 
      ? a
      : ( x > b
          ? b
          : x );
  }

  void action0() {
    while (true) {
      //std::cout<<"M_clip debugzeile hier ist I wert: "<< I[0] <<"\n";
      O.write(bound(MIN.read(), I.read(), MAX));
    }
  }
public:
  SC_HAS_PROCESS(m_IDCTclip);

  m_IDCTclip(sc_module_name name, int MAX)
    : sc_module(name),
      MAX(MAX) {
    SC_THREAD(action0);
  }
};

#endif // _INCLUDED_IDCTCLIP_HPP
