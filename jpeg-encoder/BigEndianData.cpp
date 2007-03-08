/* vim: set sw=2 ts=8 sts=2 expandtab: */

#include "BigEndianData.hpp"

std::string uint2binstr(size_t code, size_t len) {
  std::ostringstream sstr;
  size_t             mask = 1 << (len-1);
  
  for (; len > 0; --len, code <<= 1 )
    sstr << (code & mask ? "1" : "0");
  return sstr.str();
}
