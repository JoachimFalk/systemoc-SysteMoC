//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef SNK2ADDR_TABLE_WIN_ITER_HPP
#define SNK2ADDR_TABLE_WIN_ITER_HPP

#include "snk2addr_table.hpp"

class snk2addr_table_win_iter
  : public snk2addr_table
{

public:
  snk2addr_table_win_iter(const smoc_src_md_static_loop_iterator& src_iter,
                          const smoc_snk_md_static_loop_iterator& snk_iter,
                          const smoc_md_array<struct src_addr_info_struct>& ref_point_addr_offset_table);

public:
  const smoc_md_array<struct src_addr_info_struct>&
  get_win_iter_addr_offet_table() const{
    return win_iter_addr_offset_table;
  }

protected:
  smoc_md_array<struct src_addr_info_struct> 
  win_iter_addr_offset_table;

  const smoc_md_array<struct src_addr_info_struct>& 
  ref_point_addr_offset_table;
  
private:
  //Attention: modifies sink iterator!
  void build_win_iter_addr_offset_table();

  //This function tries to optimize invalid
  //addresses in order to get a more compact representation
  void optimize_invalid_addresses();

  //Tries to propagates address values from coord1 to coord2 in forward direction
  void optimize_invalid_addresses_forward(unsigned int fixed_dimensions,
                                          unsigned int coord1[],
                                          unsigned int coord2[]);
  //Tries to propagates address values from coord1 to coord2 in backward direction
  void optimize_invalid_addresses_backward(unsigned int fixed_dimensions,
                                           unsigned int coord1[],
                                           unsigned int coord2[]);

  // Performs the data propagation
  // from coord1 to coord2
  // returns false, if there stay invalid entries in coord2
  bool propagate_forward(unsigned int fixed_dimension,
                         unsigned int coord1[],
                         unsigned int coord2[]);
  bool propagate_backward(unsigned int fixed_dimension,
                          unsigned int coord1[],
                          unsigned int coord2[]);

};

#endif
