#include <cstdlib>
#include <iostream>

class BitToggle: public smoc_actor {
public:
  smoc_port_out<bool> out;
private:
  unsigned int limitCount;
  unsigned int pauseCount;
  unsigned int pause;

  void pos() {
    cout << name() << ".pos()" << endl;
    limitCount++;
    out[0] = 1;
  }

  void neg(){
    cout << name() << ".neg()" << endl;
    out[0] = 0;

    if(pauseCount >= pause) pauseCount = 0;
    pauseCount++;
  }
  
  smoc_firing_state s_pos, s_neg;
public:
  BitToggle(sc_module_name name, unsigned int limit, unsigned int pause = 0 )
    : smoc_actor(name, s_pos),
      limitCount( 0 ),
      pauseCount( 0 ),
      pause( pause ) {
    SMOC_REGISTER_CPARAM(limit);

    s_pos = (VAR(limitCount)<limit) >>
      ( out(1) )                    >>
      CALL(BitToggle::pos)          >>
      s_neg;

    s_neg = 
      !(VAR(pauseCount)<pause) >>
      ( out(1) )               >>
      CALL(BitToggle::neg)     >>
      s_pos
    |
      (VAR(pauseCount)<pause) >>
      ( out(1) )              >>
      CALL(BitToggle::neg)    >>
      s_neg;

  }
};
