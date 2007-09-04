//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef SNK2ADDR_TABLE_REF_POINT_HPP
#define SNK2ADDR_TABLE_REF_POINT_HPP

#include "snk2addr_table.hpp"

class snk2addr_table_ref_point
  : public snk2addr_table
{

public:
  snk2addr_table_ref_point(const smoc_src_md_static_loop_iterator& src_iter,
                           const smoc_snk_md_static_loop_iterator& snk_iter);

public:
  const smoc_md_array<struct src_addr_info_struct>&
  get_ref_point_addr_offet_table() const{
    return ref_point_addr_offset_table;
  }

protected:
  const iter_domain_vector_type ref_window_iterator;

protected:
  smoc_md_array<struct src_addr_info_struct> 
  ref_point_addr_offset_table;

  
private:
  //Attention: modifies sink iterator!
  void build_ref_point_addr_offset_table();

  //This function tries to optimize invalid
  //addresses in order to get a more compact representation
  void optimize_invalid_addresses();

  //This function checks, whether the table contains
  //invalid items
  bool has_invalid_items(unsigned int fixed_dimension,
                         unsigned int coord[]
                         ) const;

  ///This function tries to replace invalid table items
  ///by the same values as occuring for the neighbor
  ///items, such that the resulting if-then-else
  ///structure is smaller.
  ///
  ///cur_dimension: dimension in which we try to propagate the data
  ///coord1: Currently processed coordinate
  ///coord2: Current reference coordinate where we propagate from.
  void
  make_subtable_equal_forward(unsigned int coord1[],
                              unsigned int coord2[],
                              unsigned int cur_dimension = 0);

  /// Same for backward direction
  void
  make_subtable_equal_backward(unsigned int coord1[],
                               unsigned int coord2[],
                               unsigned int cur_dimension = 0);

  /// This function performs the propagation
  int propagate_addr(unsigned int fixed_dimension,
                     unsigned int coord1[],
                     unsigned int coord2[]);


};

#endif
