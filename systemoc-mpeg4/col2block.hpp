#ifndef _INCLUDED_COL2BLOCK_HPP
#define _INCLUDED_COL2BLOCK_HPP

#define VERBOSE_IDCT_COL2BLOCK

class m_col2block: public smoc_actor {
public:
  smoc_port_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  smoc_port_out<int> b;
private:
	

	void print_input() const {
#ifndef NDEBUG
#ifdef VERBOSE_IDCT_COL2BLOCK
#ifndef XILINX_EDK_RUNTIME
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R0[" << i << "] = " << R0[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R1[" << i << "] = " << R1[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R2[" << i << "] = " << R2[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R3[" << i << "] = " << R3[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R4[" << i << "] = " << R4[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R5[" << i << "] = " << R5[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R6[" << i << "] = " << R6[i] << endl;
		for ( int i = 0; i < 8; i++ )
			cout << name() << ": " << "R7[" << i << "] = " << R7[i] << endl;
#else
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R0[%d] = %d\r\n",name(),i,R0[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R1[%d] = %d\r\n",name(),i,R1[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R2[%d] = %d\r\n",name(),i,R2[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R3[%d] = %d\r\n",name(),i,R3[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R4[%d] = %d\r\n",name(),i,R4[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R5[%d] = %d\r\n",name(),i,R5[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R6[%d] = %d\r\n",name(),i,R6[i]);
		for ( int i = 0; i < 8; i++ )
			xil_printf("%s: R7[%d] = %d\r\n",name(),i,R7[i]);
#endif
#endif
#endif
	}

	
  void action0() {
    for ( int i = 0; i < 8; i++ )
      b[0*8 + i] = R0[i];
    for ( int i = 0; i < 8; i++ )
      b[1*8 + i] = R1[i];
    for ( int i = 0; i < 8; i++ )
      b[2*8 + i] = R2[i];
    for ( int i = 0; i < 8; i++ )
      b[3*8 + i] = R3[i];
    for ( int i = 0; i < 8; i++ )
      b[4*8 + i] = R4[i];
    for ( int i = 0; i < 8; i++ )
      b[5*8 + i] = R5[i];
    for ( int i = 0; i < 8; i++ )
      b[6*8 + i] = R6[i];
    for ( int i = 0; i < 8; i++ )
      b[7*8 + i] = R7[i];
#ifndef NDEBUG
		print_input();
#endif
  }
  
  smoc_firing_state start;
public:
  m_col2block(sc_module_name name)
    : smoc_actor(name, start) {
    start = (R0.getAvailableTokens() >= 8 &&
             R1.getAvailableTokens() >= 8 &&
             R2.getAvailableTokens() >= 8 &&
             R3.getAvailableTokens() >= 8 &&
             R4.getAvailableTokens() >= 8 &&
             R5.getAvailableTokens() >= 8 &&
             R6.getAvailableTokens() >= 8 &&
             R7.getAvailableTokens() >= 8 )   >>
            (b.getAvailableSpace() >= 64)     >>
            CALL(m_col2block::action0)        >> start;
  }
  
  virtual ~m_col2block(){}
};

#endif // _INCLUDED_COL2BLOCK_HPP
