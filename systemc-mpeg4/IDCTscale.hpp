#ifndef _INCLUDED_IDCTSCALE_HPP
#define _INCLUDED_IDCTSCALE_HPP


class m_IDCTscale: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O;
private:
  const int  G;
  const int  OS;
  
  void action0() { O[0] = OS + (G * I[0]); }
  
  smoc_firing_state start;
public:
  m_IDCTscale(sc_module_name name, int G, int OS)
    : smoc_actor(name, start),
      G(G), OS(OS) {
    start = (I.getAvailableTokens() >= 1) >>
            (O.getAvailableSpace() >= 1)  >>
            call(&m_IDCTscale::action0)   >> start;
  }
};
#endif // _INCLUDED_IDCTSCALE_HPP
