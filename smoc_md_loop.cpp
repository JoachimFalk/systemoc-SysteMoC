
#include <smoc_md_loop.hpp>

/* ******************************************************************************* */
/*                              smoc_md_loop_iterator_kind                         */
/* ******************************************************************************* */


/* ******************************************************************************* */
/*                           smoc_md_static_loop_iterator                          */
/* ******************************************************************************* */

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(
																													 const iter_domain_vector_type& max,
																													 const size_type window_dimensions
																													 )
  : smoc_md_loop_iterator_kind(window_dimensions, 
															 iter_domain_vector_type(max.size(),(smoc_md_loop_iterator_kind::data_type)0)),
    _iteration_max(max)
{
}

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator)
  : smoc_md_loop_iterator_kind(src_iterator),
    _iteration_max(src_iterator._iteration_max)
{
}

bool smoc_md_static_loop_iterator::inc(){
  
  for(int i = current_iteration.size() -  _window_dimensions - 1;
			i >= 0;
			i--){
    current_iteration[i]++;

    if (current_iteration[i] > _iteration_max[i]){
      current_iteration[i] = 0;
    }else{
      return false;
    }
  }

	//new schedule period
  return true;
}

const smoc_md_static_loop_iterator::iter_domain_vector_type
smoc_md_static_loop_iterator::max_window_iteration() const{
  iter_domain_vector_type return_vector(current_iteration);

  //replace coordinates corresponding to iteration in the inner
  //of the window by its maximum values.
  for(unsigned int i = return_vector.size() - _window_dimensions;
      i < return_vector.size();
      i++){
    return_vector[i] = _iteration_max[i];
  }

  return return_vector;
}

/* ******************************************************************************* */
/*                     smoc_md_loop_src_data_element_mapper                        */
/* ******************************************************************************* */
void smoc_md_loop_src_data_element_mapper::get_data_element_id(const iter_domain_vector_type& iteration_vector,
																															 data_element_id_type& data_element_id,
																															 id_type& schedule_period_offset
																															 ) const {
	data_element_id = mapping_offset;
	data_element_id += prod(mapping_matrix, iteration_vector);

	//Calculate schedule period offset
	schedule_period_offset = 
		data_element_id[token_dimensions-1] / max_data_element_id[token_dimensions-1];

	data_element_id[token_dimensions-1] -= 
		schedule_period_offset *  max_data_element_id[token_dimensions-1];	 
	
}

void smoc_md_loop_src_data_element_mapper::get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
																															 const iter_domain_vector_type& window_iteration,
																															 data_element_id_type& data_element_id,
																															 id_type& schedule_period_offset
																															 ) const {

	//init return-vector
	data_element_id = mapping_offset;
	
	for(unsigned i = 0; 
			i < loop_iterator.iterator_depth() - loop_iterator.window_dimensions();
			i++){
		for(unsigned j = 0; j < mapping_matrix.size1(); j++){
			data_element_id[j] += mapping_matrix(j,i) * loop_iterator[i];
		}
	}

	for(unsigned i = loop_iterator.iterator_depth() - loop_iterator.window_dimensions();
			i < loop_iterator.iterator_depth();
			i++){
		for(unsigned j = 0; j < mapping_matrix.size1(); j++){
			data_element_id[j] += mapping_matrix(j,i) * 
				window_iteration[i - (loop_iterator.iterator_depth() - loop_iterator.window_dimensions())];
		}
	}


	//Calculate schedule period offset
	schedule_period_offset = 
		data_element_id[token_dimensions-1] / max_data_element_id[token_dimensions-1];

	data_element_id[token_dimensions-1] -= 
		schedule_period_offset *  max_data_element_id[token_dimensions-1];


}


bool
smoc_md_loop_src_data_element_mapper::get_src_loop_vector(const data_element_id_type& src_data_el_id,
																													const smoc_md_loop_iterator_kind& loop_iterator,
																													iter_domain_vector_type& iteration_vector,
																													id_type& schedule_period_offset
																													) const {
	//subtract offset (initial data elements)
	data_element_id_type temp_id(src_data_el_id - mapping_offset);	

	//Check, that data elements are still situated in token space
	for(unsigned int i = 0; i < token_dimensions-1; i++){
		if (temp_id[i] < 0){
			//data element is not produced by source actor
			return false;
		}
	}

	//Calculate schedule period offset
	if (temp_id[token_dimensions-1] < 0){
		// (divide and round up)
		schedule_period_offset = 
			(-temp_id[token_dimensions-1] + max_data_element_id[token_dimensions-1] - 1) / 
			max_data_element_id[token_dimensions-1];
		temp_id[token_dimensions-1] += schedule_period_offset * 
			max_data_element_id[token_dimensions-1];
	}else{
		schedule_period_offset = 0;
	}	

	for(unsigned i = 0; i < mapping_table.size(); i++){
		iteration_vector[i] = 
			temp_id[mapping_table[i]] / mapping_matrix(mapping_table[i],i);
		temp_id[mapping_table[i]] -= 
			iteration_vector[i] * mapping_matrix(mapping_table[i],i);
	}

	return true;
	
}

bool 
smoc_md_loop_src_data_element_mapper::check_matrix(const mapping_matrix_type& mapping_matrix) const {
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

smoc_vector<int> 
smoc_md_loop_src_data_element_mapper::calc_mapping_table(const mapping_matrix_type& mapping_matrix) const {
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

	return return_vector;
}


/* ******************************************************************************* */
/*                     smoc_md_loop_snk_data_element_mapper                        */
/* ******************************************************************************* */

void smoc_md_loop_snk_data_element_mapper::get_data_element_id(const iter_domain_vector_type& iteration_vector,
																															 data_element_id_type& data_element_id
																															 ) const {
	data_element_id = mapping_offset;
	data_element_id += prod(mapping_matrix, iteration_vector);

}

void smoc_md_loop_snk_data_element_mapper::get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
																															 const iter_domain_vector_type& window_iteration,
																															 data_element_id_type& data_element_id
																															 ) const {

	//init return-vector
	data_element_id = mapping_offset;
	
	for(unsigned i = 0; 
			i < loop_iterator.iterator_depth() - loop_iterator.window_dimensions();
			i++){
		for(unsigned j = 0; j < mapping_matrix.size1(); j++){
			data_element_id[j] += mapping_matrix(j,i) * loop_iterator[i];
		}
	}

	for(unsigned i = loop_iterator.iterator_depth() - loop_iterator.window_dimensions();
			i < loop_iterator.iterator_depth();
			i++){
		for(unsigned j = 0; j < mapping_matrix.size1(); j++){
			data_element_id[j] += mapping_matrix(j,i) * 
				window_iteration[i - (loop_iterator.iterator_depth() - loop_iterator.window_dimensions())];
		}
	}
}


bool smoc_md_loop_snk_data_element_mapper::get_req_src_data_element(const smoc_md_loop_iterator_kind& snk_iterator,
																																		data_element_id_type& data_element_id) const {

	// Get the maximum iteration vector for the given window position
	smoc_md_loop_iterator_kind::iter_domain_vector_type 
		window_iteration(snk_iterator.max_window_iteration());

	border_condition_vector_type temp_vector(token_dimensions);

	//init temp_vector
	for(unsigned i = 0; i < token_dimensions; i++){
		temp_vector[i] = 0;
	}

	for(unsigned col = 0; 
			col < window_iteration.size() - token_dimensions;
			col++){
		for(unsigned row = 0; row < token_dimensions; row++){
			temp_vector[row] += 
				border_condition_matrix(row,col) * window_iteration[col];
		}		
	}

	
	/* Check, whether complete window is situated on the higher extended border */
	for(unsigned dim = 0; 
			dim < token_dimensions; 
			dim++){
		if (temp_vector[dim] > high_border_condition_vector[dim]){
			//Window is completely situated on extended border
			return false;
		}
	}

	
	/* Check, whether complete window is situated on the lower extended border */
	border_condition_vector_type temp2_vector(temp_vector);
	for(unsigned col = window_iteration.size() - token_dimensions;
			col < window_iteration.size();
			col++){
		for(unsigned row = 0; row < token_dimensions; row++){
			temp2_vector[row] += 
				border_condition_matrix(row,col) * window_iteration[col];
		}		
	}
	for(unsigned dim = 0; 
			dim < token_dimensions; 
			dim++){
		if (temp2_vector[dim] < low_border_condition_vector[dim]){
			//Window is completely situated on extended border
			return false;
		}
	}


	/* 
		 If we arrive here, at least one data element must be produced by
		 the source actor.
	 */
	//Get window iteration which is NOT situated on higher extended border
	for(unsigned dim = 0;
			dim < token_dimensions;
			dim++){
		if (temp2_vector[dim] > high_border_condition_vector[dim]){
			//we assume, that coefficients in the condition matrix
			//belonging to the window iteration are 1
			//(see check_border_condition_matrix)
			assert((id_type)window_iteration[snk_iterator.iterator_depth()-1-dim] > high_border_condition_vector[dim] - temp2_vector[dim]);
			window_iteration[snk_iterator.iterator_depth()-1-dim] -=
				high_border_condition_vector[dim] - temp2_vector[dim];
		}
	}

	
	//The following operation can be done more efficiently, 
	//but as it is easy:
	get_data_element_id(window_iteration,	data_element_id);	

	return true;

}


bool smoc_md_loop_snk_data_element_mapper::check_border_condition_matrix(const border_condition_matrix_type& border_matrix) const {
	//Check properties of the matrix coefficients belonging to the window
	//iteration

	unsigned col_offset = border_matrix.size2() - token_dimensions;

	for(unsigned  delta_col = 0;
			delta_col < token_dimensions;
			delta_col++){
		for(unsigned row = 0; row < token_dimensions; row++){
			if (row == delta_col){
				if (border_matrix(row,delta_col + col_offset) != 1) {
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
