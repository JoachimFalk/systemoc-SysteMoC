//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef ADDR_ANALYSIS_DATA_STRUCT_HPP
#define ADDR_ANALYSIS_DATA_STRUCT_HPP

#include <iostream>

// Structure in order to store results
struct src_addr_info_struct {

  //Absolute iteration of this iteration
  long curr_abs_addr;

  bool curr_addr_valid;
  
  //Difference to address of next
  //iteration
  long rel_next_addr;
  
  //Whether address of next iteration
  //is valid or not
  bool next_addr_valid;
  
};

#if 0
std::istream& 
operator>>(std::istream& stream, 
           struct src_addr_info_struct& element);
#endif

std::ostream& 
operator<<(std::ostream& stream, 
           const struct src_addr_info_struct& element);


#endif //ADDR_ANALYSIS_DATA_STRUCT_HPP
