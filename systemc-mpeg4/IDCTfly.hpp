#ifndef _INCLUDED_IDCTFLY_HPP
#define _INCLUDED_IDCTFLY_HPP


class m_IDCTfly: public smoc_actor {
public:
  smoc_port_in<int> I1;
  smoc_port_in<int> I2;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
private:
  const int  W0;
  const int  OS;
  const int  W1;
  const int  W2;
  const int  ATTEN;
  
  void action0() {
    int t = (W0 * (I1[0] + I2[0])) + OS;
    O1[0] = cal_rshift(t + (I1[0] * W1), ATTEN);
    O2[0] = cal_rshift(t + (I2[0] * W2), ATTEN);

    std::cout<<name()<<"  M_IDCTfly Debugzeile hier ist I1 O1 wert: "<< I1[0]<<" "<<O1[0] <<"\n";
    std::cout<<name()<<"  M_IDCTfly Debugzeile hier ist I2 O2 wert: "<< I2[0]<<" "<<O2[0] <<"\n";
  }
  
  smoc_firing_state start;
public:
  m_IDCTfly(sc_module_name name, int W0, int OS, int W1, int W2, int ATTEN)
    : smoc_actor(name, start),
      W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
    start = (I1.getAvailableTokens() >= 1 &&
	     I2.getAvailableTokens() >= 1 ) >>
            (O1.getAvailableSpace() >= 1 &&
             O2.getAvailableSpace() >= 1)   >>
            CALL(m_IDCTfly::action0)  	    >> start;
  }

  virtual ~m_IDCTfly() {}
};


#endif // _INCLUDED_IDCTFLY_HPP
