
#include "snk2addr_table_ref_point.hpp"


snk2addr_table_ref_point::snk2addr_table_ref_point(const smoc_src_md_static_loop_iterator& src_iter,
                                                   const smoc_snk_md_static_loop_iterator& snk_iter)
  : snk2addr_table(src_iter, snk_iter),
    //ref_window_iterator(snk_iter.max_window_iteration()/2),
    ref_window_iterator(snk_iter.token_dimensions(),
                        (smoc_snk_md_static_loop_iterator::iter_item_type)0),
    ref_point_addr_offset_table(snk_iter.iterator_depth()-
                                snk_iter.token_dimensions(),
                                snk_iter.iteration_size())
{
  build_ref_point_addr_offset_table();

  unsigned int coord1[ref_point_addr_offset_table.dimensions()];
  unsigned int coord2[ref_point_addr_offset_table.dimensions()];

  make_subtable_equal_forward(coord1,
                              coord2);
  make_subtable_equal_backward(coord1,
                               coord2);
}



void 
snk2addr_table_ref_point::build_ref_point_addr_offset_table(){

  //Note: if the first reference iterator does not access
  //      a valid address, we assume, that it uses
  //      the data element produced by the source
  //      iteration zero.
  long schedule_period = 0;
  ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr =
    calc_lin_addr(ref_window_iterator,
                  ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_addr_valid);

  bool finished = false;
  while(!finished){
    const iter_domain_vector_type
      prev_snk_iter_vector(snk_iter.iteration_vector());

    finished = snk_iter.inc();
    if (finished){
      // we have started a new schedule period.
      schedule_period++;
    }

    
    // Get the absolute address for the iteration snk_iter
    bool valid_addr;
    long lin_addr = 
      calc_lin_addr(ref_window_iterator,
                    valid_addr,
                    schedule_period);

    // From this information, we derive the relative
    // difference to the previous iteration
    ref_point_addr_offset_table[prev_snk_iter_vector].rel_next_addr =
      lin_addr -
      ref_point_addr_offset_table[prev_snk_iter_vector].curr_abs_addr;
    ref_point_addr_offset_table[prev_snk_iter_vector].next_addr_valid = 
      valid_addr;

    //Set pointer to next iteration item
    //Not, that if finished = true,
    //the last item points to the beginning of the table.
    ref_point_addr_offset_table[prev_snk_iter_vector].next_iter_item =
      &ref_point_addr_offset_table[snk_iter.iteration_vector()];

    if (!finished){
      //Store absolute address
      ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr =
        lin_addr;
      ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_addr_valid =
        valid_addr;

    }
      
  }
  
}

void 
snk2addr_table_ref_point::optimize_invalid_addresses(){
  
  const unsigned int num_dimensions = 
    ref_point_addr_offset_table.dimensions();

  unsigned int coord1[num_dimensions];
  unsigned int coord2[num_dimensions];

  for(unsigned int i = 0;
      i < num_dimensions;
      i++){
    coord1[i] = coord2[i] = 0;
  }

  
}


bool 
snk2addr_table_ref_point::has_invalid_items(unsigned int fixed_dimension,
                                            unsigned int coord[]
                                            ) const{

  bool return_value = false;

  //move to next dimension
  fixed_dimension++;

  if (fixed_dimension < ref_point_addr_offset_table.dimensions()){
    for(unsigned int i = 0;
        (i < ref_point_addr_offset_table.size(fixed_dimension)) &&
          (!return_value);
        i++){
      coord[fixed_dimension] = i;
      return_value = has_invalid_items(fixed_dimension,
                                       coord);
    }    
  }else{
    if (ref_point_addr_offset_table[coord].next_addr_valid)
      return_value = false;
    else
      return_value = true;
  }

  return return_value;
}




/*
  different scenarios (U = not valid, undefined)
  U U U U U           U U U U U
  1 0 0 1 0    ->     1 0 0 1 0
  1 0 0 1 0           1 0 0 1 0
  U U U U U           1 0 0 1 0

  
  U U U U U           U U U U U
  U 0 1 0 U    ->     U 0 1 0 0
  U 0 1 U U           U 0 1 0 0  (!!)
  U U U U U           U 0 1 0 0
  That is bad, but should not exist!


  1 0 0 0 1
  U 0 0 0 U
  ...
  Should not exist neither!


  U U U U U           U U U U U
  U 0 1 0 U    ->     U 0 1 0 0
  U 0 1 1 U           U 0 1 1 1
  U U U U U           U 0 1 1 1
  This is bad and might exist.
 
 */

void
snk2addr_table_ref_point::make_subtable_equal_forward(unsigned int coord1[],
                                                      unsigned int coord2[],
                                                      unsigned int cur_dimension){

  const unsigned int num_table_dimensions = 
    ref_point_addr_offset_table.dimensions();

  if (cur_dimension >= num_table_dimensions)
    //nothing to do
    return;

  for(coord1[cur_dimension] = 0;
      coord1[cur_dimension] < ref_point_addr_offset_table.size(cur_dimension);
      coord1[cur_dimension]++){

    int status = -1;
    if (coord1[cur_dimension] > 0){
      coord2[cur_dimension] = coord1[cur_dimension]-1;

      //try to propagate in cur_dimension
      //from coord2 to coord1
      status = 
        propagate_addr(cur_dimension,
                       coord2,
                       coord1);
    }

    if (status < 1){
      //Either items cannot be made identical
      //or there stay undefined items.
      coord2[cur_dimension] = coord1[cur_dimension];
      make_subtable_equal_forward(coord1,
                                  coord2,
                                  cur_dimension+1);
      
    }
      
  }
}

int snk2addr_table_ref_point::propagate_addr(unsigned int fixed_dimension,
                                             unsigned int coord1[],
                                             unsigned int coord2[]){

  int return_value = 1;
  
  //move to next dimension
  fixed_dimension++;

  if (fixed_dimension >= ref_point_addr_offset_table.dimensions()){
    //try to propagate data from coord1 to coord2

    if (ref_point_addr_offset_table[coord1].next_addr_valid){
      if (ref_point_addr_offset_table[coord2].next_addr_valid){
        //Both data items are valid. Hence we cannot change anything
        if (ref_point_addr_offset_table[coord1].rel_next_addr !=
            ref_point_addr_offset_table[coord2].rel_next_addr){
          //Impossible to make items identical
          return -1;
        }else{
          //Items are already identical
          return 1;
        }
      }else{
        //propagate data from coord1 to coord2
        
        //Attention: be aware, that the element for the last
        //           iteration points to iteration (0,0,0,....)
        
        long rel_addr_difference = 
          ref_point_addr_offset_table[coord1].rel_next_addr -
          ref_point_addr_offset_table[coord2].rel_next_addr;

        ref_point_addr_offset_table[coord2].rel_next_addr +=
          rel_addr_difference;
        
        ref_point_addr_offset_table[coord2].next_iter_item->curr_abs_addr +=
          rel_addr_difference;

        ref_point_addr_offset_table[coord2].next_iter_item->rel_next_addr -=
          rel_addr_difference;

        // Items are now identical
        return 1;
      }
    }else{
      if (ref_point_addr_offset_table[coord2].next_addr_valid){
        //nothing to do for forward propagation
        return 1;
      }else{
        //coord 2 stays invalid
        return 0;
      }
    }
  }else{
    for(unsigned int i = 0;
        i < ref_point_addr_offset_table.size(fixed_dimension);
        i++){
      coord1[fixed_dimension] = i;
      coord2[fixed_dimension] = i;

      int temp = 
        propagate_addr(fixed_dimension,
                       coord1,
                       coord2);

      return_value =
        (temp < return_value) ? temp : return_value;

      if (return_value < 0)
        //cannot make it equal
        break;

    }
  }

  return return_value;
}



void
snk2addr_table_ref_point::make_subtable_equal_backward(unsigned int coord1[],
                                                       unsigned int coord2[],
                                                       unsigned int cur_dimension){

  const unsigned int num_table_dimensions = 
    ref_point_addr_offset_table.dimensions();

  if (cur_dimension >= num_table_dimensions)
    //nothing to do
    return;

  for(int cur_coord = ref_point_addr_offset_table.size(cur_dimension)-1;
      cur_coord >= 0;
      cur_coord--){

    coord1[cur_dimension] = cur_coord;

    int status = -1;
    if (coord1[cur_dimension] < ref_point_addr_offset_table.size(cur_dimension)-1){
      coord2[cur_dimension] = coord1[cur_dimension]+1;

      //try to propagate in cur_dimension
      //from coord2 to coord1
      status = 
        propagate_addr(cur_dimension,
                       coord2,
                       coord1);
    }

    if (status < 1){
      //Either items cannot be made identical
      //or there stay undefined items.
      coord2[cur_dimension] = coord1[cur_dimension];
      make_subtable_equal_backward(coord1,
                                   coord2,
                                   cur_dimension+1);
      
    }
      
  }
}



