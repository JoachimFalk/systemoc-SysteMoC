
#include <smoc_md_loop.hpp>

/* ******************************************************************************* */
/*                              smoc_md_loop_iterator_kind                         */
/* ******************************************************************************* */


/* ******************************************************************************* */
/*                           smoc_md_static_loop_iterator                          */
/* ******************************************************************************* */

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator::iter_domain_vector_type& min,
		      const smoc_md_static_loop_iterator::iter_domain_vector_type& max,
		      const smoc_md_static_loop_iterator::size_type window_dimensions
		      )
  : smoc_md_loop_iterator_kind(window_dimensions, min),
		_iteration_min(min),
    _iteration_max(max)
{
}

smoc_md_static_loop_iterator::smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator)
  : smoc_md_loop_iterator_kind(src_iterator),
		_iteration_min(src_iterator._iteration_min),
    _iteration_max(src_iterator._iteration_max)
{
}

smoc_md_static_loop_iterator& smoc_md_static_loop_iterator::operator++(){
  
  for(unsigned int i = current_iteration.size() -  _window_dimensions - 1;
			i >= 0;
			i--){
    current_iteration[i]++;

    if (current_iteration[i] > _iteration_max[i]){
      current_iteration[i] = _iteration_min[i];
    }else{
      return (*this);
    }
  }

  //toggle iteration_flag
  _iteration_flag = ~ _iteration_flag;

  return (*this);
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
	
const smoc_md_static_loop_iterator::iter_domain_vector_type
smoc_md_static_loop_iterator::min_window_iteration() const{
  iter_domain_vector_type return_vector(current_iteration);

  //replace coordinates corresponding to iteration in the inner
  //of the window by its minimum values.
  for(unsigned int i = return_vector.size() - _window_dimensions;
      i < return_vector.size();
      i++){
    return_vector[i] = _iteration_min[i];
  }

  return return_vector;
}


/* ******************************************************************************* */
/*                        smoc_md_loop_data_element_mapper                         */
/* ******************************************************************************* */

smoc_md_loop_data_element_mapper::data_element_id_type
smoc_md_loop_data_element_mapper::get_data_element_id(const smoc_md_loop_data_element_mapper::iter_domain_vector_type& iteration_vector) const{
	data_element_id_type return_vector(mapping_offset_vector);

	//current token dimension
	unsigned int cur_dim = 0;

	//error checking
	assert(iteration_vector.size() >= mapping_weights.size());

	for(unsigned i = 0; i < mapping_weights.size(); i++){		
		return_vector[cur_dim] += mapping_weights[i]*iteration_vector[i];
		cur_dim++;
		if (cur_dim >= token_dimensions)
			cur_dim = 0;
	}

	return iteration_vector;
	
}

smoc_md_loop_data_element_mapper::data_element_id_type
smoc_md_loop_data_element_mapper::get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,			
																											const iter_domain_vector_type& window_iteration
																											) const{
	data_element_id_type return_vector(mapping_offset_vector);

	//current token dimensions
	unsigned int cur_dim = 0;

	//error checking
	assert(window_iteration.size() == loop_iterator.window_dimensions());

	for(unsigned i = 0; 
			i < mapping_weights.size() - window_iteration.size();
			i++){
		return_vector[cur_dim] += mapping_weights[i]*loop_iterator[i];
		cur_dim++;
		if (cur_dim >= token_dimensions)
			cur_dim = 0;
	}

	for(unsigned i = mapping_weights.size() - window_iteration.size();
			i < mapping_weights.size();
			i++){
		return_vector[cur_dim] += mapping_weights[i]*window_iteration[i];
		cur_dim++;
		if (cur_dim >= token_dimensions)
			cur_dim = 0;
	}

	return return_vector;
}


/* ******************************************************************************* */
/*                     smoc_md_loop_src_data_element_mapper                        */
/* ******************************************************************************* */

bool
smoc_md_loop_src_data_element_mapper::get_src_loop_vector(const data_element_id_type& src_data_el_id,
																													const smoc_md_loop_iterator_kind& loop_iterator,
																													iter_domain_vector_type& iteration_vector,
																													bool& prev_flag
																													) const {

	//subtract offset (initial data elements)
	data_element_id_type temp_id(src_data_el_id - mapping_offset_vector);	

	//default assignments
	prev_flag = false;

	//currently processed token dimension
	unsigned cur_token_dimension = 0;
	
	//The following formula assumes, that the minimum for each component of
	//the iteration vector is constant and amounts zero!
	for(unsigned i = 0; i < mapping_weights.size(); i++){
		//error checking
		assert(mapping_weights[i] > 0);
		assert(loop_iterator.iteration_min()[i] == 0);

		if (temp_id[cur_token_dimension] < 0){
			if (i == 0){
				//Data element has been produced during previous schedule period.
				prev_flag = true;
				temp_id[cur_token_dimension] += mapping_weights[i];

				assert(temp_id[cur_token_dimension] >= 0);
				//If temp_id[cur_token_dimension] is still smaller then zero, then we have to specify a larger
				//iteration period.
			}else{
				//Data element has not been produced by source actor.
				return false;
			}
		}

		iteration_vector[i] = temp_id[cur_token_dimension] / mapping_weights[i];
		temp_id[cur_token_dimension] -= iteration_vector[i] * mapping_weights[i];

		cur_token_dimension++;
		if (cur_token_dimension >= token_dimensions)
			cur_token_dimension = 0;

		//error checking
		smoc_md_loop_iterator_kind::data_type iter_max = 
			loop_iterator.iteration_max(i,iteration_vector);
		smoc_md_loop_iterator_kind::data_type iter_min = 
			loop_iterator.iteration_min()[i];

		assert(iteration_vector[i] <= iter_max);
		assert(iteration_vector[i] >= iter_min);
		
	}

	return true;
	
}


/* ******************************************************************************* */
/*                     smoc_md_loop_snk_data_element_mapper                        */
/* ******************************************************************************* */

bool
smoc_md_loop_snk_data_element_mapper::get_src_data_element_id(const smoc_md_loop_iterator_kind& snk_loop_iterator,
																															const iter_domain_vector_type& window_iteration,
																															data_element_id_type& src_data_el_id) const {
	bool return_value = true;

	src_data_el_id = get_data_element_id(snk_loop_iterator,window_iteration);

	for(unsigned i = 0; i < token_dimensions; i++){
		if (src_data_el_id[i] < min_data_el_id[i]){
			// border data element
			return_value = false;
			src_data_el_id[i] = min_data_el_id[i];
		}else if (src_data_el_id[i] > max_data_el_id[i]){
			// border data element
			return_value = false;
			src_data_el_id[i] = max_data_el_id[i];
		}
	}

	return return_value;
}

