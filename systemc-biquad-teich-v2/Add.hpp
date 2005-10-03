
#include <callib.hpp>

class m_Add: public smoc_actor {
// The actor has 3 Ports.
public: 
   smoc_port_in<int> A;
   smoc_port_in<int> B;
   smoc_port_out<int> OUT;

// The actor has 1 Parameters and 0 Variable declarations.
private: 
   const int sz;
   
// The actor has 1 Actions and 0 Guards.
private:
void m_Add::action(void) {
// The action has 0 local variable declarations.
   int a = A[0];
   int b = B[0];
   OUT[0] = (a + b);
}

   smoc_firing_state start;
      
public:
 m_Add(sc_module_name name, int sz)
 : smoc_actor(name, start), sz(sz) {
 start = 
   (A.getAvailableTokens() >= 1 && 
   B.getAvailableTokens() >= 1) >> 
   (OUT.getAvailableSpace() >= 1)  >>
   call(&m_Add::action) >> start;  
 }
};

