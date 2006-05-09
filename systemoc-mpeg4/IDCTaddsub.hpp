
#ifndef _INCLUDED_IDCTADDSUB_HPP
#define _INCLUDED_IDCTADDSUB_HPP
class m_IDCTaddsub: public smoc_actor {
public:
  smoc_port_in<int> I1;
  smoc_port_in<int> I2;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
private:
  const int  G;
  const int  OS;
  const int  ATTEN;
  
  void action0() {
    O1[0] = cal_rshift(G * (I1[0] + I2[0]) + OS, ATTEN);
    O2[0] = cal_rshift(G * (I1[0] - I2[0]) + OS, ATTEN);

    //std::cout<<name()<<"  M_IDCTaddsub Debugzeile hier ist I1 O1 wert: "<< I1[0]<<" "<<O1[0] <<"\n";
    //std::cout<<name()<<"  M_IDCTaddsub Debugzeile hier ist I2 O2 wert: "<< I2[0]<<" "<<O2[0] <<"\n";
  }
  
  smoc_firing_state start;
public:
  m_IDCTaddsub(sc_module_name name,
               SMOC_ACTOR_CPARAM(int, G),
	       SMOC_ACTOR_CPARAM(int, OS),
	       SMOC_ACTOR_CPARAM(int, ATTEN))
    : smoc_actor(name, start),
      G(G), OS(OS), ATTEN(ATTEN) {
    start = (I1.getAvailableTokens() >= 1 &&
             I2.getAvailableTokens() >= 1)    >>
            (O1.getAvailableSpace()  >= 1 &&
             O2.getAvailableSpace()  >= 1)    >>
            CALL(m_IDCTaddsub::action0)       >> start;
  }
  
  virtual ~m_IDCTaddsub(){}
};

#endif // _INCLUDED_IDCTADDSUB_HPP
