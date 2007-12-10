#include <cstdlib>
#include <iostream>

class BitGen: public smoc_actor {
public:
  smoc_port_out<bool> out;
private:
  unsigned int pos;
  std::string bitString;

  bool bitsLeft() const{
    return pos < bitString.length();
  }

  void produceBit() {
    bool b = (bitString[pos] == '1');
    std::cout << this->name() << ": " << bitString[pos] << " - "
              << b << std::endl;
    out[0] = b;
    ++pos;
  }

  
  smoc_firing_state main;
public:
  BitGen(sc_module_name name, std::string bits = "111111111111000000111111000000000000000000000000" )
    : smoc_actor(name, main),
      pos( 0 ),
      bitString( bits ) {

    main = GUARD(BitGen::bitsLeft)  >>
      ( out(1) )                    >>
      CALL(BitGen::produceBit)      >>
      main;

  }
};
