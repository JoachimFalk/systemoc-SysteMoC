#ifndef _INCLUDED_IDCTCLIP_HPP
#define _INCLUDED_IDCTCLIP_HPP

class m_IDCTclip: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_in<int>  MIN;
  smoc_port_out<int> O;
private:
  const int MAX;
  
  int bound(int a, int x, int b) {
    return x < a 
      ? a
      : ( x > b
          ? b
          : x );
  }
  
  void action0() { 
    //std::cout<<"M_clip debugzeile hier ist I wert: "<< I[0] <<"\n";
    O[0] = bound(MIN[0], I[0], MAX); }
  
    smoc_firing_state start;
public:
  m_IDCTclip(sc_module_name name, int MAX)
    : smoc_actor(name, start),
      MAX(MAX) {
    start = (I.getAvailableTokens() >= 1 &&
             MIN.getAvailableTokens() >= 1 )  >>
            (O.getAvailableSpace() >= 1)      >>
            CALL(m_IDCTclip::action0)  	      >> start;

      }
  virtual ~m_IDCTclip(){}
};

#endif // _INCLUDED_IDCTCLIP_HPP
