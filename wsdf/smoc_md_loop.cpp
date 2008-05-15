//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:


#include <systemoc/smoc_md_loop.hpp>


#define FAST_CALC_MODE
#define FAST_CALC_MODE2

#ifndef VERBOSE_LEVEL_SMOC_MD_LOOP
#define VERBOSE_LEVEL_SMOC_MD_LOOP 0
// 100: verbose execution
// 101: general debug
// 102: memory access error
// 103: get_req_src_data_element
// 104: calc_eff_window_displacement
// 105: max_data_element_id
// 106: Border processing
#endif

/* ******************************************************************************* */
/*                              smoc_md_loop_iterator_kind                         */
/* ******************************************************************************* */


const smoc_md_loop_iterator_kind::iter_domain_vector_type 
smoc_md_loop_iterator_kind::iteration_size() const{
  iter_domain_vector_type return_vector(iteration_max());

  for(unsigned int i = 0; i < return_vector.size(); i++){
    return_vector[i]++;
  }

  return return_vector;
}

void smoc_md_loop_iterator_kind::set_window_iterator(const iter_domain_vector_type& window_iter){
  for(unsigned int i = 0, 
        j = current_iteration.size() - _token_dimensions;
      i < _token_dimensions;
      i++, j++){
    current_iteration[j] = window_iter[i];
  }
}

bool smoc_md_loop_iterator_kind::inc_window(){
  const iter_domain_vector_type& 
    win_iter_max(max_window_iteration());

  bool return_value = true;

  for(int i = _token_dimensions-1, 
        j = current_iteration.size() - 1;
      i >= 0;
      i--, j--){
    current_iteration[j]++;
    if (current_iteration[j] >= win_iter_max[i]){
      current_iteration[j] = 0;
    }else{
      return_value = false;
      break;
    }
  }

  return return_value;
}


/* ******************************************************************************* */
/*                     smoc_md_loop_data_element_mapper                        */
/* ******************************************************************************* */

smoc_vector<int> 
smoc_md_loop_data_element_mapper::calc_mapping_table(const mapping_matrix_type& mapping_matrix) const {

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Enter smoc_md_loop_data_element_mapper::calc_mapping_table" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "Mapping-matrix: " << mapping_matrix << std::endl;
#endif

  smoc_vector<int> return_vector(mapping_matrix.size2());

  for(unsigned int col = 0; col < mapping_matrix.size2(); col++){
    bool found = false;
    for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
      if (mapping_matrix(row,col) > 0){
	return_vector[col] = row;
	found = true;
	break;
      }
    }
    if (!found){
      return_vector[col] = -1;
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Mapping table: " << return_vector;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "Leave smoc_md_loop_data_element_mapper::calc_mapping_table" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}

bool 
smoc_md_loop_data_element_mapper::check_matrix(const mapping_matrix_type& mapping_matrix) const {
  //in each column, only one element is allowed to be non-zero
        
  for(unsigned int col = 0; col < mapping_matrix.size2(); col++){
    bool non_zero = false;
    for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
      if (mapping_matrix(row,col) > 0){
	if (non_zero){
	  //check failed
	  return false;
	}else{
	  non_zero = true;
	}
      }
    }
  }

  return true;
}

#if 0
smoc_vector<smoc_vector<unsigned int> > 
smoc_md_loop_data_element_mapper::calc_iteration_table(const mapping_matrix_type& mapping_matrix) const {
  smoc_vector<smoc_vector<unsigned int> >  return_list (mapping_matrix.size1());

  for(unsigned row = 0; row < mapping_matrix.size1(); row++){
    smoc_vector<unsigned int> temp_vector(0);

    for (unsigned col = 0; col < mapping_matrix.size2(); col++){
      if(mapping_matrix(row,col) != 0){
	temp_vector.resize(temp_vector.size()+1);
	temp_vector[temp_vector.size()-1] = col;
      }
    }

    return_list[row] = temp_vector;
  }

  return return_list;
        
}
#endif


/* ******************************************************************************* */
/*                     smoc_src_md_loop_iterator_kind                        */
/* ******************************************************************************* */
const smoc_src_md_loop_iterator_kind::data_element_id_type& 
smoc_src_md_loop_iterator_kind::schedule_period_max_data_element_id() const {
  return _max_data_element_id;
}

void smoc_src_md_loop_iterator_kind::calc_schedule_period_offset(data_element_id_type& data_element_id,
								 id_type& schedule_period_offset) const{
  //Calculate schedule period offset
  schedule_period_offset = 
    data_element_id[ _token_dimensions - 1] / (_max_data_element_id[_token_dimensions-1]+1);

  data_element_id[_token_dimensions-1] -= 
    schedule_period_offset *  (_max_data_element_id[_token_dimensions-1]+1);
}

void smoc_src_md_loop_iterator_kind::get_data_element_id(const iter_domain_vector_type& iteration_vector,
							 data_element_id_type& data_element_id,
							 id_type& schedule_period_offset
							 ) const {
  data_element_id = mapping_offset;
#ifdef FAST_CALC_MODE
  for(unsigned int col = 0; col < iteration_vector.size(); col++){
    const int row = mapping_table[col];
    if (row >= 0){
      data_element_id[row] += 
	mapping_matrix(row,col) * iteration_vector[col];
    }
  }
#if 0
  for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
    const unsigned num_col_iter = iteration_table[row].size();
    for(unsigned col_iter = 0; col_iter < num_col_iter; col_iter++){
      const unsigned col = iteration_table[row][col_iter];
      data_element_id[row] += 
	mapping_matrix(row,col) * iteration_vector[col];
    }
  }
#endif
#else
  data_element_id += prod(mapping_matrix, iteration_vector);
#endif

  calc_schedule_period_offset(data_element_id, schedule_period_offset);
        
}

bool smoc_src_md_loop_iterator_kind::inc(){
  update_base_data_element_id();
  return true;
}


const smoc_src_md_loop_iterator_kind::data_element_id_type& 
smoc_src_md_loop_iterator_kind::get_base_data_element_id() const {
  return base_data_element_id;
}

void smoc_src_md_loop_iterator_kind::get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
								    data_element_id_type& data_element_offset) const {
  // init return value
  for(unsigned int i = 0; 
      i < data_element_offset.size(); 
      i++){
    data_element_offset[i] = 0;
  }

#ifdef FAST_CALC_MODE
  for(unsigned int delta_col = 0, col = mapping_matrix.size2() - _token_dimensions; 
      delta_col < data_element_offset.size(); 
      delta_col++, col++){
    const int row = mapping_table[col];
    if (row >= 0){
      data_element_offset[row] +=
	mapping_matrix(row,col) * window_iteration[delta_col];
    }               
  }
#else
  for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
    for(unsigned int delta_col = 0, col = mapping_matrix.size2() - _token_dimensions; 
	delta_col < data_element_offset.size(); 
	delta_col++, col++){
      data_element_offset[row] +=
	mapping_matrix(row,col) * window_iteration[delta_col];
                                
    }
  }
#endif
}

void smoc_src_md_loop_iterator_kind::max_data_element_id(
							 data_element_id_type& max_data_element_id,
							 id_type& schedule_period_offset) const{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
  CoSupport::Streams::dout << "Enter smoc_src_md_loop_iterator_kind::max_data_element_id" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  //get the data element with the larges coordinates      
  const iter_domain_vector_type& 
    max_window_iteration(this->max_window_iteration());

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
  CoSupport::Streams::dout << "max_window_iteration = " << max_window_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

  get_window_data_element_offset(max_window_iteration,
				 max_data_element_id);
  max_data_element_id += base_data_element_id;

  calc_schedule_period_offset(max_data_element_id, schedule_period_offset);

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
  CoSupport::Streams::dout << "Leave smoc_src_md_loop_iterator_kind::max_data_element_id" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
}

const smoc_src_md_loop_iterator_kind::data_element_id_type 
smoc_src_md_loop_iterator_kind::size_token_space() const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Enter smoc_src_md_loop_iterator_kind::size_token_space()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif
  data_element_id_type return_vector(_max_data_element_id);

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Size of return_vector: " << return_vector.size() << std::endl;
#endif

  for(unsigned i = 0; i < return_vector.size(); i++){
    return_vector[i]++;
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Leave smoc_src_md_loop_iterator_kind::size_token_space()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}


bool
smoc_src_md_loop_iterator_kind::get_src_loop_iteration(const data_element_id_type& src_data_el_id,
						       iter_domain_vector_type& iteration_vector,
						       id_type& schedule_period_offset
						       ) const {

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Enter smoc_src_md_loop_iterator_kind::get_src_loop_iteration" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;

  CoSupport::Streams::dout << "Source data element ID: " << src_data_el_id;
  CoSupport::Streams::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "iteration_vector.size() = " << iteration_vector.size() << std::endl;
#endif



  bool return_value = true;

  //subtract offset (initial data elements)
  data_element_id_type temp_id(src_data_el_id - mapping_offset);  

  //Check, that data elements are still situated in token space
  for(unsigned int i = 0; i < _token_dimensions-1; i++){
    if (temp_id[i] < 0){
      //data element is not produced by source actor
#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
      CoSupport::Streams::dout << "Data element not produced by source actor" << std::endl;
#endif
      return_value = false;
      //goto smoc_src_md_loop_iterator_kind_get_src_loop_iteration_end;
#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
      CoSupport::Streams::dout << "Leave smoc_src_md_loop_iterator_kind::get_src_loop_iteration" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

      return return_value;
    }
  }

  //Calculate schedule period offset
  if (temp_id[_token_dimensions-1] < 0){
    // (divide and round up)
    schedule_period_offset = 
      (-temp_id[_token_dimensions-1] + _max_data_element_id[_token_dimensions-1]) / 
      (_max_data_element_id[_token_dimensions-1]+1);
    temp_id[_token_dimensions-1] += schedule_period_offset * 
      (_max_data_element_id[_token_dimensions-1]+1);
    schedule_period_offset *= -1;
  }else{
    schedule_period_offset = 0;
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "schedule_period_offset = " << schedule_period_offset << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Calculate iteration vector ..." << std::endl;
#endif
  for(unsigned i = 0; i < mapping_table.size(); i++){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::Streams::dout << "i = " << i
		    << " mapping_table[i] = " << mapping_table[i] << std::endl;
    CoSupport::Streams::dout << "iteration_vector.size() = " << iteration_vector.size() << std::endl;
#endif
    const int row = mapping_table[i];
    if (row >= 0) {
      iteration_vector[i] = 
	temp_id[row] / mapping_matrix(row,i);
      temp_id[row] -= 
	iteration_vector[i] * mapping_matrix(row,i);
    }else{
      // Iteration vector can only have the value zero.
      iteration_vector[i] = 0;
    }
  }


  //smoc_src_md_loop_iterator_kind_get_src_loop_iteration_end:

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Leave smoc_src_md_loop_iterator_kind::get_src_loop_iteration" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_value;
        
}

void smoc_src_md_loop_iterator_kind::update_base_data_element_id(){

  base_data_element_id = mapping_offset;

#ifdef FAST_CALC_MODE
  for(unsigned int col = 0; 
      col < mapping_matrix.size2() - _token_dimensions; 
      col++){
    const int row = mapping_table[col];
    if (row >= 0){
      base_data_element_id[row] += 
	mapping_matrix(row,col) * current_iteration[col];
    }
  }
#else
  for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
    for(unsigned int col = 0; 
	col < mapping_matrix.size2() - _token_dimensions; 
	col++){
      base_data_element_id[row] += 
	mapping_matrix(row,col) * current_iteration[col];
    }
  }       
#endif
}

/* ******************************************************************************* */
/*                     smoc_snk_md_loop_iterator_kind                        */
/* ******************************************************************************* */

void smoc_snk_md_loop_iterator_kind::get_data_element_id(const iter_domain_vector_type& iteration_vector,
							 data_element_id_type& data_element_id
							 ) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::get_data_element_id";
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  data_element_id = mapping_offset;
#ifdef FAST_CALC_MODE
  for(unsigned int col = 0; col < iteration_vector.size(); col++){
    const int row = mapping_table[col];
    if (row >= 0){
      data_element_id[row] += 
	mapping_matrix(row,col) * iteration_vector[col];
    }
  }
#else
  data_element_id += prod(mapping_matrix, iteration_vector);
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::get_data_element_id";
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

}

bool smoc_snk_md_loop_iterator_kind::inc(){
  update_base_data_element_id();
  update_base_border_condition_vector();
  return true;
}

const smoc_snk_md_loop_iterator_kind::data_element_id_type& 
smoc_snk_md_loop_iterator_kind::get_base_data_element_id() const {
  return base_data_element_id;
}

const smoc_snk_md_loop_iterator_kind::id_type 
smoc_snk_md_loop_iterator_kind::get_base_data_element_id(unsigned int token_dimension) const {
  return base_data_element_id[token_dimension];
}

void smoc_snk_md_loop_iterator_kind::get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
								    data_element_id_type& data_element_offset) const {
  // init return value
  for(unsigned int i = 0; 
      i < data_element_offset.size(); 
      i++){
    data_element_offset[i] = 0;
  }

#ifdef FAST_CALC_MODE
  for(unsigned int delta_col = 0, col = mapping_matrix.size2() - _token_dimensions; 
      delta_col < data_element_offset.size(); 
      delta_col++, col++){
    const int row = mapping_table[col];
    if (row >= 0){
      data_element_offset[row] +=
	mapping_matrix(row,col) * window_iteration[delta_col];
    }                               
  }
#else
  for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
    for(unsigned int delta_col = 0, col = mapping_matrix.size2() - _token_dimensions; 
	delta_col < data_element_offset.size(); 
	delta_col++, col++){
      data_element_offset[row] +=
	mapping_matrix(row,col) * window_iteration[delta_col];
                                
    }
  }
#endif
}



bool smoc_snk_md_loop_iterator_kind::get_req_src_data_element(data_element_id_type& data_element_id) const {
#if (VERBOSE_LEVEL_SMOC_MD_LOOP == 103) || (VERBOSE_LEVEL_SMOC_MD_LOOP == 102)
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::get_req_src_data_element" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "high_border_condition_vector = " << high_border_condition_vector;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "low_border_condition_vector = " << low_border_condition_vector;
  CoSupport::Streams::dout << std::endl;
#endif
  bool return_value = true;

  // Get the maximum iteration vector for the given window position
  iter_domain_vector_type 
    window_iteration(max_window_iteration());
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Max window iteration: " << window_iteration;
  CoSupport::Streams::dout << std::endl;
#endif


  const border_condition_vector_type&
    temp_vector(get_base_border_condition_vector());

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Base border condition: " << temp_vector;
  CoSupport::Streams::dout << std::endl;
#endif
        
  /* Check, whether complete window is situated on the higher extended border */
  for(unsigned dim = 0; 
      dim < _token_dimensions; 
      dim++){
    if (temp_vector[dim] > high_border_condition_vector[dim]){
      //Window is completely situated on extended border
      return_value = false;
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
      CoSupport::Streams::dout << "Window is completely situated on high extended border" << std::endl;
      CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::get_req_src_data_element" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
      return return_value;
    }
  }
        
  /* Check, whether complete window is situated on the lower extended border */
  border_condition_vector_type temp2_vector = 
    temp_vector + calc_border_condition_offset(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Border condition for max window iteration: " << temp2_vector;
  CoSupport::Streams::dout << std::endl;
#endif

  for(unsigned dim = 0; 
      dim < _token_dimensions; 
      dim++){
    if (temp2_vector[dim] < low_border_condition_vector[dim]){
      //Window is completely situated on extended border
      return_value = false;
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
      CoSupport::Streams::dout << "Window is completely situated on low extended border" << std::endl;
#endif
      goto smoc_snk_md_loop_iterator_kind_get_req_src_data_element_end;
    }
  }


  /* 
     If we arrive here, at least one data element must be produced by
     the source actor.
  */
  //Get window iteration which is NOT situated on higher extended border
  for(unsigned dim = 0;
      dim < _token_dimensions;
      dim++){
    if (temp2_vector[dim] > high_border_condition_vector[dim]){
      //we assume, that coefficients in the condition matrix
      //belonging to the window iteration are 1
      //(see check_border_condition_matrix)
      assert((id_type)window_iteration[_token_dimensions-1-dim] > 
	     high_border_condition_vector[dim] - temp2_vector[dim]);
      window_iteration[_token_dimensions-1-dim] -=
	temp2_vector[dim] - high_border_condition_vector[dim];
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Window iteration not situated on the extended border:";
  CoSupport::Streams::dout << window_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

        
  get_window_data_element_offset(window_iteration, data_element_id);       
  data_element_id += base_data_element_id;

 smoc_snk_md_loop_iterator_kind_get_req_src_data_element_end:

#if (VERBOSE_LEVEL_SMOC_MD_LOOP == 103)  || (VERBOSE_LEVEL_SMOC_MD_LOOP == 102)
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::get_req_src_data_element" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_value;

}


bool smoc_snk_md_loop_iterator_kind::is_iteration_max(
						      unsigned token_dimension,
						      bool ignore_window_iteration
						      ) const {
        
  const unsigned loop_bound = 
    ignore_window_iteration ? mapping_matrix.size2() - _token_dimensions : mapping_matrix.size2();

  for(unsigned col = 0; col < loop_bound; col++){
    if (mapping_matrix(token_dimension,col) != 0){
      if (current_iteration[col] != iteration_max()[col]){
	return false;
      }
    }
  }

  return true;

}

bool smoc_snk_md_loop_iterator_kind::is_virt_iteration_max() const {
  const unsigned loop_bound = 
    mapping_matrix.size2() - _token_dimensions;
  
  for(unsigned col = 0; col < loop_bound; col++){
    if ((mapping_table[col] < 0) && 
        (current_iteration[col] != iteration_max()[col]))
      return false;
  }

  return true;
}

bool
smoc_snk_md_loop_iterator_kind::calc_eff_window_displacement(
							     unsigned token_dimension,
							     mapping_type& window_displacement
							     ) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::calc_eff_window_displacement" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "snk_iterator.iteration_vector() = " << snk_iterator.iteration_vector();
  CoSupport::Streams::dout << std::endl;
#endif

  id_type temp_window_displacement = 0;

  // Change in the border condition due to window displacement
  id_type border_condition_change = 0;

  bool finished = false;

  // Calculate the change in the data element identifiers
  // and the border condition
  // when we would propagate in the direction given by token_dimension
  for(int col = mapping_matrix.size2() - 1 - _token_dimensions;
      col >= 0;
      col--){
    if(mapping_matrix(token_dimension, col) != 0){
      //iteration level is relevant for given token dimension
      if (current_iteration[col] >= 
	  iteration_max(col, current_iteration)){
	//coordinate will become zero
	temp_window_displacement -= current_iteration[col] * 
	  mapping_matrix(token_dimension,col);
	border_condition_change -= current_iteration[col] * 
	  border_condition_matrix(token_dimension, col);
      }else{
	temp_window_displacement += mapping_matrix(token_dimension,col);
	border_condition_change += border_condition_matrix(token_dimension, col);
	finished = true;
	break;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
  CoSupport::Streams::dout << "border_condition_change = " << border_condition_change << std::endl;
  CoSupport::Streams::dout << "temp_window_displacement = " << temp_window_displacement << std::endl;
#endif


  if (!finished){
    //In the given dimension, we are at the end of a schedule period
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
    CoSupport::Streams::dout << "End of schedule period" << std::endl;
    CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::calc_eff_window_displacement" << std::endl;
    CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
    return false;
  }       
        
  /* now check, whether change in border condition has any influence */

  //in order to simplify things, we require
  assert(border_condition_matrix(token_dimension, 
				 border_condition_matrix.size2() -
				 token_dimension - 1) <= 1);
  assert(mapping_matrix(token_dimension, 
			mapping_matrix.size2() - 
			token_dimension - 1) <= 1);

  if (border_condition_change > 0){
    //Window might have left low extended border
    id_type 
      base_border_condition(base_border_condition_vector[token_dimension]);
    id_type delta_low_condition = 
      low_border_condition_vector[token_dimension] - base_border_condition;
    id_type delta_high_condition =
      base_border_condition - high_border_condition_vector[token_dimension];

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
    CoSupport::Streams::dout << "delta_low_condition = " << delta_low_condition << std::endl;
    CoSupport::Streams::dout << "delta_high_condition = " << delta_high_condition << std::endl;
#endif
                
    if (delta_low_condition > 0){
      //Previously, we have been in the extended border
      if (border_condition_change <= delta_low_condition){
	//next window will also be situated in extended border or at the
	//border of the extended border
	temp_window_displacement -= border_condition_change;
      }else{
	temp_window_displacement -= 
	  border_condition_change - delta_low_condition;
      }
    }else{
      //Nothing to do, because window has not been in low extended border
    }

    if (delta_high_condition > 0){
      // The current window is situated completely in the 
      // extended border
      temp_window_displacement -= border_condition_change;
    }else if (border_condition_change + delta_high_condition > 0){
      // the next window will be situated completely in the high
      // extended border
      temp_window_displacement -=
	border_condition_change + delta_high_condition - 1;
    }
  }
        
  if (border_condition_change < 0){
    //window might have entered extended border
    id_type 
      base_border_condition(base_border_condition_vector[token_dimension]);
    id_type delta_low_condition = 
      low_border_condition_vector[token_dimension] - base_border_condition;

    if (delta_low_condition > 0){
      //current window is already situated in extended border
      temp_window_displacement -= border_condition_change;
    }else{
      temp_window_displacement -= 
	border_condition_change - delta_low_condition;
    }
  }

  // error checking
  assert(temp_window_displacement >= 0);

  window_displacement = (mapping_type) temp_window_displacement;

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::calc_eff_window_displacement" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return true;
        
        
}


bool
smoc_snk_md_loop_iterator_kind::calc_consumed_window_iterations(
								const data_element_id_type& max_data_element_id,
								iter_domain_vector_type& consumed_window_start,
								iter_domain_vector_type& consumed_window_end
								) const {

  if (!is_virt_iteration_max())
    //No data element is read the last time.
    return false;

  for(unsigned int i = 0; i < _token_dimensions; i++){

    //in order to make things simpler
    assert((mapping_matrix(i,mapping_matrix.size2()-i-1) == 1) ||
	   // When the window extension is only one pixel
	   // then the mapping coefficient might also be zero
	   (mapping_matrix(i,mapping_matrix.size2()-i-1) == 0)
	   );

    consumed_window_start[_token_dimensions-i-1] = 
      calc_num_low_border_pixels(i);

    mapping_type eff_window_displacement;
    if (calc_eff_window_displacement(i,eff_window_displacement)){
      if (eff_window_displacement <= 0){
	return false;
      }

      consumed_window_end[_token_dimensions-i-1] = 
	consumed_window_start[_token_dimensions-i-1] + 
	eff_window_displacement - 1;

    }else{
      //End of schedule period in dimension i
	
      //Set consumed_window_end[_token_dimensions-i-1] 
      //in such a way, that all
      //resting data elements are read the last time
	
      consumed_window_end[_token_dimensions-i-1] =
	get_window_iteration(i,max_data_element_id[i]);
	
    }    
      
  }

  return true;

}


smoc_snk_md_loop_iterator_kind::id_type 
smoc_snk_md_loop_iterator_kind::calc_num_low_border_pixels(unsigned int token_dimension) const{
  id_type 
    base_border_condition(base_border_condition_vector[token_dimension]);
  id_type delta_low_condition = 
    low_border_condition_vector[token_dimension] - base_border_condition;

  if (delta_low_condition > 0){
    // window is (partly) situated on extended lower border
    return delta_low_condition;
  }else{
    //window is not situated on extended lower border
    return 0;
  }
}

smoc_snk_md_loop_iterator_kind::iter_item_type 
smoc_snk_md_loop_iterator_kind::get_window_iteration(unsigned int token_dimension, id_type coord) const{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 101
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::get_window_iteration" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;

  CoSupport::Streams::dout << "mapping_matrix = " << mapping_matrix << std::endl;
#endif


  assert(
	 (mapping_matrix(token_dimension,
			 mapping_matrix.size2() - token_dimension -1) == 1) || 
	 //if the size of the window is one, then the corresponding
	 //mapping coefficient might also be zero
	 (mapping_matrix(token_dimension,
			 mapping_matrix.size2() - token_dimension -1) == 0)
	 );
  

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 101
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::get_window_iteration" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return coord - base_data_element_id[token_dimension];
}


bool smoc_snk_md_loop_iterator_kind::check_border_condition_matrix(const border_condition_matrix_type& border_matrix) const {
  //Check properties of the matrix coefficients belonging to the window
  //iteration

  unsigned col_offset = border_matrix.size2() - _token_dimensions;

  for(unsigned  delta_col = 0;
      delta_col < _token_dimensions;
      delta_col++){
    for(unsigned row = 0; row < _token_dimensions; row++){
      if (row == _token_dimensions - delta_col - 1){
	if (border_matrix(row,delta_col + col_offset) > 1) {
	  return false;
	}
      }else{
	if (border_matrix(row,delta_col + col_offset) != 0) {
	  return false;
	}
      }
    }
  }

  //check passed.
  return true;
                        
}


#if 0
smoc_snk_md_loop_iterator_kind::id_type 
smoc_snk_md_loop_iterator_kind::calc_base_border_condition(
							   unsigned dimension
							   ) const {
  id_type return_value = 0;

  for(unsigned col = 0;
      col < current_iteration.size() - _token_dimensions;
      col++){
    return_value += 
      border_condition_matrix(dimension,col) * current_iteration[col];
  }

  return return_value;
}
#endif

smoc_snk_md_loop_iterator_kind::id_type 
smoc_snk_md_loop_iterator_kind::calc_window_border_condition(id_type base_border_condition,
							     const iter_domain_vector_type& iteration,
							     unsigned dimension) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::calc_window_border_condition" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "dimension = " << dimension << std::endl;
  CoSupport::Streams::dout << "iteration.size() = " << iteration.size() << std::endl;
  CoSupport::Streams::dout << "_token_dimensions = " << _token_dimensions << std::endl;
  CoSupport::Streams::dout << "border_condition_matrix = " << border_condition_matrix;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "iteration = " << iteration;
  CoSupport::Streams::dout << std::endl;
#endif
  for(unsigned col = iteration.size() - _token_dimensions;
      col < iteration.size();
      col++){
    base_border_condition += 
      border_condition_matrix(dimension,col) * iteration[col];
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
  CoSupport::Streams::dout << "return_value = " << base_border_condition << std::endl;
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::calc_window_border_condition" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return base_border_condition;
}

smoc_snk_md_loop_iterator_kind::border_condition_vector_type 
smoc_snk_md_loop_iterator_kind::calc_window_border_condition_vector(const border_condition_vector_type& base_border_condition_vector,
								    const iter_domain_vector_type& iteration) const {
#ifdef FAST_CALC_MODE
  border_condition_vector_type 
    return_vector(base_border_condition_vector.size(),(id_type)0);
  for(unsigned col = iteration.size()-_token_dimensions; col < iteration.size(); col++){
    const int row = mapping_table[col];
    if (row >= 0){
      return_vector[row] += 
	border_condition_matrix(row,col)*iteration[col];
    }
  }
#else
  border_condition_vector_type 
    return_vector(base_border_condition_vector.size());
        
  for(unsigned row = 0; row < _token_dimensions; row++){
    return_vector[row] = 
      calc_window_border_condition(base_border_condition_vector[row],
				   iteration,
				   row);                  
  }
#endif

  return return_vector;
}


smoc_snk_md_loop_iterator_kind::border_condition_vector_type
smoc_snk_md_loop_iterator_kind::calc_border_condition_offset(const iter_domain_vector_type& window_iteration) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::calc_border_condition_offset" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  border_condition_vector_type return_vector(window_iteration.size(),(id_type)0);

#ifdef FAST_CALC_MODE
  for(unsigned int delta_col = 0, col = border_condition_matrix.size2() - _token_dimensions;
      delta_col < _token_dimensions;
      delta_col++, col++){
    const int row = mapping_table[col];
    if(row >= 0){
      return_vector[row] += 
	border_condition_matrix(row,col) * window_iteration[delta_col];
    }
  }
#else
  for(unsigned row = 0; row < border_condition_matrix.size1(); row++){
    for(unsigned int delta_col = 0, col = border_condition_matrix.size2() - _token_dimensions;
	delta_col < _token_dimensions;
	delta_col++, col++){
      return_vector[row] += 
	border_condition_matrix(row,col) * window_iteration[delta_col];
    }
  }
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::calc_border_condition_offset" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}

smoc_snk_md_loop_iterator_kind::border_type_vector_type 
smoc_snk_md_loop_iterator_kind::is_border_pixel(const border_condition_vector_type& border_condition_vector,
						bool& is_border) const{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::is_border_pixel" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "border_condition_vector = " << border_condition_vector;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "low_border_condition_vector = " << low_border_condition_vector;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "high_border_condition_vector = " << high_border_condition_vector;
  CoSupport::Streams::dout << std::endl;
#endif

  border_type_vector_type return_vector(_token_dimensions);

  is_border = false;

  for(unsigned int i = 0; i < _token_dimensions; i++){
    if (border_condition_vector[i] < low_border_condition_vector[i]){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
      CoSupport::Streams::dout << "i = " << i << " is situated at left-border" << std::endl;
#endif
      return_vector[i] = LEFT_BORDER;
      is_border = true;
    }else if(border_condition_vector[i] > high_border_condition_vector[i]){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
      CoSupport::Streams::dout << "i = " << i << " is situated at right-border" << std::endl;
#endif
      return_vector[i] = RIGHT_BORDER;
      is_border = true;
    }else{
      return_vector[i] = NO_BORDER;
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Leavesmoc_snk_md_loop_iterator_kind::is_border_pixel" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_vector;
}


smoc_snk_md_loop_iterator_kind::border_type_vector_type 
smoc_snk_md_loop_iterator_kind::is_ext_border(const iter_domain_vector_type& window_iteration,
                                              bool& is_border) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::is_ext_border" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "current_iteration = " << current_iteration;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "window_iteration = " << window_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

  border_condition_vector_type 
    border_condition_vector(get_base_border_condition_vector());
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "base_border_condition_vector = " << border_condition_vector;
  CoSupport::Streams::dout << std::endl;
#endif
  border_condition_vector += 
    calc_border_condition_offset(window_iteration);                      

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "border_condition_vector = " << border_condition_vector;
  CoSupport::Streams::dout << std::endl;
#endif
                        
  border_type_vector_type
    return_vector(is_border_pixel(border_condition_vector, 
                                  is_border));

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::is_ext_border" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return return_vector;

}



void smoc_snk_md_loop_iterator_kind::update_base_data_element_id() {
  base_data_element_id = mapping_offset;

#ifdef FAST_CALC_MODE
  for(unsigned col = 0; 
      col < mapping_matrix.size2() - _token_dimensions; 
      col++){
    const int row = mapping_table[col];
    if (row >= 0){
      base_data_element_id[row] += 
	mapping_matrix(row,col) * current_iteration[col];
    }
  }
#else
  for(unsigned row = 0; row < mapping_matrix.size1(); row++){
    for(unsigned col = 0; 
	col < mapping_matrix.size2() - _token_dimensions; 
	col++){
      base_data_element_id[row] += 
	mapping_matrix(row,col) * current_iteration[col];
    }
  }
#endif
}

void smoc_snk_md_loop_iterator_kind::update_base_border_condition_vector(){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Enter smoc_snk_md_loop_iterator_kind::update_base_border_condition_vector";
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "current_iteration = " << current_iteration;
  CoSupport::Streams::dout << std::endl;
#endif
        
  for(unsigned row = 0; row < _token_dimensions; row++){
    base_border_condition_vector[row] = 0;
  }
  for(unsigned col = 0; col < current_iteration.size()-_token_dimensions; col++){
    const int row = mapping_table[col];
    if (row >= 0){
      base_border_condition_vector[row] += 
	border_condition_matrix(row,col)*current_iteration[col];
    }
  }
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
  CoSupport::Streams::dout << "Leave smoc_snk_md_loop_iterator_kind::update_base_border_condition_vector";
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
}



/* ******************************************************************************* */
/*                          smoc_md_static_loop_iterator_base                           */
/* ******************************************************************************* */


smoc_md_static_loop_iterator_base::smoc_md_static_loop_iterator_base(const iter_domain_vector_type& iteration_max,
                                                                     unsigned int token_dimensions)
  : _iteration_max(iteration_max),
    _max_window_iteration(calc_max_window_iteration(token_dimensions,iteration_max)),
    num_invocations_per_period(calc_schedule_period_invocations())
{}

smoc_md_static_loop_iterator_base::smoc_md_static_loop_iterator_base(const smoc_md_static_loop_iterator_base& src_iterator)
  : _iteration_max(src_iterator._iteration_max),
    _max_window_iteration(src_iterator._max_window_iteration),
    num_invocations_per_period(src_iterator.num_invocations_per_period)
{}


const smoc_md_static_loop_iterator_base::iter_domain_vector_type&
smoc_md_static_loop_iterator_base::max_window_iteration() const{
  return _max_window_iteration;
}

const smoc_md_static_loop_iterator_base::iter_domain_vector_type 
smoc_md_static_loop_iterator_base::calc_max_window_iteration(unsigned int token_dimensions,
                                                             const iter_domain_vector_type& iteration_max) {
  iter_domain_vector_type return_vector(token_dimensions);
  for(unsigned int i = 0, j = iteration_max.size() - token_dimensions; 
      i < token_dimensions; 
      i++, j++){
    return_vector[i] = iteration_max[j];
  }

  return return_vector;
}

long 
smoc_md_static_loop_iterator_base::calc_iteration_id(const iter_domain_vector_type& iter,
                                                     long schedule_period) const {
  long factor = 1;
  long return_value = 0;

  // As the loop boundaries are static, calculation
  // of the iteration ID is quite simple.
  for(int i = iter.size()-1;
      i >= 0;
      i--){
    return_value += factor*iter[i];
    factor *= _iteration_max[i]+1;
  }

  return_value += 
    schedule_period * num_invocations_per_period;

  return return_value;
}

long 
smoc_md_static_loop_iterator_base::calc_schedule_period_invocations() const {
  long return_value = 1;
  for(unsigned int i = 0; i < _iteration_max.size(); i++){
    return_value *= _iteration_max[i]+1;
  }
  return return_value;
}

/* ******************************************************************************* */
/*                             smoc_md_static_loop_iterator                        */
/* ******************************************************************************* */

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(const iter_domain_vector_type& iteration_max,
                                                           unsigned int token_dimensions)
  : smoc_md_loop_iterator_kind(token_dimensions,
                               iter_domain_vector_type(iteration_max.size(),
                                                       (smoc_md_loop_iterator_kind::data_type)0)),
    smoc_md_static_loop_iterator_base(iteration_max, 
                                      token_dimensions)
{
}

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator)
  : smoc_md_loop_iterator_kind(src_iterator),
    smoc_md_static_loop_iterator_base(src_iterator)
{
}


bool smoc_md_static_loop_iterator::inc(){

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Enter smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  //default initialization
  _new_schedule_period = true;
  
  for(int i = current_iteration.size() - _token_dimensions - 1;
      i >= 0;
      i--){
    current_iteration[i]++;
    if (current_iteration[i] > _iteration_max[i]){
      current_iteration[i] = 0;
    }else{
      _new_schedule_period = false;
      break;
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "New loop iteration: " << current_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Leave smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return _new_schedule_period;
}



/* ******************************************************************************* */
/*                           smoc_src_md_static_loop_iterator                      */
/* ******************************************************************************* */

smoc_src_md_static_loop_iterator::smoc_src_md_static_loop_iterator(
								   const iter_domain_vector_type& iteration_max,
								   const mapping_matrix_type& mapping_matrix,
								   const mapping_offset_type& mapping_offset
								   )
  : smoc_src_md_loop_iterator_kind(iter_domain_vector_type(iteration_max.size(),(smoc_md_loop_iterator_kind::data_type)0),
				   mapping_matrix,
				   mapping_offset                                                                                                                          ,
				   calc_max_data_element_id(iteration_max,mapping_matrix)
				   ),
    smoc_md_static_loop_iterator_base(iteration_max, mapping_offset.size())
{
}

smoc_src_md_static_loop_iterator::smoc_src_md_static_loop_iterator(const smoc_src_md_static_loop_iterator& src_iterator)
  : smoc_src_md_loop_iterator_kind(src_iterator),
    smoc_md_static_loop_iterator_base(src_iterator)
{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Enter smoc_src_md_static_loop_iterator::smoc_src_md_static_loop_iterator" << std::endl;
  CoSupport::Streams::dout << "Leave smoc_src_md_static_loop_iterator::smoc_src_md_static_loop_iterator" << std::endl;
#endif
}

smoc_src_md_static_loop_iterator::data_element_id_type 
smoc_src_md_static_loop_iterator::calc_max_data_element_id(const iter_domain_vector_type& iteration_max,
							   const mapping_matrix_type& mapping_matrix) const {

  //Attention: when this function is called, we cannot already access the mapping table
  //Hence, we use an ordinary multiplication.
  return prod(mapping_matrix,iteration_max);
}


bool smoc_src_md_static_loop_iterator::inc(){

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Enter smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  //default initialization
  _new_schedule_period = true;
  
  for(int i = current_iteration.size() -  _token_dimensions - 1;
      i >= 0;
      i--){
#ifdef FAST_CALC_MODE2
    iter_item_type old_value = current_iteration[i];
#endif
    current_iteration[i]++;

    if (current_iteration[i] > _iteration_max[i]){
      current_iteration[i] = 0;
#ifdef FAST_CALC_MODE2
      const int row = mapping_table[i];
      if (row >= 0){
	//update base data element
	base_data_element_id[row] -= old_value * mapping_matrix(row,i);
      }
#endif
    }else{
      _new_schedule_period = false;
#ifdef FAST_CALC_MODE2
      const int row = mapping_table[i];
      if (row >= 0){
	//update base data element
	base_data_element_id[row] += mapping_matrix(row,i);
      }
#endif

      break;
    }
  }

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "New loop iteration: " << current_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

#ifndef FAST_CALC_MODE2
  parent_type::inc();
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Leave smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return _new_schedule_period;
}




/* ******************************************************************************* */
/*                           smoc_snk_md_static_loop_iterator                      */
/* ******************************************************************************* */

smoc_snk_md_static_loop_iterator::smoc_snk_md_static_loop_iterator(
								   const iter_domain_vector_type& iteration_max,
								   const mapping_matrix_type& mapping_matrix,
								   const mapping_offset_type& mapping_offset,
								   const border_condition_matrix_type& border_matrix,
								   const border_condition_vector_type& low_border_vector,
								   const border_condition_vector_type& high_border_vector
								   )
  : smoc_snk_md_loop_iterator_kind(iter_domain_vector_type(iteration_max.size(),(smoc_md_loop_iterator_kind::data_type)0),
				   mapping_matrix,
				   mapping_offset,
				   border_matrix,
				   low_border_vector,
				   high_border_vector
				   ),
    smoc_md_static_loop_iterator_base(iteration_max, mapping_offset.size())
{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
  CoSupport::Streams::dout << "Enter smoc_snk_md_static_loop_iterator::smoc_snk_md_static_loop_iterator" << std::endl;
  CoSupport::Streams::dout << "Leave smoc_snk_md_static_loop_iterator::smoc_snk_md_static_loop_iterator" << std::endl;
#endif
}

smoc_snk_md_static_loop_iterator::smoc_snk_md_static_loop_iterator(const smoc_snk_md_static_loop_iterator& snk_iterator)
  : smoc_snk_md_loop_iterator_kind(snk_iterator),
    smoc_md_static_loop_iterator_base(snk_iterator)
{
}

bool smoc_snk_md_static_loop_iterator::inc(){

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Enter smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  //default initialization
  _new_schedule_period = true;


  for(int i = current_iteration.size() -  _token_dimensions - 1;
      i >= 0;
      i--){
#ifdef FAST_CALC_MODE2
    iter_item_type old_value = current_iteration[i];
#endif
    current_iteration[i]++;

    if (current_iteration[i] > _iteration_max[i]){
      current_iteration[i] = 0;
#ifdef FAST_CALC_MODE2
      const int row = mapping_table[i];
      if (row >= 0){
	//update base data element
	base_data_element_id[row] -= old_value * mapping_matrix(row,i);
	//update base border condition
	base_border_condition_vector[row] -= 
	  old_value * border_condition_matrix(row,i);

      }
#endif
    }else{
      _new_schedule_period = false;
#ifdef FAST_CALC_MODE2
      const int row = mapping_table[i];
      if (row >= 0){
	//update base data element
	base_data_element_id[row] += mapping_matrix(row,i);
	//update base border condition
	base_border_condition_vector[row] += 
	  border_condition_matrix(row,i);

      }
#endif

      break;
    }
  }


#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "New loop iteration: " << current_iteration;
  CoSupport::Streams::dout << std::endl;
#endif

#ifndef FAST_CALC_MODE2
  parent_type::inc();
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
  CoSupport::Streams::dout << "Leave smoc_md_static_loop_iterator::inc()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

  return _new_schedule_period;
}

