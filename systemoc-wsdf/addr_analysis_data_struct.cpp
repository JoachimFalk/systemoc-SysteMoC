//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include "addr_analysis_data_struct.hpp"


// Corresponding stream operators required for input and output.
std::istream& 
operator>>(std::istream& stream, 
           struct src_addr_info_struct& element) {

  unsigned int temp;

  stream >> temp;
  if (temp)
    element.curr_addr_valid = true;
  else
    element.curr_addr_valid = false;

  stream >> element.curr_abs_addr;


  stream >> temp;
  if (temp)
    element.next_addr_valid = true;
  else
    element.next_addr_valid = false;

  stream >> element.rel_next_addr;

  // We do not reconstruct the pointer structure!
  element.next_iter_item = NULL;


  return stream;
}


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


bool operator==(const struct src_addr_info_struct& e1,
                const struct src_addr_info_struct& e2) {
  return e1 == e2;
}
