#ifndef _INCLUDED_IDCTSCALE_HPP
#define _INCLUDED_IDCTSCALE_HPP


class m_IDCTscale: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O;
private:
  const int  G;
  const int  OS;
  
  void action0() {
    O[0] = OS + (G * I[0]);
    
    //std::cout<<name()<<"  M_IDCTscale Debugzeile hier ist I O wert: "<< I[0]<<" "<<O[0] <<"\n";
  }
  
  smoc_firing_state start;
public:
  m_IDCTscale(sc_module_name name,
              SMOC_ACTOR_CPARAM(int, G),
	      SMOC_ACTOR_CPARAM(int, OS))
    : smoc_actor(name, start),
      G(G), OS(OS) {
    start = (I.getAvailableTokens() >= 1) >>
            (O.getAvailableSpace() >= 1)  >>
            CALL(m_IDCTscale::action0)    >> start;
  }
};
#endif // _INCLUDED_IDCTSCALE_HPP
