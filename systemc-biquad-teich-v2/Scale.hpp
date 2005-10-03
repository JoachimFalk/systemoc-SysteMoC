
#include <callib.hpp>

class m_Scale: public smoc_actor {
// The actor has 2 Ports.
public: 
   smoc_port_in<IN;
   smoc_port_out<OUT;

// The actor has 2 Parameters and 0 Variable declarations.
private: 
   const int sz;
   const int c;
   
// The actor has 1 Actions and 0 Guards.
private:
void m_Scale::action(void) {
// The action has 0 local variable declarations.
   int a = IN[0];
   OUT[0] = (c * a);
}

   smoc_firing_state start;
      
public:
 m_Scale(sc_module_name name, int sz, int c)
 : smoc_actor(name, start), sz(sz), c(c) {
 start = 
   (IN.getAvailableTokens() >= 1) >> 
   (OUT.getAvailableSpace() >= 1)  >>
   call(&m_Scale::action) >> start;  
 }
};

