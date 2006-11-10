#ifndef _INCLUDED_IDCTSCALE_HPP
#define _INCLUDED_IDCTSCALE_HPP

#define VERBOSE_IDCT_SCALE

class m_IDCTscale: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O;
private:
  const int  G;
  const int  OS;
  
  void action0() {
		int temp = OS + (G * I[0]);
#ifdef VERBOSE_IDCT_SCALE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "O[0] = " << temp << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: O[0] = %d\r\n",name(),temp);
#endif
#endif
#endif
    O[0] = temp;
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
