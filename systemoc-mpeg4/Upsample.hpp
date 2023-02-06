#ifndef _INCLUDED_UPSAMPLE_HPP
#define _INCLUDED_UPSAMPLE_HPP

#include "callib.hpp"

//#define VERBOSE_IDCT_UPSAMPLE

class m_Upsample: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_out<int> O;
private:
  int  mem;
  
  void upsampleStart() { 
#ifdef VERBOSE_IDCT_UPSAMPLE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "O[0] = " << mem << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: O[0] = %d\r\n",name(),mem);
#endif
#endif
#endif
		O[0] = mem = I[0]; 
	}
  void upsampleRest()  { 
#ifdef VERBOSE_IDCT_UPSAMPLE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "O[0] = " << mem << endl;
#else
		xil_printf("%s: O[0] = %d\r\n",name(),mem);
#endif
#endif
#endif
		O[0] = mem;        
	}
  
  smoc_firing_state s0, s1, s2, s3, s4, s5, s6, s7;
public:
  m_Upsample(sc_module_name name): smoc_actor(name, s0) {
    s0 = (I.getAvailableTokens() >= 1)   >>
         (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleStart) >> s1;
    s1 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s2;
    s2 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s3;
    s3 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s4;
    s4 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s5;
    s5 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s6;
    s6 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s7;
    s7 = (O.getAvailableSpace()  >= 1)   >>
         CALL(m_Upsample::upsampleRest)  >> s0;
  }
};

#endif // _INCLUDED_UPSAMPLE_HPP
