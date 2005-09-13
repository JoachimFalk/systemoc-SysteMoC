#ifndef _INCLUDED_COL2BLOCK_HPP
#define _INCLUDED_COL2BLOCK_HPP

class m_col2block: public smoc_actor {
public:
  smoc_port_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  smoc_port_out<int> b;
private:
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
            call(&m_col2block::action0)       >> start;
  }
  
  virtual ~m_col2block(){}
};

#endif // _INCLUDED_COL2BLOCK_HPP
