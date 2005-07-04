class IDCTclip: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_in<int> MIN;
  smoc_port_out<int> O;
private:
  int bound(int a, int x, int b) {
      if (x<a) return a;
      elseif (x>b) return b;
      else return x;
  }
  int  max;
  
  void action0() {O[0] = bound(MIN, I[0], max); }
  smoc_firing_state start;

public:
  IDCTclip(sc_module_name name, 
	     int MAX)
    : smoc_actor(name, start),
      max(MAX), {
    start = (I.getAvailableTokens() >= 1 &&
		 MIN.getAvailableTokens() >= 1 ) >>
		 call(&IDCTclip::action0) 	   >> start;
  }
};

