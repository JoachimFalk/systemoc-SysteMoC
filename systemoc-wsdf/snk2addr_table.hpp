//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef SNK2ADDR_TABLE_HPP
#define SNK2ADDR_TABLE_HPP

#include "addr_analysis_data_struct.hpp"
#include <systemoc/smoc_md_loop.hpp>
#include <systemoc/smoc_md_array.hpp>
#include "cosupport/smoc_debug_out.hpp"


class snk2addr_table {
public:
  
  snk2addr_table(const smoc_src_md_static_loop_iterator& src_iter,
                 const smoc_snk_md_static_loop_iterator& snk_iter)
    : src_iter(src_iter),
      snk_iter(snk_iter)
  {}
  virtual ~snk2addr_table(){}

protected:
  /// Typedefs
  typedef smoc_snk_md_static_loop_iterator::iter_domain_vector_type 
  iter_domain_vector_type;

  typedef smoc_snk_md_static_loop_iterator::data_element_id_type
  data_element_id_type;

protected:

  // This function returns the absolute address for the
  // the given window iteration.
  // By help of the valid flag it signals, whether this
  // is a valid address not situated on the extended border.
  //
  // Currently, the absolute address is derived from the
  // iteration ID of the corresponding source iteration.
  // If for example the effective tokens shall be rounded
  // up to powers of two, this has to be changed.
  long
  calc_lin_addr(const iter_domain_vector_type& window_iterator,
                bool& valid,
                long schedule_period = 0) const;

  //this function checks, whether all table items with the coordinates
  //(coord1[0],..,coord1[fixed_dimension],x[i])
  //and (coord2[0],..,coord2[fixed_dimension],x[i])
  //are identical (i > fixed_dimension; x stands for all possible coordinates)
  bool subtable_is_equal(const smoc_md_array<struct src_addr_info_struct>& table,
                         unsigned int fixed_dimension,
                         unsigned int coord1[],
                         unsigned int coord2[]) const;

protected:
  smoc_src_md_static_loop_iterator src_iter;
  smoc_snk_md_static_loop_iterator snk_iter;

};







#endif
