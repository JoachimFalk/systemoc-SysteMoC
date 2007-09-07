
#include "snk2addr_table_win_iter.hpp"

#define SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL 100


snk2addr_table_win_iter::snk2addr_table_win_iter(const smoc_src_md_static_loop_iterator& src_iter,
                                                 const smoc_snk_md_static_loop_iterator& snk_iter,
                                                 const smoc_md_array<struct src_addr_info_struct>& ref_point_addr_offset_table)
  : snk2addr_table(src_iter, snk_iter),
    win_iter_addr_offset_table(snk_iter.iterator_depth(),
                               snk_iter.iteration_size()),
    ref_point_addr_offset_table(ref_point_addr_offset_table)
  
{
  build_win_iter_addr_offset_table();
  optimize_invalid_addresses();

}



void 
snk2addr_table_win_iter::build_win_iter_addr_offset_table(){

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Enter snk2addr_table_win_iter::build_win_iter_addr_offset_table" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  do {
    smoc_snk_md_static_loop_iterator::iter_domain_vector_type
      window_iterator(snk_iter.token_dimensions(),
                      (smoc_snk_md_static_loop_iterator::iter_item_type)0);
    
    // Iteration over all window elements
    bool end_of_window = false;
    while(!end_of_window){

      // Set window iterator, such that we can use the
      // iteration vector as array index
      snk_iter.set_window_iterator(window_iterator);

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
      CoSupport::dout << "Iteration: " 
                      << snk_iter.iteration_vector() 
                      << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
#endif


      bool addr_valid;
      long lin_addr = 
        calc_lin_addr(window_iterator,
                      addr_valid);

      //Note, that here the structure names are not very nice.
      //Perhaps it would be better to introduce a new structure.
      //Store absolute address
      win_iter_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr =
        lin_addr;
      //Calculate relative address to reference pixel
      win_iter_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr =
        lin_addr - 
        ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr;
      
      win_iter_addr_offset_table[snk_iter.iteration_vector()].curr_addr_valid =
        win_iter_addr_offset_table[snk_iter.iteration_vector()].next_addr_valid =
        addr_valid;

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
      CoSupport::dout << "Absolute address: " << lin_addr << std::endl;
      CoSupport::dout << "Relative address: " 
                      << win_iter_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr 
                      << std::endl;
      CoSupport::dout << (addr_valid ? "address valid" : "address not valid")
                      << std::endl;

      CoSupport::dout << "Move to next window ..." << std::endl;
#endif

      end_of_window = true;
      //Move to next window position
      for(int i = snk_iter.token_dimensions()-1;
          i >= 0;
          i--){
        window_iterator[i]++;
        if (window_iterator[i] > snk_iter.max_window_iteration()[i]){
          window_iterator[i] = 0;
        }else{
          end_of_window = false;
          break;
        }
      }
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
      CoSupport::dout << CoSupport::Indent::Down;
#endif

    } //iteration of window

    
  }while(!snk_iter.inc());

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Leave snk2addr_table_win_iter::build_win_iter_addr_offset_table" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  
}

void snk2addr_table_win_iter::optimize_invalid_addresses() {
  const unsigned int table_dimensions =
    snk_iter.iterator_depth();

  unsigned int coord1[table_dimensions];
  unsigned int coord2[table_dimensions];

  //Propagate forward from coord2 to coord1
  optimize_invalid_addresses_forward(0,coord2,coord1);
  

  //Propagate forward from coord1 to coord2
  optimize_invalid_addresses_backward(0,coord1,coord2);
 
}

void snk2addr_table_win_iter::optimize_invalid_addresses_forward(unsigned int fixed_dimensions,
                                                                 unsigned int coord1[],
                                                                 unsigned int coord2[]){

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Enter snk2addr_table_win_iter::optimize_invalid_addresses_forward "
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;

  CoSupport::dout << "fixed_dimensions = " << fixed_dimensions
                  << std::endl;
#endif
  
  const unsigned int table_dimensions =

    snk_iter.iterator_depth();

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  for(unsigned int i = 0; i < table_dimensions; i++){
    CoSupport::dout << "coord1[" << i << "]=" << coord1[i] << ",";
  }
  CoSupport::dout << std::endl;

  for(unsigned int i = 0; i < table_dimensions; i++){
    CoSupport::dout << "coord2[" << i << "]=" << coord2[i] << ",";
  }
  CoSupport::dout << std::endl;
#endif

  if(fixed_dimensions < table_dimensions){

    //Special case for first "row"
    coord1[fixed_dimensions] = coord2[fixed_dimensions] = 0;
    optimize_invalid_addresses_forward(fixed_dimensions+1,
                                       coord1,
                                       coord2);

    for(coord2[fixed_dimensions] = 1;
        coord2[fixed_dimensions] < 
          win_iter_addr_offset_table.size(fixed_dimensions);
        coord2[fixed_dimensions]++){

      coord1[fixed_dimensions] = coord2[fixed_dimensions]-1;
      
      if (subtable_is_equal(win_iter_addr_offset_table,
                            fixed_dimensions,
                            coord1,
                            coord2)){
        if(!propagate_forward(fixed_dimensions,
                              coord1,
                              coord2)){
          //Could not replace all invalid items in coord2
          coord1[fixed_dimensions] = coord2[fixed_dimensions];
          optimize_invalid_addresses_forward(fixed_dimensions+1,
                                             coord1,
                                             coord2);
        }
      }else{
        //Subtables are not equal.
        coord1[fixed_dimensions] = coord2[fixed_dimensions];
        optimize_invalid_addresses_forward(fixed_dimensions+1,
                                           coord1,
                                           coord2);
      }
    }
  }

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Leave snk2addr_table_win_iter::optimize_invalid_addresses_forward "
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
}

void snk2addr_table_win_iter::optimize_invalid_addresses_backward(unsigned int fixed_dimensions,
                                                                  unsigned int coord1[],
                                                                  unsigned int coord2[]){

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Enter snk2addr_table_win_iter::optimize_invalid_addresses_backward "
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;

  CoSupport::dout << "fixed_dimensions = " << fixed_dimensions
                  << std::endl;
#endif

  const unsigned int table_dimensions =
    snk_iter.iterator_depth();

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  for(unsigned int i = 0; i < table_dimensions; i++){
    CoSupport::dout << "coord1[" << i << "]=" << coord1[i] << ",";
  }
  CoSupport::dout << std::endl;

  for(unsigned int i = 0; i < table_dimensions; i++){
    CoSupport::dout << "coord2[" << i << "]=" << coord2[i] << ",";
  }
  CoSupport::dout << std::endl;
#endif

  if(fixed_dimensions < table_dimensions){

#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
    CoSupport::dout << "Enter optimization loop ..." << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif

    for(coord1[fixed_dimensions] = 
          win_iter_addr_offset_table.size(fixed_dimensions)-1;
        coord1[fixed_dimensions] > 0;        
        coord1[fixed_dimensions]--){
      coord2[fixed_dimensions] = coord1[fixed_dimensions]-1;
      
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
      CoSupport::dout << "coord1[fixed_dimensions] = "
                      << coord1[fixed_dimensions]
                      << std::endl;
#endif
      
      if (subtable_is_equal(win_iter_addr_offset_table,
                            fixed_dimensions,
                            coord1,
                            coord2)){
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
        CoSupport::dout << "Subtable is equal."
                        << std::endl;
#endif
        if(!propagate_backward(fixed_dimensions,
                               coord1,
                               coord2)){
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
          CoSupport::dout << "Could not replace all invalid items."
                          << std::endl;
#endif
          //Could not replace all invalid items in coord2
          coord2[fixed_dimensions] = coord1[fixed_dimensions];
          optimize_invalid_addresses_backward(fixed_dimensions+1,
                                              coord1,
                                              coord2);
        }
      }else{
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
        CoSupport::dout << "Subtable is not equal."
                        << std::endl;
#endif
        coord2[fixed_dimensions] = coord1[fixed_dimensions];
        optimize_invalid_addresses_backward(fixed_dimensions+1,
                                            coord1,
                                            coord2);
      }
    }

    //Special case for first "row"
    coord2[fixed_dimensions] = coord1[fixed_dimensions] = 0;
    optimize_invalid_addresses_backward(fixed_dimensions+1,
                                        coord1,
                                        coord2);


#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
    CoSupport::dout << "Leave optimization." << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif

  }
  
#if SNK2ADDR_TABLE_WIN_ITER_VERBOSE_LEVEL == 100
  CoSupport::dout << "Leave snk2addr_table_win_iter::optimize_invalid_addresses_backward"
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

}

bool snk2addr_table_win_iter::propagate_forward(unsigned int fixed_dimension,
                                                unsigned int coord1[],
                                                unsigned int coord2[]){
  bool return_value = true;

  const unsigned int table_dimensions =
    snk_iter.iterator_depth();

  if (fixed_dimension < table_dimensions-1){
    //move to next dimension
    fixed_dimension++;
    for(unsigned int i = 0;
        i < win_iter_addr_offset_table.size(fixed_dimension);
        i++){
      coord1[fixed_dimension] = i;
      coord2[fixed_dimension] = i;
      return_value = return_value &
        propagate_forward(fixed_dimension,
                          coord1,
                          coord2);
    }
  }else{
    if(!win_iter_addr_offset_table[coord2].next_addr_valid){
      if (win_iter_addr_offset_table[coord1].next_addr_valid){
        win_iter_addr_offset_table[coord2].curr_abs_addr +=
          win_iter_addr_offset_table[coord1].rel_next_addr-
          win_iter_addr_offset_table[coord2].rel_next_addr;
        
        win_iter_addr_offset_table[coord2].next_addr_valid = true;
        win_iter_addr_offset_table[coord2].rel_next_addr =
          win_iter_addr_offset_table[coord1].rel_next_addr;
        return_value = true;
      }else{
        return_value = false;
      }
    }else{
      return_value = true;
    }
  }

  return return_value;
}


bool snk2addr_table_win_iter::propagate_backward(unsigned int fixed_dimension,
                                                 unsigned int coord1[],
                                                 unsigned int coord2[]){
  bool return_value = true;

  const unsigned int table_dimensions =
    snk_iter.iterator_depth();

  if (fixed_dimension < table_dimensions-1){
    //move to next dimension
    fixed_dimension++;
    for(int i = win_iter_addr_offset_table.size(fixed_dimension) - 1;
        i >= 0;
        i--){
      coord1[fixed_dimension] = i;
      coord2[fixed_dimension] = i;
      return_value = return_value &
        propagate_forward(fixed_dimension,
                          coord1,
                          coord2);
    }
  }else{
    if(!win_iter_addr_offset_table[coord2].next_addr_valid){
      if (win_iter_addr_offset_table[coord1].next_addr_valid){
        win_iter_addr_offset_table[coord2].curr_abs_addr +=
          win_iter_addr_offset_table[coord1].rel_next_addr-
          win_iter_addr_offset_table[coord2].rel_next_addr;

        win_iter_addr_offset_table[coord2].next_addr_valid = true;
        win_iter_addr_offset_table[coord2].rel_next_addr =
          win_iter_addr_offset_table[coord1].rel_next_addr;
        return_value = true;
      }else{
        return_value = false;
      }
    }else{
      return_value = true;
    }
  }

  return return_value;
}
