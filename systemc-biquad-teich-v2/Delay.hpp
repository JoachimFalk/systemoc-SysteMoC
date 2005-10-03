
#include <callib.hpp>

class m_Delay: public smoc_actor {
// The actor has 2 Ports.
public: 
   smoc_port_in<int> IN;
   smoc_port_out<int> OUT;

// The actor has 2 Parameters and 1 Variable declarations.
private: 
   const int sz;
   const int init;
   int s; 
    
// The actor has 1 Actions and 0 Guards.
private:
void m_Delay::action(void) {
// The action has 0 local variable declarations.
   int a = IN[0];
   s = a; 
   OUT[0] = s;
}

   smoc_firing_state start;
      
public:
 m_Delay(sc_module_name name, int sz, int init)
 : smoc_actor(name, start), sz(sz), init(init), s(0) {
 start = 
   (IN.getAvailableTokens() >= 1) >> 
   (OUT.getAvailableSpace() >= 1)  >>
   call(&m_Delay::action) >> start;  
 }
};

