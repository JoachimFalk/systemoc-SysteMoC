
#ifndef _INCLUDED_MIN_DUPLEX_HPP
#define _INCLUDED_MIN_DUPLES_HPP

class m_MIN_duplex: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O0;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
  smoc_port_out<int> O3;
  smoc_port_out<int> O4;
  smoc_port_out<int> O5;
  smoc_port_out<int> O6;
  smoc_port_out<int> O7;

private:
  
  void action() {
    O0[0] = I[0];
    O1[0] = I[0];
    O2[0] = I[0];
    O3[0] = I[0];
    O4[0] = I[0];
    O5[0] = I[0];
    O6[0] = I[0];
    O7[0] = I[0];
  }
  
  smoc_firing_state start;
  
public:
  m_MIN_duplex(sc_module_name name)
    : smoc_actor(name, start) {
    start = I(1) >>
            (O0(1) && O1(1) && O2(1) && O3(1) &&
             O4(1) && O5(1) && O6(1) && O7(1))    >>
            CALL(m_MIN_duplex::action)            >> start;
  }
};

#endif // _INCLUDED_MIN_DUPLEX_HPP
