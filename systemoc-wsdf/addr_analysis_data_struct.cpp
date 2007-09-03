//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include "addr_analysis_data_struct.hpp"

#if 0
// Corresponding stream operators required for input and output.
std::istream& 
operator>>(std::istream& stream, 
           struct src_addr_info_struct& element) {

  unsigned int temp;

  stream >> temp;
  if (temp)
    element.valid = true;
  else
    element.valid = false;

  stream >> element.abs_addr;
  stream >> element.rel_addr;

  return stream;
}
#endif

std::ostream& 
operator<<(std::ostream& stream, 
           const struct src_addr_info_struct& element){
  if (element.curr_addr_valid){
    stream << "1 ";
  }else{
    stream << "0 ";
  }
  stream << element.curr_abs_addr << " ";
  
  if (element.next_addr_valid){
    stream << "1 ";
  }else{
    stream << "0 ";
  }
  stream << element.rel_next_addr << " ";

  return stream;
}
