//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include <cosupport/smoc_debug_out.hpp>
#include <cosupport/smoc_math.hpp>

#include <systemoc/smoc_wsdf_edge.hpp>

#define FAST_CALC_MODE

//0: No output
///100: debug
#ifndef VERBOSE_LEVEL_SMOC_WSDF_EDGE
#define VERBOSE_LEVEL_SMOC_WSDF_EDGE 0
#endif


smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_src_firing_block(udata_type block_size,
                                               unsigned token_dimension) const {

  return get_scm_firing_block(src_firing_blocks,
                              block_size,
                              token_dimension);
  
}


smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_snk_firing_block(udata_type block_size,
                                               unsigned token_dimension) const {

  if (block_size < delta_c(token_dimension)){
    // We have to split sliding window
    if (block_size <= 1)
      //always possible
      return 1;
    else if (c[token_dimension] <= block_size){
      //not possible to insert iteration level
      return 0;
    }

    return get_scm_firing_block(snk_window_firing_blocks,
                                block_size,
                                token_dimension);
    
  }else{
    
    if (block_size % delta_c(token_dimension)){
      //block size must be a multiple of delta_c
      return 0;
    }
    block_size /= delta_c(token_dimension);


    if (block_size <= 1)
      //block size does not require an own iterator level
      return true;

    
    return get_scm_firing_block(snk_firing_blocks,
                                block_size,
                                token_dimension);
    
  }
  
}


void 
smoc_wsdf_edge_descr::insert_src_firing_level(udata_type block_size,
                                              unsigned token_dimension){
  set_change_indicator();
  
  //start at smallest firing level
  unsigned firing_level = 0;

  if (block_size <= 1)
    //block size does not require an own iterator level
    return;

  while((firing_level < src_num_firing_levels) &&
        (src_firing_blocks[firing_level][token_dimension] < block_size)){
    firing_level++;
  }

  if (firing_level < src_num_firing_levels){
    if (src_firing_blocks[firing_level][token_dimension] == block_size){
      //block size already exists
      return;
    }
  }

  //create new firing block
  uvector_type new_block(token_dimensions);
  for(unsigned int i = 0; i < token_dimension; i++){
    if (firing_level == src_num_firing_levels)
      new_block[i] = src_firing_blocks[firing_level-1][i];
    else
      new_block[i] = src_firing_blocks[firing_level][i];
  }
  new_block[token_dimension] = block_size;
  for(unsigned int i = token_dimension+1; i < token_dimensions; i++){    
    if (firing_level == 0)
      new_block[i] = 1;
    else
      new_block[i] = src_firing_blocks[firing_level-1][i];
  }

  //insert firing block
  src_firing_blocks.insert_item(firing_level, new_block);
  if (firing_level < src_num_eff_token_firing_levels){
    //we have split effetive token
    src_num_eff_token_firing_levels++;
  }
  src_num_firing_levels++;
  
}


bool 
smoc_wsdf_edge_descr::insert_snk_firing_level(udata_type block_size,
                                              unsigned token_dimension){

  
  set_change_indicator();

  //start with smallest firing block
  unsigned firing_level = 0;

  //calculate effective block_size
  if (block_size < delta_c(token_dimension)){
    // we have to split sliding window
    if (block_size <= 1)
      //we do not require an own iterator level
      return true;
    else if (c[token_dimension] <= block_size){
      //not possible to insert iteration level
      return false;
    }
    
    while(snk_window_firing_blocks[firing_level][token_dimension] < 
          block_size){
      //Note, that loop terminates due to 
      //c[token_dimension] > block_size
      firing_level++;
    }

    //Check, if block already exists
    if (snk_window_firing_blocks[firing_level][token_dimension] == block_size){
      //block size already exists
      return true;
    }


    //create new firing block
    uvector_type new_block(token_dimensions);
    for(unsigned int i = 0; i < token_dimension; i++){      
      if (firing_level == snk_window_firing_blocks.size())
        new_block[i] = snk_window_firing_blocks[firing_level-1][i];
      else
        new_block[i] = snk_window_firing_blocks[firing_level][i];
    }
    new_block[token_dimension] = block_size;
    for(unsigned int i = token_dimension+1; i < token_dimensions; i++){
      if (firing_level == 0)
        new_block[i] = 1;
      else
        new_block[i] = snk_window_firing_blocks[firing_level-1][i];
    }
    
    //insert new firing block
    snk_window_firing_blocks.insert_item(firing_level, new_block);

    return true;
    
  }else{
    assert(block_size % delta_c(token_dimension) == 0);
    block_size /= delta_c(token_dimension);


    if (block_size <= 1)
      //block size does not require an own iterator level
      return true;

    while((firing_level < snk_num_firing_levels) &&
          (snk_firing_blocks[firing_level][token_dimension] < block_size)){
      firing_level++;
    }

    //Check, if block already exists.
    if (firing_level < snk_num_firing_levels){
      if (snk_firing_blocks[firing_level][token_dimension] == block_size){
        //block size already exists
        return true;
      }
    }

    //create new firing block
    uvector_type new_block(token_dimensions);
    for(unsigned int i = 0; i < token_dimension; i++){
      if (firing_level == snk_num_firing_levels)
        new_block[i] = snk_firing_blocks[firing_level-1][i];
      else
        new_block[i] = snk_firing_blocks[firing_level][i];
    }
    new_block[token_dimension] = block_size;
    for(unsigned int i = token_dimension+1; i < token_dimensions; i++){
      if (firing_level == 0)
        new_block[i] = 1;
      else
        new_block[i] = snk_firing_blocks[firing_level-1][i];
    }


    //Insert firing block
    snk_firing_blocks.insert_item(firing_level, new_block);
    snk_num_firing_levels++;

    return true;

  }
  
}


void 
smoc_wsdf_edge_descr::firing_levels_snk2src() {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::firing_levels_snk2src()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  
  /*
    Process sink window propagation
  */
  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    
    // Determine, whether sink block can
    // be represented by source.
    // If yes, determine the resulting block
    // size.
    udata_type res_block_size = 
      get_scm_src_firing_block(delta_c[token_dimension],
                               token_dimension);
    res_block_size *= 
      delta_c[token_dimension];
    
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "res_block_size = " << res_block_size << std::endl;
#endif
    
    if (get_scm_snk_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "Insert firing level" << std::endl;
#endif
      insert_src_firing_level(res_block_size,
                              token_dimension);
      insert_snk_firing_level(res_block_size,
                              token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "src_firing_blocks = " 
                      << src_firing_blocks << std::endl;
      CoSupport::dout << "snk_firing_blocks = " 
                      << snk_firing_blocks << std::endl;
#endif
      
    }else{
      //Either resulting block size cannot be integrated at all
      //or it belongs to next source firing block
    }    
  }
  

  /*
    Process sink firing blocks
   */
  for(unsigned int firing_level = 0; 
      firing_level < snk_num_firing_levels; 
      firing_level++){
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level 
                      << ", token_dimension = " << token_dimension 
                      << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
      CoSupport::dout << "snk_firing_blocks[firing_level][token_dimension] = "
                      << snk_firing_blocks[firing_level][token_dimension]
                      << std::endl;
#endif
      
      // Determine, whether sink block can
      // be represented by source.
      // If yes, determine the resulting block
      // size.
      udata_type res_block_size = 
        get_scm_src_firing_block(snk_firing_blocks[firing_level][token_dimension]*
                                 delta_c[token_dimension],
                                 token_dimension);
      res_block_size *= 
        snk_firing_blocks[firing_level][token_dimension]*
        delta_c[token_dimension];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "res_block_size = " << res_block_size << std::endl;
#endif

      if (get_scm_snk_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::dout << "Insert firing level" << std::endl;
#endif
        insert_src_firing_level(res_block_size,
                                token_dimension);
        insert_snk_firing_level(res_block_size,
                                token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::dout << "src_firing_blocks = " 
                        << src_firing_blocks << std::endl;
        CoSupport::dout << "snk_firing_blocks = " 
                        << snk_firing_blocks << std::endl;
#endif

      }else{
        //Either resulting block size cannot be integrated at all
        //or it belongs to next source firing block
      }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << CoSupport::Indent::Down;
#endif
      
    }
  }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::firing_levels_snk2src()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
  
}
  

void
smoc_wsdf_edge_descr::firing_levels_src2snk() {
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::firing_levels_src2snk()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  for(unsigned int firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level 
                      << ", token_dimension = " << token_dimension 
                      << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
      CoSupport::dout << "src_firing_blocks[firing_level][token_dimension] = "
                      << src_firing_blocks[firing_level][token_dimension]
                      << std::endl;
#endif
      
      // Determine, whether source block can
      // be represented by sink.
      // If yes, determine the resulting block
      // size.
      udata_type res_block_size = 
        get_scm_snk_firing_block(src_firing_blocks[firing_level][token_dimension],
                                 token_dimension);
      res_block_size *= src_firing_blocks[firing_level][token_dimension];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "res_block_size = " << res_block_size << std::endl;
#endif

      if (get_scm_src_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::dout << "Insert firing level" << std::endl;
#endif
        insert_src_firing_level(res_block_size,
                                token_dimension);
        insert_snk_firing_level(res_block_size,
                                token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::dout << "src_firing_blocks = " 
                        << src_firing_blocks << std::endl;
        CoSupport::dout << "snk_firing_blocks = " 
                        << snk_firing_blocks << std::endl;
#endif

      }else{
        //Either resulting block size cannot be integrated at all
        //or it belongs to next source firing block
      }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << CoSupport::Indent::Down;
#endif
      
    }
  }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::firing_levels_src2snk()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
}


smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_firing_block(u2vector_type firing_blocks,
                                           udata_type block_size,
                                           unsigned token_dimension) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  //default value, if no block_size does does not match
  //any firing block
  udata_type return_value = 0;

  if (block_size <= 1){
    //always possible
    return_value = 1;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "return_value = " << return_value << std::endl;
    CoSupport::dout << "Leave smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return return_value;
  }

  //look for block size which is dividable by block_size
  unsigned int firing_level = 0;
  udata_type prev_block_size = 1;
  while(firing_level < firing_blocks.size()){
    if (firing_blocks[firing_level][token_dimension] % block_size == 0){
      //found
      return_value = CoSupport::scm(prev_block_size, block_size);
      return_value /= block_size;
      break;
    }
    
    //Memorize previous block size
    firing_level++;
    if (firing_level < firing_blocks.size()){
      if (firing_blocks[firing_level][token_dimension] != 
          firing_blocks[firing_level-1][token_dimension]){
        prev_block_size = 
          firing_blocks[firing_level-1][token_dimension];
      }
    }
  }  


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "return_value = " << return_value << std::endl;
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_value;
}


const smoc_wsdf_edge_descr::uvector_type& 
smoc_wsdf_edge_descr::snk_iteration_max() const {

  if (cache_snk_iter_max_valid)
    return snk_iteration_max_cached;

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();
        
  uvector_type snk_vtu_iteration_level(token_dimensions);
  insert_snk_vtu_iterations(snk_iteration_level_table,
			    snk_vtu_iteration_level
			    );

  uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
						    snk_vtu_iteration_level));

  append_snk_window_iteration(iteration_max);

  snk_iteration_max_cached = iteration_max;
  cache_snk_iter_max_valid = true;

  return snk_iteration_max_cached;
}


const smoc_wsdf_edge_descr::uvector_type& 
smoc_wsdf_edge_descr::src_iteration_max() const {


  // return cached value, if valid.
  if (cache_src_iter_max_valid)
    return src_iteration_max_cached;

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter  smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  
  uvector_type return_vector(calc_src_iteration_levels());
  unsigned iter_level = calc_src_iteration_levels()-1;

  uvector_type current_firing_block_size(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    current_firing_block_size[i] = 1;

  for(unsigned firing_level = 0;
      firing_level < src_num_firing_levels;
      firing_level++){
    for(unsigned token_dimension = 0;
	token_dimension < token_dimensions;
	token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level;
      CoSupport::dout << " token_dimension = " << token_dimension << std::endl;
#endif
      if (src_has_iteration_level(firing_level, token_dimension)){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::dout << "Is associated to iteration level " 
                        << iter_level << std::endl;
#endif
	assert(src_firing_blocks[firing_level][token_dimension] %
	       current_firing_block_size[token_dimension] == 0);
	return_vector[iter_level] = 
	  src_firing_blocks[firing_level][token_dimension] /
	  current_firing_block_size[token_dimension] - 1;
                                
	iter_level--;
	current_firing_block_size[token_dimension] = 
	  src_firing_blocks[firing_level][token_dimension];
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Source iteration max: " 
                  << return_vector << std::endl;
#endif


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  cache_src_iter_max_valid = true;
  src_iteration_max_cached = return_vector;

  return src_iteration_max_cached;
}

smoc_wsdf_edge_descr::svector_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_vector() const {
  return -(bs);
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_matrix() const {

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();
        
  uvector_type snk_vtu_iteration_level(token_dimensions);
  insert_snk_vtu_iterations(snk_iteration_level_table,
			    snk_vtu_iteration_level
			    );

  uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
						    snk_vtu_iteration_level));

  append_snk_window_iteration(iteration_max);
  umatrix_type return_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
								  snk_vtu_iteration_level,
								  iteration_max));

#ifdef FAST_CALC_MODE  
  matrix_thin_out(return_matrix, iteration_max);
#endif  
        

  return return_matrix;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::src_data_element_mapping_vector() const {
  return d;
}

smoc_wsdf_edge_descr::smatrix_type 
smoc_wsdf_edge_descr::calc_border_condition_matrix() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix()" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();
        
  uvector_type snk_vtu_iteration_level(token_dimensions);
  insert_snk_vtu_iterations(snk_iteration_level_table,
			    snk_vtu_iteration_level
			    );

  uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
						    snk_vtu_iteration_level));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Iteration-Max (without window iteration): " 
                  << iteration_max << std::endl;
#endif

  append_snk_window_iteration(iteration_max);
  umatrix_type mapping_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
								   snk_vtu_iteration_level,
								   iteration_max));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Mapping-matrix: " << mapping_matrix << std::endl;
#endif

  smatrix_type return_matrix(mapping_matrix.size1(),mapping_matrix.size2());

  calc_border_condition_matrix(mapping_matrix,
			       snk_vtu_iteration_level,
			       return_matrix);

#ifdef FAST_CALC_MODE  
  matrix_thin_out(return_matrix, iteration_max);
#endif  

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif


  return return_matrix;
}


smoc_wsdf_edge_descr::svector_type
smoc_wsdf_edge_descr::calc_low_border_condition_vector() const {
  return bs;
}

smoc_wsdf_edge_descr::svector_type
smoc_wsdf_edge_descr::calc_high_border_condition_vector() const {
  svector_type return_vector(u0.size());

  for(unsigned int i = 0; i < u0.size(); i++){
    return_vector[i] = u0[i] + bs[i] - 1;
  }

  return return_vector;
}

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::max_data_element_id() const{
  uvector_type return_vector(src_firing_blocks[src_firing_blocks.size()-1]);
        
  for(unsigned int i = 0; i < return_vector.size(); i++){
    return_vector[i] --;
  }

  return return_vector;

}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::src_data_element_mapping_matrix() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::src_data_element_mapping_matrix()" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  uvector_type prev_mapping_factor(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    prev_mapping_factor[i] = 1;

  const unsigned matrix_rows = token_dimensions;
  const unsigned matrix_cols = calc_src_iteration_levels();       
  umatrix_type return_matrix(matrix_rows, matrix_cols);

  signed iter_level = calc_src_iteration_levels()-1;

  for(unsigned firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0;
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level, token_dimension)){
	//default assignment
	for(unsigned row = 0; row < matrix_rows; row++){
	  return_matrix(row, iter_level) = 0;
	}

	return_matrix(token_dimension,iter_level) =
	  prev_mapping_factor[token_dimension];
                                
	prev_mapping_factor[token_dimension] = 
	  src_firing_blocks[firing_level][token_dimension];

	iter_level--;
      }
    }
  }

  //error checking
  assert(iter_level == -1); //otherwise not all iteration levels covered

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Mapping matrix: " << return_matrix << std::endl;
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::src_data_element_mapping_matrix()" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_matrix;
}


smoc_wsdf_edge_descr::uvector_type
smoc_wsdf_edge_descr::calc_snk_r_vtu() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  uvector_type return_vector(u0+bs+bt-c);

  for(unsigned int i = 0; i < token_dimensions; i++){
    assert(return_vector[i] % delta_c[i] == 0); //Invalid parameters

    return_vector[i] /= delta_c[i];
    return_vector[i]++;
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "snk_r_vtu = " << return_vector;
  CoSupport::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_src_r_vtu() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_src_r_vtu()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  uvector_type return_vector(token_dimensions);
        
  for(unsigned int i = 0; i < token_dimensions; i++){
    //currently, we require, that an effective token
    //exactly belongs to one virtual token union
    assert(u0[i] % p[i] == 0);

    return_vector[i] = u0[i] / p[i];
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "src_r_vtu = " << return_vector;
  CoSupport::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_src_r_vtu()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}



void smoc_wsdf_edge_descr::check_local_balance() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::check_local_balance" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;

  CoSupport::dout << "snk_firing_blocks = " << snk_firing_blocks;
  CoSupport::dout << std::endl;
  CoSupport::dout << "c = " << c;
  CoSupport::dout << std::endl;

  CoSupport::dout << "src_firing_blocks = " << src_firing_blocks;
  CoSupport::dout << std::endl;
#endif

  //Calculate number of invocations per virtual token union
  const uvector_type snk_r_vtu(calc_snk_r_vtu());
  const uvector_type src_r_vtu(calc_src_r_vtu());

  //calculate the number of virtual token unions in each dimension
  udata_type snk_num_vtu;
  udata_type src_num_vtu;

  for(unsigned int i = 0; i < token_dimensions; i++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "Token dimension " << i << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
    //Check for incomplete virtual token union
    if (snk_firing_blocks[snk_num_firing_levels-1][i] % snk_r_vtu[i] != 0){
      std::cout << "Violation of balance equation in dimension " << i << std::endl;
      std::cout << "snk_firing_blocks[snk_num_firing_levels-1][i] = " 
		<< snk_firing_blocks[snk_num_firing_levels-1][i]
		<< std::endl;
      std::cout << "snk_r_vtu[i] = " 
		<< snk_r_vtu[i]
		<< std::endl;
      print_edge_parameters(std::cout);
      assert(false);
    }

    snk_num_vtu = 
      snk_firing_blocks[snk_num_firing_levels-1][i] / snk_r_vtu[i];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "snk_num_vtu =  " << snk_num_vtu  << std::endl;
#endif

    //Check for incomplete virtual token unions
    assert((src_firing_blocks[src_num_firing_levels-1][i] / p[i]) % src_r_vtu[i] == 0);

    src_num_vtu = 
      src_firing_blocks[src_num_firing_levels-1][i] / p[i] / src_r_vtu[i];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "src_num_vtu =  " << src_num_vtu  << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << CoSupport::Indent::Down;
#endif

                
    //Check, if edge balanced
    assert(snk_num_vtu == src_num_vtu);
  }       

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::check_local_balance" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
}


void smoc_wsdf_edge_descr::check_parameters() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::check_parameters" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "token_dimensions = " << token_dimensions << std::endl;
  CoSupport::dout << "v.size() = " << v.size() << std::endl;
#endif

  /* Check number of dimensions */
  assert(v.size() == token_dimensions);
  assert(u0.size() == token_dimensions);
  assert(c.size() == token_dimensions);
  assert(delta_c.size() == token_dimensions);
  assert(d.size() == token_dimensions);
  assert(bs.size() == token_dimensions);
  assert(bt.size() == token_dimensions);


  /* Check, that we only have complete firing blocks */
  //source
  for(unsigned int i = 0; i < token_dimensions; i++){             
    for(unsigned int j = 0; j < src_num_firing_levels-1; j++){
      assert(src_firing_blocks[j+1][i] % src_firing_blocks[j][i] == 0);
    }
  }

  //sink
  for(unsigned int i = 0; i < token_dimensions; i++){             
    for(unsigned int j = 0; j < snk_num_firing_levels-1; j++){
      assert(snk_firing_blocks[j+1][i] % snk_firing_blocks[j][i] == 0);
    }               
  }

  check_local_balance();

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::check_parameters" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
}


bool smoc_wsdf_edge_descr::snk_has_iteration_level(unsigned firing_level,
						   unsigned token_dimension,
                                                   u2vector_type snk_firing_blocks) const {

  if (firing_level == 0){
    if (snk_firing_blocks[firing_level][token_dimension] == 1){
      //ingore firing blocks of size one except if all firing blocks
      //have the size one
      if (snk_firing_blocks[snk_firing_blocks.size()-1][token_dimension] == 1){
        return true;
      }else{
        return false;
      }
    }else{
      return true;
    }
  }else if (snk_firing_blocks[firing_level][token_dimension] == 
	    snk_firing_blocks[firing_level-1][token_dimension]){
    return false;
  }else if (snk_firing_blocks[firing_level][token_dimension] == 1){
    return false;
  }else{
    return true;
  }
}

bool smoc_wsdf_edge_descr::src_has_iteration_level(unsigned firing_level,
						   unsigned token_dimension) const {
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::src_has_iteration_level" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
  CoSupport::dout << "firing_level = " << firing_level << std::endl;
  CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
  CoSupport::dout << "src_num_firing_levels = " << src_num_firing_levels << std::endl;
#endif

  bool return_value = false;

  if (firing_level <= 0){
    //Always include firing_level 0 (effective token)
    return_value = true;
  }else if ((firing_level == 1) && 
            (src_firing_blocks[firing_level][token_dimension] ==
             src_firing_blocks[src_num_firing_levels-1][token_dimension])){
    //All firing levels have the same size.
    //Return at least one not belong to the effective token.
    return_value =  true;
  }else if (src_firing_blocks[firing_level][token_dimension] == 1){
    //firing blocks of size 1 need not to be considered
    return_value = false;
  }else if (src_firing_blocks[firing_level][token_dimension] == 
	    src_firing_blocks[firing_level-1][token_dimension]){
    return_value = false;
  }else{
    return_value =  true;
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::src_has_iteration_level" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_value;


}


smoc_wsdf_edge_descr::s2vector_type 
smoc_wsdf_edge_descr::calc_src_iteration_level_table() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter  smoc_wsdf_edge_descr::calc_src_iteration_level_table()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  unsigned iteration_level = 0;

  s2vector_type return_table(src_firing_blocks.size()-1,svector_type(token_dimensions));
        

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Size of return-table: " << return_table.size() << std::endl;
#endif

  for(int firing_level = src_num_firing_levels-1; 
      firing_level >= 1; //exclude effective token
      firing_level--){
    for(int token_dimension = token_dimensions-1; 
	token_dimension >= 0; 
	token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level << std::endl;
      CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
      CoSupport::dout << "iteration_level = " << iteration_level << std::endl;
#endif
                        
      if (src_has_iteration_level(firing_level, token_dimension)){
	return_table[firing_level-1][token_dimension] = iteration_level;
	iteration_level++;
      }else{
	return_table[firing_level-1][token_dimension] = -1;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave  smoc_wsdf_edge_descr::calc_src_iteration_level_table()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_table;
}



void smoc_wsdf_edge_descr::print_edge_parameters(std::ostream& os) const{
  os << "Token dimensions: " << token_dimensions << std::endl;
  os << "p = " << p << std::endl;
  os << "u0 = " << u0 << std::endl;
  os << "c = " << c << std::endl;
  os << "delta_c = " << delta_c << std::endl;
  os << "d = " << d << std::endl;
  os << "bs = " << bs << std::endl;
  os << "bt = " << bt << std::endl;
  os << "src_firing_blocks = " << src_firing_blocks << std::endl;
  os << "snk_firing_blocks = " << snk_firing_blocks << std::endl;
}


smoc_wsdf_edge_descr::s2vector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_level_table() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  unsigned iteration_level = 0;

  //Generate a data structure with the same number of data elements than
  //snk_firing_blocks
  s2vector_type return_table(snk_firing_blocks.size(),svector_type(token_dimensions));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Size of return-table: " << return_table.size() << std::endl;
#endif

  for(int firing_level = snk_num_firing_levels-1; 
      firing_level >= 0; 
      firing_level--){
    for(int token_dimension = token_dimensions-1; 
	token_dimension >= 0; 
	token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level << std::endl;
      CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
      CoSupport::dout << "iteration_level = " << iteration_level << std::endl;
#endif
                        
      if (snk_has_iteration_level(firing_level, 
                                  token_dimension,
                                  snk_firing_blocks)){
	return_table[firing_level][token_dimension] = iteration_level;
	iteration_level++;
      }else{
	return_table[firing_level][token_dimension] = -1;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_table;
}


void smoc_wsdf_edge_descr::insert_snk_vtu_iterations(s2vector_type& iteration_level_table,
						     uvector_type& vtu_iteration_level,
						     bvector_type& new_vtu_iteration) const {


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::insert_snk_vtu_iterations" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  unsigned level_inc = 0;
  bool found[token_dimensions];

  uvector_type snk_r_vtu(calc_snk_r_vtu());

  //init found table
  for(unsigned i = 0; i < token_dimensions; i++){
    found[i] = false;
  }

  for(int firing_level = snk_num_firing_levels-1;
      firing_level >= 0;
      firing_level--){
    for(int token_dimension = token_dimensions-1;
	token_dimension >= 0;
	token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "firing_level = " << firing_level << std::endl;
      CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
#endif

      //update iteration level table in order to take
      //previous modifications into account
      if (iteration_level_table[firing_level][token_dimension] >= 0)
	iteration_level_table[firing_level][token_dimension] += level_inc;

                                                
      if (!found[token_dimension]){
	if (iteration_level_table[firing_level][token_dimension] >= 0){
	  if (snk_firing_blocks[firing_level][token_dimension] == 
	      snk_r_vtu[token_dimension]){
                                                
	    //we have found a block which corresponds to the
	    //virtual token union
	    vtu_iteration_level[token_dimension] = 
	      iteration_level_table[firing_level][token_dimension];

	    found[token_dimension] = true;
	    new_vtu_iteration[token_dimension] = false;
	  }else if (snk_firing_blocks[firing_level][token_dimension] <
		    snk_r_vtu[token_dimension]){
                                                
	    // we have not found a block matching the vtu
	    // However, we the firing blocks are already getting to
	    // small. Hence, we have to add an iteration level

	    vtu_iteration_level[token_dimension] = 
	      iteration_level_table[firing_level][token_dimension];
	    iteration_level_table[firing_level][token_dimension]++;
	    level_inc++;
	    found[token_dimension] = true;
	    new_vtu_iteration[token_dimension] = true;
	  }else if (firing_level == 0){
	    //the vtu is smaller than the smallest firing block
	    vtu_iteration_level[token_dimension] = 
	      iteration_level_table[firing_level][token_dimension]+1;
	    level_inc++;
	    found[token_dimension] = true;
	    new_vtu_iteration[token_dimension] = true;
	  }
	}
      }
    }
  }

  //error checking
  for(unsigned i = 0; i < token_dimensions; i++){
    assert(found[i]);
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "iteration_level_table = " << iteration_level_table << std::endl;
  CoSupport::dout << "vtu_iteration_level = " << vtu_iteration_level << std::endl;
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::insert_snk_vtu_iterations" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

}

void smoc_wsdf_edge_descr::insert_snk_vtu_iterations(s2vector_type& snk_iteration_level_table,
						     uvector_type& snk_vtu_iteration_level
						     ) const {
  bvector_type dummy(snk_vtu_iteration_level.size());
        
  insert_snk_vtu_iterations(snk_iteration_level_table,
			    snk_vtu_iteration_level,
			    dummy);
}

unsigned 
smoc_wsdf_edge_descr::get_num_iteration_levels(const s2vector_type& snk_iteration_level_table,
					       const uvector_type& snk_vtu_iteration_level) const {

  unsigned return_value;

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  if(snk_iteration_level_table.max_value() > (sdata_type)snk_vtu_iteration_level[0])
    return_value = snk_iteration_level_table.max_value()+1;
  else
    return_value = snk_vtu_iteration_level[0] + 1;

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "return_value = " << return_value << std::endl;
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_value;
}


unsigned smoc_wsdf_edge_descr::calc_src_iteration_levels() const {
  unsigned return_value = 0;


  for(unsigned firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level,token_dimension)){
	return_value++;
      }
    }
  }
  return return_value;
}


unsigned smoc_wsdf_edge_descr::calc_window_iteration_levels() const {
  unsigned return_value = 0;


  for(unsigned firing_level = 0; 
      firing_level < snk_window_firing_blocks.size(); 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){
	return_value++;
      }
    }
  }
  return return_value;
}

unsigned smoc_wsdf_edge_descr::calc_eff_token_iteration_levels() const{

  unsigned return_value = 0;


  for(unsigned firing_level = 0; 
      firing_level < src_num_eff_token_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level,
                                  token_dimension)){
	return_value++;
      }
    }
  }
  return return_value;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_max(const s2vector_type& snk_iteration_level_table,
					     const uvector_type& snk_vtu_iteration_level
					     ) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
  CoSupport::dout << "snk_iteration_level_table.size() = " 
                  << snk_iteration_level_table.size();
#endif

  unsigned num_iteration_levels = get_num_iteration_levels(snk_iteration_level_table,
							   snk_vtu_iteration_level
							   );

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "num_iteration_levels = " << num_iteration_levels << std::endl;
#endif


  uvector_type return_vector(num_iteration_levels);
  bool found_vtu[token_dimensions];

  uvector_type snk_r_vtu(calc_snk_r_vtu());

  uvector_type current_firing_block_size(token_dimensions);

  for(unsigned int i = 0; i < token_dimensions; i++)
    found_vtu[i] = false;

  for(unsigned int i = 0; i < token_dimensions; i++)
    current_firing_block_size[i] = 1;

  for(unsigned token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){

    for(unsigned firing_level = 0; 
	firing_level < snk_num_firing_levels; 
	firing_level++){
      if(snk_iteration_level_table[firing_level][token_dimension] >= 0){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	CoSupport::dout << "firing_level = " << firing_level << std::endl;
	CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
#endif

	// check for complete blocks
	assert(snk_firing_blocks[firing_level][token_dimension] %
	       current_firing_block_size[token_dimension] == 0);
                                
	return_vector[snk_iteration_level_table[firing_level][token_dimension]] = 
	  snk_firing_blocks[firing_level][token_dimension] /
	  current_firing_block_size[token_dimension]
	  -1;                             

	// Check, if vtu is covered
	if ((sdata_type)snk_vtu_iteration_level[token_dimension] ==
	    snk_iteration_level_table[firing_level][token_dimension]){
                                        
	  found_vtu[token_dimension] = true;                                      
	}

	if (snk_firing_blocks[firing_level][token_dimension] >
	    snk_r_vtu[token_dimension]){
	  if (!found_vtu[token_dimension]){
	    //Firing blocks are already larger as vtu,
	    //but vtu is not already covered

	    //Calculate the number of vtu insight the vtu
	    unsigned num_vtu = 
	      snk_firing_blocks[firing_level][token_dimension] / 
	      snk_r_vtu[token_dimension];

	    //Check, that firing block is a multiple of the vtu
	    assert(snk_firing_blocks[firing_level][token_dimension] % 
		   snk_r_vtu[token_dimension] == 0);
	    return_vector[snk_iteration_level_table[firing_level][token_dimension]] = 
	      num_vtu - 1;

	    //Assign loop borders for vtu
	    assert(snk_r_vtu[token_dimension] %     current_firing_block_size[token_dimension] == 0);
	    return_vector[snk_vtu_iteration_level[token_dimension]] = 
	      snk_r_vtu[token_dimension] /
	      current_firing_block_size[token_dimension]
	      -1;

	    found_vtu[token_dimension] = true;
                                                
	  }
	}

	// Update firing block size
	current_firing_block_size[token_dimension] = snk_firing_blocks[firing_level][token_dimension];
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}




void smoc_wsdf_edge_descr::append_snk_window_iteration(uvector_type& iteration_max) const {

  const unsigned old_size = iteration_max.size();
  const unsigned window_iteration_levels = calc_window_iteration_levels();
  const unsigned new_size = old_size + window_iteration_levels;
  iteration_max.resize(new_size);

  uvector_type current_block_size(token_dimensions,1);

  unsigned int iter_level = new_size-1;
  for(unsigned int firing_level = 0;
      firing_level < snk_window_firing_blocks.size();
      firing_level++){
    // Start with smallest block size.
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){
        iteration_max[iter_level] = 
          snk_window_firing_blocks[firing_level][token_dimension]/
          current_block_size[token_dimension]
          -1;
        iter_level--;
        current_block_size[token_dimension] = 
          snk_window_firing_blocks[firing_level][token_dimension];
      }
    }
  }
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix(const s2vector_type& snk_iteration_level_table,
							   const uvector_type& snk_vtu_iteration_level,
							   const uvector_type& snk_iter_max
							   ) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  uvector_type prev_mapping_factor(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    prev_mapping_factor[i] = delta_c[i];

  bool found_vtu[token_dimensions];
  for(unsigned int i = 0; i < token_dimensions; i++)
    found_vtu[i] = false;

  const unsigned matrix_rows = token_dimensions;
  const unsigned matrix_cols = 
    get_num_iteration_levels(snk_iteration_level_table,snk_vtu_iteration_level) +
    calc_window_iteration_levels();
        
  umatrix_type return_matrix(matrix_rows, matrix_cols);

  for(unsigned token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    for(unsigned firing_level = 0; 
	firing_level < snk_num_firing_levels; 
	firing_level++){                        
      if(snk_iteration_level_table[firing_level][token_dimension] >= 0){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	CoSupport::dout << "firing_level = " << firing_level << std::endl;
	CoSupport::dout << "token_dimension = " << token_dimension << std::endl;
#endif

	//default assignment
	for(unsigned row = 0; row < matrix_rows; row++){
	  return_matrix(row, snk_iteration_level_table[firing_level][token_dimension]) = 0;
	}

	return_matrix(token_dimension,snk_iteration_level_table[firing_level][token_dimension]) =
	  prev_mapping_factor[token_dimension];
                                
	//check, whether iteration level represents vtu
	if ((sdata_type)snk_vtu_iteration_level[token_dimension] == 
	    snk_iteration_level_table[firing_level][token_dimension]){
	  //vtu requires special attention
	  prev_mapping_factor[token_dimension] =
	    u0[token_dimension];
	  found_vtu[token_dimension] = true;
	}else if(((sdata_type)snk_vtu_iteration_level[token_dimension] > 
		  snk_iteration_level_table[firing_level][token_dimension]) &&
		 (!found_vtu[token_dimension])){
	  //Firing block is already larger than vtu. However, vtu has not already
	  //been covered

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	  CoSupport::dout << "Insert vtu" << std::endl;
#endif

	  //Insert data element mapping for vtu
	  for(unsigned row = 0; row < matrix_rows; row++){
	    return_matrix(row, snk_vtu_iteration_level[token_dimension]) = 0;
	  }
	  return_matrix(token_dimension,snk_vtu_iteration_level[token_dimension]) =
	    prev_mapping_factor[token_dimension];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	  CoSupport::dout << "prev_mapping_factor[token_dimension] = " 
		    << prev_mapping_factor[token_dimension] << std::endl;
#endif
                                        
	  //Correct data mapping weighths
	  return_matrix(token_dimension,snk_iteration_level_table[firing_level][token_dimension]) =
	    u0[token_dimension];

	  //Calculate new data element mapping factor
	  prev_mapping_factor[token_dimension] = u0[token_dimension] * 
	    (snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] + 1);

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	  CoSupport::dout << "snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] = " 
		    << snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] << std::endl;
#endif

	  found_vtu[token_dimension] = true;
                                        
	}else{
	  prev_mapping_factor[token_dimension] *= 
	    (snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] + 1);
	}
      }
    }
  }       

        
  // Add special iterations
  insert_snk_window_mapping(return_matrix,
                            snk_iter_max);

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_matrix;

}


void 
smoc_wsdf_edge_descr::insert_snk_window_mapping(umatrix_type& data_element_mapping_matrix,
                                                const uvector_type& snk_iter_max) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::insert_snk_window_mapping" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
  CoSupport::dout << "Matrix-Cols: " << data_element_mapping_matrix.size2() << std::endl;
  CoSupport::dout << "snk_window_firing_blocks.size() = " 
                  << snk_window_firing_blocks.size() << std::endl;
  CoSupport::dout << "snk_iter_max.size() = " << snk_iter_max.size() << std::endl;
  CoSupport::dout.flush();
#endif


  const unsigned matrix_cols = data_element_mapping_matrix.size2();

  uvector_type current_block_size(token_dimensions,1);

  unsigned int iter_level = matrix_cols-1;
  for(unsigned int firing_level = 0;
      firing_level < snk_window_firing_blocks.size();
      firing_level++){
    // Start with smallest block size.

    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){

        // Default value
        for(unsigned int i = 0;
            i < token_dimensions;
            i++)
          data_element_mapping_matrix(i, iter_level) = 0;

        data_element_mapping_matrix(token_dimension, iter_level) =
          current_block_size(token_dimension);
        current_block_size(token_dimension) *=
          snk_iter_max(iter_level)+1;

        iter_level--;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::insert_snk_window_mapping" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
  CoSupport::dout.flush();
#endif
}



void
smoc_wsdf_edge_descr::calc_border_condition_matrix(const umatrix_type& mapping_matrix,
						   const uvector_type& snk_vtu_iteration_level,
						   smatrix_type& border_cond_matrix) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
        
  const unsigned num_rows = mapping_matrix.size1();
  const unsigned num_cols = mapping_matrix.size2();


  for(unsigned row = 0; row < num_rows; row++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::dout << "row = " << row 
	      << ", vtu-iteration level = " 
	      << snk_vtu_iteration_level[row] << std::endl;
#endif
    for(unsigned col = 0; 
	col < snk_vtu_iteration_level[row];
	col++){
      border_cond_matrix(row,col) = 0;
    }
    for(unsigned col = snk_vtu_iteration_level[row];
	col < num_cols;
	col++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::dout << "col = " << col << std::endl; 
      CoSupport::dout << "mapping_matrix(row, col) = " 
		<< mapping_matrix(row, col) << std::endl;
#endif
      border_cond_matrix(row, col) = mapping_matrix(row, col);
    }
  }       

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::dout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

}

