
#include <callib.hpp>

class m_IDCTfly: public smoc_actor {
// The actor has 4 Ports.
public: 
   smoc_port_in<int> I1;
   smoc_port_in<int> I2;
   smoc_port_out<int> O1;
   smoc_port_out<int> O2;

// The actor has 5 Parameters and 0 Variable declarations.
private: 
   const int W0;
   const int OS;
   const int W1;
   const int W2;
   const int ATTEN;
   
// The actor has 1 Actions and 0 Guards.
private:
void m_IDCTfly::action(void) {
// The action has 3 local variable declarations.
   int i1 = I1[0];
   int i2 = I2[0];
   int t = ((565 * (i1 + i2)) + 4);
   int o1 = (t + (i1 * (2841 - 565)));
   int o2 = (t + (i2 * ((-2841) - 565)));
   if (o1 < 0) { 
      o1 = (o1 - (8 - 1)); 
   }
   o1 = (o1 / 8); 
   if (o2 < 0) { 
      o2 = (o2 - (8 - 1)); 
   }
   o2 = (o2 / 8); 
   O1[0] = o1;
   O2[0] = o2;
}

   smoc_firing_state start;
      
public:
 m_IDCTfly(sc_module_name name, int W0, int OS, int W1, int W2, int ATTEN)
 : smoc_actor(name, start), W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
 start = 
   (I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1) >> 
   (O1.getAvailableSpace() >= 1 &&  
   O2.getAvailableSpace() >= 1)  >>
   call(&m_IDCTfly::action) >> start;  
 }
};

