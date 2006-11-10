#ifndef _INCLUDED_IDCTFLY_HPP
#define _INCLUDED_IDCTFLY_HPP

#define VERBOSE_IDCT_FLY

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
		int O1_internal = cal_rshift(t + (I1[0] * W1), ATTEN);
		int O2_internal = cal_rshift(t + (I2[0] * W2), ATTEN);
    O1[0] = O1_internal;
    O2[0] = O2_internal;
#ifdef VERBOSE_IDCT_FLY
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
  m_IDCTfly(sc_module_name name,
            SMOC_ACTOR_CPARAM(int, W0),
	    SMOC_ACTOR_CPARAM(int, OS),
	    SMOC_ACTOR_CPARAM(int, W1),
	    SMOC_ACTOR_CPARAM(int, W2),
	    SMOC_ACTOR_CPARAM(int, ATTEN))
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
