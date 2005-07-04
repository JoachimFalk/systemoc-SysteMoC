class IDCTaddsub: public smoc_actor {
public:
  smoc_port_in<int> I1;
  smoc_port_in<int> I2;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
private:
  int  G;
  int  OS;
  int  ATTEN;
  
  void action0() {
    O1[0] = (G * (I1[0] + I2[0]) + OS)) >> ATTEN; 
    O2[0] = (G * (I1[0] - I2[0]) + OS)) >> ATTEN;}
  
  smoc_firing_state start;

public:
  IDCTfly(sc_module_name name)
    : smoc_actor(name, start),
      G(G), OS(OS), ATTEN(ATTEN) {
    start = (I1.getAvailableTokens() >= 1 &&
		 I2.getAvailableTokens() >= 1 ) >>
		 call(&IDCTaddsub::action0) 	   >> start;
  }
};


