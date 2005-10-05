
#include <callib.hpp>

class m_IDCTclip: public smoc_actor {
// The actor has 3 Ports.
public: 
   smoc_port_in<int> I;
   smoc_port_in<int> MIN;
   smoc_port_out<int> O;

// The actor has 1 Parameters and 0 Variable declarations.
private: 
   const int MAX;
   
   int bound(int a, int x, int b) const {
   return( (x < a) ? a : ((x > b) ? b : (x)) );
   }
   
   
// The actor has 1 Actions and 0 Guards.
private:
void m_IDCTclip::action(void) {
// The action has 0 local variable declarations.
   int i = I[0];
   int min = MIN[0];
   O[0] = bound(min, i, 255);
}

   smoc_firing_state start;
      
public:
 m_IDCTclip(sc_module_name name, int MAX)
 : smoc_actor(name, start), MAX(MAX) {
 start = 
   (I.getAvailableTokens() >= 1 && 
   MIN.getAvailableTokens() >= 1) >> 
   (O.getAvailableSpace() >= 1)  >>
   call(&m_IDCTclip::action) >> start;  
 }
};

