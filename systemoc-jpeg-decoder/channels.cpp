

#include "channels.hpp"


//
std::ostream &operator << (std::ostream &out, const codeword_t val) {
  out << (unsigned int) val;
  return out;
}
