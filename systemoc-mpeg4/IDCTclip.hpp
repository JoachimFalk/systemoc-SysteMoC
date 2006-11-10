#ifndef _INCLUDED_IDCTCLIP_HPP
#define _INCLUDED_IDCTCLIP_HPP

#define VERBOSE_IDCT_CLIP

class m_IDCTclip: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_in<int>  MIN;
  smoc_port_out<int> O;
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
		int O_internal = bound(MIN[0], I[0], MAX);
#ifdef VERBOSE_IDCT_CLIP
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "MIN[0] = " << MIN[0] << endl;
		cout << name() << ": " << "MAX = " << MAX << endl;
		cout << name() << ": " << "O[0] = " << O_internal << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: MIN[0] = %d\r\n",name(),MIN[0]);
		xil_printf("%s: MAX = %d\r\n",name(),MAX);
		xil_printf("%s: O[0] = %d\r\n",name(),O_internal);
#endif
#endif
#endif
    O[0] = O_internal; 
	}
  
    smoc_firing_state start;
public:
  m_IDCTclip(sc_module_name name,
             SMOC_ACTOR_CPARAM(int, MAX))
    : smoc_actor(name, start),
      MAX(MAX) {
    start = (I.getAvailableTokens() >= 1 &&
             MIN.getAvailableTokens() >= 1 )  >>
            (O.getAvailableSpace() >= 1)      >>
            CALL(m_IDCTclip::action0)  	      >> start;

  }
  virtual ~m_IDCTclip(){}
};

#endif // _INCLUDED_IDCTCLIP_HPP
