class m_transpose: public smoc_actor {
public:
  smoc_port_in<int>  I0, I1, I2, I3, I4, I5, I6, I7;
  smoc_port_out<int> O0, O1, O2, O3, O4, O5, O6, O7;
private:
  void action0() { O0[0] = I0[0]; O0[1] = I1[0]; O0[2] = I2[0]; O0[3] = I3[0];
                   O0[4] = I4[0]; O0[5] = I5[0]; O0[6] = I6[0]; O0[7] = I7[0]; }
  void action1() { O1[0] = I0[0]; O1[1] = I1[0]; O1[2] = I2[0]; O1[3] = I3[0];
                   O1[4] = I4[0]; O1[5] = I5[0]; O1[6] = I6[0]; O1[7] = I7[0]; }
  void action2() { O2[0] = I0[0]; O2[1] = I1[0]; O2[2] = I2[0]; O2[3] = I3[0];
                   O2[4] = I4[0]; O2[5] = I5[0]; O2[6] = I6[0]; O2[7] = I7[0]; }
  void action3() { O3[0] = I0[0]; O3[1] = I1[0]; O3[2] = I2[0]; O3[3] = I3[0];
                   O3[4] = I4[0]; O3[5] = I5[0]; O3[6] = I6[0]; O3[7] = I7[0]; }
  void action4() { O4[0] = I0[0]; O4[1] = I1[0]; O4[2] = I2[0]; O4[3] = I3[0];
                   O4[4] = I4[0]; O4[5] = I5[0]; O4[6] = I6[0]; O4[7] = I7[0]; }
  void action5() { O5[0] = I0[0]; O5[1] = I1[0]; O5[2] = I2[0]; O5[3] = I3[0];
                   O5[4] = I4[0]; O5[5] = I5[0]; O5[6] = I6[0]; O5[7] = I7[0]; }
  void action6() { O6[0] = I0[0]; O6[1] = I1[0]; O6[2] = I2[0]; O6[3] = I3[0];
                   O6[4] = I4[0]; O6[5] = I5[0]; O6[6] = I6[0]; O6[7] = I7[0]; }
  void action7() { O7[0] = I0[0]; O7[1] = I1[0]; O7[2] = I2[0]; O7[3] = I3[0];
                   O7[4] = I4[0]; O7[5] = I5[0]; O7[6] = I6[0]; O7[7] = I7[0]; }
  smoc_firing_state s0, s1, s2, s3, s4, s5, s6, s7;
public:
  m_transpose(sc_module_name name)
    : smoc_actor(name, s0) {
    s0 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O0.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action0)    >> s1;
    s1 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O1.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action1)    >> s2;
    s2 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O2.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action2)    >> s3;
    s3 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O3.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action3)    >> s4;
    s4 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O4.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action4)    >> s5;
    s5 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O5.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action5)    >> s6;
    s6 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O6.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action6)    >> s7;
    s7 = (I0.getAvailableTokens() >= 1 &&
		I1.getAvailableTokens() >= 1 &&
		I2.getAvailableTokens() >= 1 &&
		I3.getAvailableTokens() >= 1 &&
		I4.getAvailableTokens() >= 1 &&
		I5.getAvailableTokens() >= 1 &&
		I6.getAvailableTokens() >= 1 &&
		I7.getAvailableTokens() >= 1 ) >>
		(O7.getAvailableSpace() >= 8)  >>
		call(&m_transpose::action7)    >> s0;
    }
};

