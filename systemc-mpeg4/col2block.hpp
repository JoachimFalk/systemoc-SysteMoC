class m_col2block: public smoc_actor {
public:
  smoc_port_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  smoc_port_out<cal_list<int>::t> B;
private:
  void action0() {
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R0[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R1[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R2[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R3[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R4[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R5[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R6[i]);
    for ( int i = 0; i < 8; i++ )
      B[0].push_back(R7[i]);
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
            (B.getAvailableSpace() >= 1)      >>
            call(&m_col2block::action0)       >> start;
  }
};
