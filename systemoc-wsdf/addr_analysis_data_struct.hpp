//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef ADDR_ANALYSIS_DATA_STRUCT_HPP
#define ADDR_ANALYSIS_DATA_STRUCT_HPP

#include <iostream>

// Structure in order to store results
struct src_addr_info_struct {
  //Absolute address
  long abs_addr;
  //Whether address is valid or not
  bool valid;
  

};


std::istream& 
operator>>(std::istream& stream, 
           struct src_addr_info_struct& element);

std::ostream& 
operator<<(std::ostream& stream, 
           const struct src_addr_info_struct& element);


#endif //ADDR_ANALYSIS_DATA_STRUCT_HPP
