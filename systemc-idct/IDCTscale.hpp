#ifndef _INCLUDED_IDCTSCALE_HPP
#define _INCLUDED_IDCTSCALE_HPP

#include <systemc.h>

class m_IDCTscale: public sc_module {
public:
  sc_fifo_in<int> I;
  sc_fifo_out<int> O;
private:
  const int  G;
  const int  OS;
 
  void action0() {
    while (true) {
      O.write(OS + (G * I.read()));
      //std::cout<<name()<<"  M_IDCTscale Debugzeile hier ist I O wert: "<< I[0]<<" "<<O[0] <<"\n";
    }
  }
public:
  SC_HAS_PROCESS(m_IDCTscale);

  m_IDCTscale(sc_module_name name, int G, int OS)
    : sc_module(name),
      G(G), OS(OS) {
    SC_THREAD(action0);
  }
};
#endif // _INCLUDED_IDCTSCALE_HPP
