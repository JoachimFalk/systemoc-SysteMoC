
#include <callib.hpp>

class m_IDCTtranspose: public smoc_actor {
// The actor has 16 Ports.
public: 
   smoc_port_in<int> I0;
   smoc_port_in<int> I1;
   smoc_port_in<int> I2;
   smoc_port_in<int> I3;
   smoc_port_in<int> I4;
   smoc_port_in<int> I5;
   smoc_port_in<int> I6;
   smoc_port_in<int> I7;
   smoc_port_out<int> O0;
   smoc_port_out<int> O1;
   smoc_port_out<int> O2;
   smoc_port_out<int> O3;
   smoc_port_out<int> O4;
   smoc_port_out<int> O5;
   smoc_port_out<int> O6;
   smoc_port_out<int> O7;

// The actor has 1 Parameters and 0 Variable declarations.
private: 
   const  int;
   
// The actor has 8 Actions and 0 Guards.
private:
void m_IDCTtranspose::Out0(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O0[0] = i0;
   O0[1] = i1;
   O0[2] = i2;
   O0[3] = i3;
   O0[4] = i4;
   O0[5] = i5;
   O0[6] = i6;
   O0[7] = i7;
}
void m_IDCTtranspose::Out1(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O1[0] = i0;
   O1[1] = i1;
   O1[2] = i2;
   O1[3] = i3;
   O1[4] = i4;
   O1[5] = i5;
   O1[6] = i6;
   O1[7] = i7;
}
void m_IDCTtranspose::Out2(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O2[0] = i0;
   O2[1] = i1;
   O2[2] = i2;
   O2[3] = i3;
   O2[4] = i4;
   O2[5] = i5;
   O2[6] = i6;
   O2[7] = i7;
}
void m_IDCTtranspose::Out3(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O3[0] = i0;
   O3[1] = i1;
   O3[2] = i2;
   O3[3] = i3;
   O3[4] = i4;
   O3[5] = i5;
   O3[6] = i6;
   O3[7] = i7;
}
void m_IDCTtranspose::Out4(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O4[0] = i0;
   O4[1] = i1;
   O4[2] = i2;
   O4[3] = i3;
   O4[4] = i4;
   O4[5] = i5;
   O4[6] = i6;
   O4[7] = i7;
}
void m_IDCTtranspose::Out5(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O5[0] = i0;
   O5[1] = i1;
   O5[2] = i2;
   O5[3] = i3;
   O5[4] = i4;
   O5[5] = i5;
   O5[6] = i6;
   O5[7] = i7;
}
void m_IDCTtranspose::Out6(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O6[0] = i0;
   O6[1] = i1;
   O6[2] = i2;
   O6[3] = i3;
   O6[4] = i4;
   O6[5] = i5;
   O6[6] = i6;
   O6[7] = i7;
}
void m_IDCTtranspose::Out7(void) {
// The action has 0 local variable declarations.
   int i0 = I0[0];
   int i1 = I1[0];
   int i2 = I2[0];
   int i3 = I3[0];
   int i4 = I4[0];
   int i5 = I5[0];
   int i6 = I6[0];
   int i7 = I7[0];
   O7[0] = i0;
   O7[1] = i1;
   O7[2] = i2;
   O7[3] = i3;
   O7[4] = i4;
   O7[5] = i5;
   O7[6] = i6;
   O7[7] = i7;
}

   smoc_firing_state s0, s1, s2, s3, s4, s5, s6, s7; 
          
public:
 m_IDCTtranspose(sc_module_name name, int int)
 : smoc_actor(name, s0), int(int) {
s0.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O0.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out0) >> s1); 
   
s1.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O1.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out1) >> s2); 
   
s2.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O2.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out2) >> s3); 
   
s3.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O3.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out3) >> s4); 
   
s4.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O4.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out4) >> s5); 
   
s5.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O5.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out5) >> s6); 
   
s6.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O6.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out6) >> s7); 
   
s7.addTransition((I0.getAvailableTokens() >= 1 && 
   I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1 && 
   I3.getAvailableTokens() >= 1 && 
   I4.getAvailableTokens() >= 1 && 
   I5.getAvailableTokens() >= 1 && 
   I6.getAvailableTokens() >= 1 && 
   I7.getAvailableTokens() >= 1) >> 
   (O7.getAvailableSpace() >= 8) >>
   call(&m_IDCTtranspose::Out7) >> s0); 
    
 }
};

