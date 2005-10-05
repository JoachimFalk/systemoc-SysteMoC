
#include <callib.hpp>

class m_Upsample: public smoc_actor {
// The actor has 2 Ports.
public: 
   smoc_port_in<int> I;
   smoc_port_out<int> O;

// The actor has 1 Parameters and 2 Variable declarations.
private: 
   const int factor;
   int state; 
    int mem; 
    
// The actor has 2 Actions and 2 Guards.
private:
bool m_Upsample::guard_read(void)  const {
   const int i = I[0];
   return( (state == 0) );
}
bool m_Upsample::guard_copy(void)  const {
   return( (state > 0) );
}
void m_Upsample::read(void) {
// The action has 0 local variable declarations.
   int i = I[0];
   mem = i; 
   state = (state + 1); 
   O[0] = i;
}
void m_Upsample::copy(void) {
// The action has 0 local variable declarations.
   state = (state + 1); 
   if (state == 8) { 
      state = 0; 
   }
   O[0] = mem;
}

   smoc_firing_state start;
      
public:
 m_Upsample(sc_module_name name, int factor)
 : smoc_actor(name, start), factor(factor), state(0), mem(mem) {
 start = 
   (I.getAvailableTokens() >= 1 && 
   guard(&m_Upsample::guard_read)) >> 
   (O.getAvailableSpace() >= 1)  >>
   call(&m_Upsample::read) >> start |  
   (guard(&m_Upsample::guard_copy)) >> 
   (O.getAvailableSpace() >= 1)  >>
   call(&m_Upsample::copy) >> start;  
 }
};

