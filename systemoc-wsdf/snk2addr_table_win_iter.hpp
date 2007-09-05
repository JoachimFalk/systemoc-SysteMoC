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

};

#endif
