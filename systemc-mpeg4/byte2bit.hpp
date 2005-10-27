
#include <callib.hpp>

class m_byte2bit: public smoc_actor {
public:
  smoc_port_in<int>  in8;
  smoc_port_out<int> out;
private:
  void action0() {
    out[0] = (in8[0] >> 7) & 1; 
    out[1] = (in8[0] >> 6) & 1;
    out[2] = (in8[0] >> 5) & 1;
    out[3] = (in8[0] >> 4) & 1;
    out[4] = (in8[0] >> 3) & 1;
    out[5] = (in8[0] >> 2) & 1;
    out[6] = (in8[0] >> 1) & 1;
    out[7] = (in8[0] >> 0) & 1;

  for(int j=0; j<8; j++)
    std::cout<< "byte2bit wert" << out[j]<< endl;
  
  } 
  
  smoc_firing_state start;
public:
  m_byte2bit(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in8.getAvailableTokens() >= 1) >>
            (out.getAvailableSpace() >= 8)  >>
            CALL(m_byte2bit::action0)       >> start;
  }
};
