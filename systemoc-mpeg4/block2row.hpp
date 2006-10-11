#ifndef _INCLUDED_BLOCK2ROW_HPP
#define _INCLUDED_BLOCK2ROW_HPP

#include "callib.hpp"

class m_block2row: public smoc_actor {
public:
  smoc_port_in<int>  b;
  smoc_port_out<int> C0, C1, C2, C3, C4, C5, C6, C7;
private:
  void action0() {
#ifndef XILINX_EDK_RUNTIME
#ifndef NDEBUG
    for ( int i = 0; i <= 63; ++i )
#ifndef KASCPAR_PARSING 
      std::cout << "b[" << i << "] == " << b[i] << std::endl;
#endif
#endif
#endif //XILINX_EDK_RUNTIME
    C0[0]=b[0];C0[1]=b[ 8];C0[2]=b[16];C0[3]=b[24];C0[4]=b[32];C0[5]=b[40];C0[6]=b[48];C0[7]=b[56];
    C1[0]=b[1];C1[1]=b[ 9];C1[2]=b[17];C1[3]=b[25];C1[4]=b[33];C1[5]=b[41];C1[6]=b[49];C1[7]=b[57];
    C2[0]=b[2];C2[1]=b[10];C2[2]=b[18];C2[3]=b[26];C2[4]=b[34];C2[5]=b[42];C2[6]=b[50];C2[7]=b[58];
    C3[0]=b[3];C3[1]=b[11];C3[2]=b[19];C3[3]=b[27];C3[4]=b[35];C3[5]=b[43];C3[6]=b[51];C3[7]=b[59];
    C4[0]=b[4];C4[1]=b[12];C4[2]=b[20];C4[3]=b[28];C4[4]=b[36];C4[5]=b[44];C4[6]=b[52];C4[7]=b[60];
    C5[0]=b[5];C5[1]=b[13];C5[2]=b[21];C5[3]=b[29];C5[4]=b[37];C5[5]=b[45];C5[6]=b[53];C5[7]=b[61];
    C6[0]=b[6];C6[1]=b[14];C6[2]=b[22];C6[3]=b[30];C6[4]=b[38];C6[5]=b[46];C6[6]=b[54];C6[7]=b[62];
    C7[0]=b[7];C7[1]=b[15];C7[2]=b[23];C7[3]=b[31];C7[4]=b[39];C7[5]=b[47];C7[6]=b[55];C7[7]=b[63];
  }
  smoc_firing_state start;
public:
  m_block2row(sc_module_name name)
    : smoc_actor(name, start){
    start = (b.getAvailableTokens() >= 64)   >>
            (C0.getAvailableSpace() >= 8 &&
             C1.getAvailableSpace() >= 8 &&
             C2.getAvailableSpace() >= 8 &&
             C3.getAvailableSpace() >= 8 &&
             C4.getAvailableSpace() >= 8 &&
             C5.getAvailableSpace() >= 8 &&
             C6.getAvailableSpace() >= 8 &&
             C7.getAvailableSpace() >= 8)   >>
            CALL(m_block2row::action0)      >> start;
    
    }
  virtual ~m_block2row(){}
};
#endif // _INCLUDED_BLOCK2ROW_HPP
