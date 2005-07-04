class IDCTscale: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O;
private:
  int  G;
  int  OS;

  void action0() {O[0] = OS + (G * I[0]); }
  smoc_firing_state start;

public:
  IDCTscale(sc_module_name name, 
	     int G, int OS)
    : smoc_actor(name, start),
      G(G),OS(OS)), {
    start = (I.getAvailableTokens() >= 1)    >>
		 call(&IDCTscale::action0) 	   >> start;
  }
};
