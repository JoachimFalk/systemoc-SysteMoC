
#include <callib.hpp>

class m_byte2bit: public smoc_actor {
public:
  smoc_port_in<int>  in8;
  smoc_port_out<int> out;
private:
  void action0() {
    for(int j = 0; j <= 7; j++) {
      int bit = (in8[0] >> (7-j)) & 1; 
      std::cout<< "byte2bit wert" << bit << std::endl;
      out[j] = bit;
    }
  }
  
  smoc_firing_state start;
public:
  m_byte2bit(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in8(1)) >>
			(out(8))  >>
			CALL(m_byte2bit::action0)       >> start;
  }
};
