
#ifndef _INCLUDED_IDCTADDSUB_HPP
#define _INCLUDED_IDCTADDSUB_HPP


#define VERBOSE_IDCT_ADDSUB


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
		int O1_internal = cal_rshift(G * (I1[0] + I2[0]) + OS, ATTEN);
		int O2_internal = cal_rshift(G * (I1[0] - I2[0]) + OS, ATTEN);
    O1[0] = O1_internal;
    O2[0] = O2_internal;
#ifdef VERBOSE_IDCT_ADDSUB
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I1[0] = " << I1[0] << endl;
		cout << name() << ": " << "I2[0] = " << I2[0] << endl;
		cout << name() << ": " << "O1[0] = " << O1_internal << endl;
		cout << name() << ": " << "O2[0] = " << O2_internal << endl;
#else
		xil_printf("%s: I1[0] = %d\r\n",name(),I1[0]);
		xil_printf("%s: I2[0] = %d\r\n",name(),I2[0]);
		xil_printf("%s: O1[0] = %d\r\n",name(),O1_internal);
		xil_printf("%s: O2[0] = %d\r\n",name(),O2_internal);

#endif
#endif
#endif
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
