
#include <callib.hpp>

class m_IDCTaddsub: public smoc_actor {
// The actor has 4 Ports.
public: 
   smoc_port_in<int> I1;
   smoc_port_in<int> I2;
   smoc_port_out<int> O1;
   smoc_port_out<int> O2;

// The actor has 3 Parameters and 0 Variable declarations.
private: 
   const int G;
   const int OS;
   const int ATTEN;
   
// The actor has 1 Actions and 0 Guards.
private:
void m_IDCTaddsub::action(void) {
// The action has 2 local variable declarations.
   int i1 = I1[0];
   int i2 = I2[0];
   int o1 = (1 * (i1 + i2) + 0);
   int o2 = (1 * (i1 - i2) + 0);
   if (o1 < 0) { 
      o1 = (o1 - (16384 - 1)); 
   }
   o1 = (o1 / 16384); 
   if (o2 < 0) { 
      o2 = (o2 - (16384 - 1)); 
   }
   o2 = (o2 / 16384); 
   O1[0] = o1;
   O2[0] = o2;
}

   smoc_firing_state start;
      
public:
 m_IDCTaddsub(sc_module_name name, int G, int OS, int ATTEN)
 : smoc_actor(name, start), G(G), OS(OS), ATTEN(ATTEN) {
 start = 
   (I1.getAvailableTokens() >= 1 && 
   I2.getAvailableTokens() >= 1) >> 
   (O1.getAvailableSpace() >= 1 &&  
   O2.getAvailableSpace() >= 1)  >>
   call(&m_IDCTaddsub::action) >> start;  
 }
};

