
#include <iostream>

#include "snk2addr_table.hpp"

#define VERBOSE_LEVEL_SNK2ADDR_TABLE 0

long
snk2addr_table::calc_lin_addr(const iter_domain_vector_type& window_iterator,
                              bool& valid,
                              long schedule_period) const{

  long return_value;

  const data_element_id_type&
    base_data_element_id(snk_iter.get_base_data_element_id());

  //check, whether pixel is situated on extended border
  bool is_ext_border;
  snk_iter.is_ext_border(window_iterator,
                         is_ext_border);      

  if (!is_ext_border){
    // pixel must be produced by source actor
    
    iter_domain_vector_type 
      src_iteration(src_iter.iterator_depth());
    smoc_src_md_static_loop_iterator::id_type
      schedule_period_offset;

    //Get required data element
    data_element_id_type
      data_element_id(snk_iter.token_dimensions());

    snk_iter.get_window_data_element_offset(window_iterator,
                                            data_element_id);
    data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SNK2ADDR_TABLE == 100
    CoSupport::dout << "Required data element: " 
                    << data_element_id 
                    << std::endl;
#endif


    // Get required source iteration
    bool temp =
      src_iter.get_src_loop_iteration(data_element_id,
                                      src_iteration,
                                      schedule_period_offset
                                      );
    assert(temp);

    //Eliminate schedule period offset
    src_iteration[src_iteration.size()-1] +=
      schedule_period_offset*
      src_iter.iteration_size()[src_iteration.size()-1];

#if VERBOSE_LEVEL_SNK2ADDR_TABLE == 100
    CoSupport::dout << "src_iteration = " 
                    << src_iteration 
                    << std::endl;
#endif

    //calculate linearized address
    return_value = 
      src_iter.calc_iteration_id(src_iteration,
                                 schedule_period);

#if VERBOSE_LEVEL_SNK2ADDR_TABLE == 100
    CoSupport::dout << "lin_addr = " 
                    << return_value 
                    << std::endl;
#endif
    valid = true;
        
          
  }else{
    // data element is not produced by source actor
    // Insert a don't care into the table

#if VERBOSE_LEVEL_SNK2ADDR_TABLE == 100
    CoSupport::dout << "Situated on extended border" << std::endl;
#endif

    // By default, we set the address to zero.
    // However, we have to take the schedule period into account.

    iter_domain_vector_type 
      src_iteration(src_iter.iterator_depth(),
                    (smoc_src_md_static_loop_iterator::iter_item_type)0);
    return_value = 
      src_iter.calc_iteration_id(src_iteration,
                                 schedule_period);
    valid = false;
    
  }

  return return_value;

}


bool snk2addr_table::subtable_is_equal(const smoc_md_array<struct src_addr_info_struct>& table,
                                       unsigned int fixed_dimension,
                                       unsigned int coord1[],
                                       unsigned int coord2[]) const{
  bool return_value = true;
    
  //move to next dimension
  fixed_dimension++;

  if (fixed_dimension < table.dimensions()){
    for(unsigned int i = 0; 
        (i < table.size(fixed_dimension)) && return_value; 
        i++){
      coord1[fixed_dimension] = i;
      coord2[fixed_dimension] = i;
      return_value = 
        subtable_is_equal(table,fixed_dimension,coord1,coord2);
    }
    
    //write start values for next run
    coord1[fixed_dimension] = 0;
    coord2[fixed_dimension] = 0;
  }else{
    if (table[coord1].next_addr_valid && 
        table[coord2].next_addr_valid)
      return_value =  
        (table[coord1].rel_next_addr == table[coord2].rel_next_addr);
    else 
      return_value = true;
  }

    return return_value;
}

