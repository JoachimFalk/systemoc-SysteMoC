class IDCTfly: public smoc_actor {
public:
  smoc_port_in<int> I1;
  smoc_port_in<int> I2;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
private:
  int  W0;
  int  OS;
  int  W1;
  int  W2;
  int  ATTEN;
  int  t;
  
  void action0() {
    t = (W0 * (I1[0] + I2[0])) + OS;
    O1[0] = (t + (I1[0] * W1)) >> ATTEN; 
    O2[0] = (t + (I2[0] * W2)) >> ATTEN;
  }
  
  smoc_firing_state start;
public:
  IDCTfly(sc_module_name name)
    : smoc_actor(name, start),
      W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
    start = (I1.getAvailableTokens() >= 1 &&
		 I2.getAvailableTokens() >= 1 ) >>
		 call(&IDCTfly::action0) 	   >> start;
  }
};
