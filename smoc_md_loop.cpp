
#include <smoc_md_loop.hpp>
#include <smoc_debug_out.hpp>

#define FAST_CALC_MODE

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

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
	dout << "Enter smoc_md_static_loop_iterator::inc()" << endl;
	dout << inc_level;
#endif

	//default initialization
	_new_schedule_period = true;
  
  for(int i = current_iteration.size() -  _window_dimensions - 1;
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
	dout << "New loop iteration: " << current_iteration;
	dout << endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
	dout << "Leave smoc_md_static_loop_iterator::inc()" << endl;
	dout << dec_level;
#endif

  return _new_schedule_period;
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
/*                     smoc_md_loop_data_element_mapper                        */
/* ******************************************************************************* */

smoc_vector<int> 
smoc_md_loop_data_element_mapper::calc_mapping_table(const mapping_matrix_type& mapping_matrix) const {

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
	dout << "Enter smoc_md_loop_src_data_element_mapper::calc_mapping_table" << endl;
	dout << inc_level;
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
	dout << "Mapping table: " << return_vector;
	dout << endl;
	dout << "Leave smoc_md_loop_src_data_element_mapper::calc_mapping_table" << endl;
	dout << dec_level;
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
/*                     smoc_md_loop_src_data_element_mapper                        */
/* ******************************************************************************* */
const smoc_md_loop_src_data_element_mapper::data_element_id_type& 
smoc_md_loop_src_data_element_mapper::max_data_element_id() const {
	return _max_data_element_id;
}

void smoc_md_loop_src_data_element_mapper::get_data_element_id(const iter_domain_vector_type& iteration_vector,
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

	//Calculate schedule period offset
	schedule_period_offset = 
		data_element_id[ _token_dimensions - 1] / (_max_data_element_id[_token_dimensions-1]+1);

	data_element_id[_token_dimensions-1] -= 
		schedule_period_offset *  (_max_data_element_id[_token_dimensions-1]+1);
	
}

void smoc_md_loop_src_data_element_mapper::get_base_data_element_id(const iter_domain_vector_type& iteration_vector,
																																		data_element_id_type& data_element_id
																																		) const {
	data_element_id = mapping_offset;

#ifdef FAST_CALC_MODE
	for(unsigned int col = 0; 
			col < mapping_matrix.size2() - _token_dimensions; 
			col++){
		const int row = mapping_table[col];
		if (row >= 0){
			data_element_id[row] += 
				mapping_matrix(row,col) * iteration_vector[col];
		}
	}
#else
	for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
		for(unsigned int col = 0; 
				col < mapping_matrix.size2() - _token_dimensions; 
				col++){
			data_element_id[row] += 
				mapping_matrix(row,col) * iteration_vector[col];
		}
	}	
#endif
}

void smoc_md_loop_src_data_element_mapper::get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
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


void smoc_md_loop_src_data_element_mapper::max_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
																															 data_element_id_type& max_data_element_id,
																															 id_type& schedule_period_offset) const{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
	dout << "Enter smoc_md_loop_src_data_element_mapper::max_data_element_id" << endl;
	dout << inc_level;
#endif

	//get the data element with the larges coordinates	
	const iter_domain_vector_type 
		max_window_iteration(loop_iterator.max_window_iteration());

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
	dout << "max_window_iteration = " << max_window_iteration;
	dout << endl;
#endif

	get_data_element_id(max_window_iteration,
											max_data_element_id,
											schedule_period_offset
											);

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 105
	dout << "Leave smoc_md_loop_src_data_element_mapper::max_data_element_id" << endl;
	dout << dec_level;
#endif
}

const smoc_md_loop_src_data_element_mapper::data_element_id_type 
smoc_md_loop_src_data_element_mapper::size_token_space() const {
	data_element_id_type return_vector(_max_data_element_id);

	for(unsigned i = 0; i < return_vector.size(); i++){
		return_vector[i]++;
	}

	return return_vector;
}


bool
smoc_md_loop_src_data_element_mapper::get_src_loop_iteration(const data_element_id_type& src_data_el_id,
																														 iter_domain_vector_type& iteration_vector,
																														 id_type& schedule_period_offset
																														 ) const {

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
	dout << "Enter smoc_md_loop_src_data_element_mapper::get_src_loop_iteration" << endl;
	dout << inc_level;

	dout << "Source data element ID: " << src_data_el_id;
	dout << endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
	dout << "iteration_vector.size() = " << iteration_vector.size() << endl;
#endif



	bool return_value = true;

	//subtract offset (initial data elements)
	data_element_id_type temp_id(src_data_el_id - mapping_offset);	

	//Check, that data elements are still situated in token space
	for(unsigned int i = 0; i < _token_dimensions-1; i++){
		if (temp_id[i] < 0){
			//data element is not produced by source actor
#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
			dout << "Data element not produced by source actor" << endl;
#endif
			return_value = false;
			//goto smoc_md_loop_src_data_element_mapper_get_src_loop_iteration_end;
#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
			dout << "Leave smoc_md_loop_src_data_element_mapper::get_src_loop_iteration" << endl;
			dout << dec_level;
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
	dout << "schedule_period_offset = " << schedule_period_offset << endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
	dout << "Calculate iteration vector ..." << endl;
#endif
	for(unsigned i = 0; i < mapping_table.size(); i++){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
		dout << "i = " << i
				 << " mapping_table[i] = " << mapping_table[i] << endl;
		dout << "iteration_vector.size() = " << iteration_vector.size() << endl;
#endif
		const int row = mapping_table[i];
		if (row >= 0) {
			iteration_vector[i] = 
				temp_id[row] / mapping_matrix(row,i);
			temp_id[row] -= 
				iteration_vector[i] * mapping_matrix(row,i);
		}
	}


	//smoc_md_loop_src_data_element_mapper_get_src_loop_iteration_end:

#if VERBOSE_LEVEL_SMOC_MD_LOOP >= 101
	dout << "Leave smoc_md_loop_src_data_element_mapper::get_src_loop_iteration" << endl;
	dout << dec_level;
#endif

	return return_value;
	
}

/* ******************************************************************************* */
/*                     smoc_md_loop_snk_data_element_mapper                        */
/* ******************************************************************************* */

void smoc_md_loop_snk_data_element_mapper::get_data_element_id(const iter_domain_vector_type& iteration_vector,
																															 data_element_id_type& data_element_id
																															 ) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Enter smoc_md_loop_snk_data_element_mapper::get_data_element_id";
	dout << endl;
	dout << inc_level;
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
	dout << "Leave smoc_md_loop_snk_data_element_mapper::get_data_element_id";
	dout << endl;
	dout << dec_level;
#endif

}

void smoc_md_loop_snk_data_element_mapper::get_base_data_element_id(const iter_domain_vector_type& iteration_vector,
																																		data_element_id_type& data_element_id
																																		) const {
	data_element_id = mapping_offset;

#ifdef FAST_CALC_MODE
	for(unsigned col = 0; 
			col < mapping_matrix.size2() - _token_dimensions; 
			col++){
		const int row = mapping_table[col];
		if (row >= 0){
			data_element_id[row] += 
				mapping_matrix(row,col) * iteration_vector[col];
		}
	}
#else
	for(unsigned row = 0; row < mapping_matrix.size1(); row++){
		for(unsigned col = 0; 
				col < mapping_matrix.size2() - _token_dimensions; 
				col++){
			data_element_id[row] += 
				mapping_matrix(row,col) * iteration_vector[col];
		}
	}
#endif
}

void smoc_md_loop_snk_data_element_mapper::get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
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



bool smoc_md_loop_snk_data_element_mapper::get_req_src_data_element(const smoc_md_loop_iterator_kind& snk_iterator,
																																		data_element_id_type& data_element_id) const {
#if (VERBOSE_LEVEL_SMOC_MD_LOOP == 103) || (VERBOSE_LEVEL_SMOC_MD_LOOP == 102)
	dout << "Enter smoc_md_loop_snk_data_element_mapper::get_req_src_data_element" << endl;
	dout << inc_level;
	dout << "high_border_condition_vector = " << high_border_condition_vector;
	dout << endl;
	dout << "low_border_condition_vector = " << low_border_condition_vector;
	dout << endl;
#endif
	bool return_value = true;

	// Get the maximum iteration vector for the given window position
	iter_domain_vector_type 
		window_iteration(snk_iterator.max_window_iteration());
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Max window iteration: " << window_iteration;
	dout << endl;
#endif


	border_condition_vector_type 
		temp_vector(calc_base_border_condition_vector(window_iteration));

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Base border condition: " << temp_vector;
	dout << endl;
#endif
	
	/* Check, whether complete window is situated on the higher extended border */
	for(unsigned dim = 0; 
			dim < _token_dimensions; 
			dim++){
		if (temp_vector[dim] > high_border_condition_vector[dim]){
			//Window is completely situated on extended border
			return_value = false;
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
			dout << "Window is completely situated on high extended border" << endl;
			dout << "Leave smoc_md_loop_snk_data_element_mapper::get_req_src_data_element" << endl;
			dout << dec_level;
#endif
			return return_value;
		}
	}
	
	/* Check, whether complete window is situated on the lower extended border */
	border_condition_vector_type 
	temp2_vector(calc_window_border_condition_vector(temp_vector,
																									 window_iteration));
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Border condition for max window iteration: " << temp2_vector;
	dout << endl;
#endif

	for(unsigned dim = 0; 
			dim < _token_dimensions; 
			dim++){
		if (temp2_vector[dim] < low_border_condition_vector[dim]){
			//Window is completely situated on extended border
			return_value = false;
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
			dout << "Window is completely situated on low extended border" << endl;
#endif
			goto smoc_md_loop_snk_data_element_mapper_get_req_src_data_element_end;
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
			assert((id_type)window_iteration[snk_iterator.iterator_depth()-1-dim] > high_border_condition_vector[dim] - temp2_vector[dim]);
			window_iteration[snk_iterator.iterator_depth()-1-dim] -=
				temp2_vector[dim] - high_border_condition_vector[dim];
		}
	}

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Window iteration not situated on the extended border:";
	dout << window_iteration;
	dout << endl;
#endif

	
	//The following operation can be done more efficiently, 
	//but as it is easy:
	get_data_element_id(window_iteration,	data_element_id);	

smoc_md_loop_snk_data_element_mapper_get_req_src_data_element_end:

#if (VERBOSE_LEVEL_SMOC_MD_LOOP == 103)  || (VERBOSE_LEVEL_SMOC_MD_LOOP == 102)
	dout << "Leave smoc_md_loop_snk_data_element_mapper::get_req_src_data_element" << endl;
	dout << dec_level;
#endif

	return return_value;

}


bool smoc_md_loop_snk_data_element_mapper::is_iteration_max(const smoc_md_loop_iterator_kind& snk_iterator,
																														unsigned token_dimension,
																														bool ignore_window_iteration) const {

	unsigned loop_bound = 
		ignore_window_iteration ? mapping_matrix.size2() - _token_dimensions : mapping_matrix.size2();

	for(unsigned col = 0; col < loop_bound; col++){
		if (mapping_matrix(token_dimension,col) != 0){
			if (snk_iterator[col] != snk_iterator.iteration_max()[col]){
				return false;
			}
		}
	}

	return true;

}

#if 0
smoc_md_loop_snk_data_element_mapper::mapping_type 
smoc_md_loop_snk_data_element_mapper::calc_num_low_border_data_elements(const smoc_md_loop_iterator_kind& snk_iterator,
																																				unsigned token_dimension) const {
	id_type temp = 0;
	mapping_type return_value;

	for(unsigned col = 0; 
			col < border_condition_matrix.size2() - _token_dimensions;
			col++){
		temp += 
			border_condition_matrix(token_dimension,col) * snk_iterator[col];
	}		
	

	if (temp >= (id_type)low_border_condition_vector[token_dimension]){
		return_value = 0;
	}else{
		return_value = 
			(mapping_type)((id_type)low_border_condition_vector[token_dimension] - temp);

		mapping_type window_size = 			
			snk_iterator.iteration_max(mapping_matrix.size2() - _token_dimensions + token_dimension,
																 snk_iterator.iteration_vector());
		assert(mapping_matrix(token_dimension, mapping_matrix.size2() - _token_dimensions + token_dimension) == 1);	

		if (return_value > window_size)
			return_value = window_size;
	}

	return return_value;	
}

smoc_md_loop_snk_data_element_mapper::mapping_type 
smoc_md_loop_snk_data_element_mapper::calc_num_high_border_data_elements(const smoc_md_loop_iterator_kind& snk_iterator,
																																				 unsigned token_dimension) const {
	id_type temp = 0;
	mapping_type return_value;

	const iter_domain_vector_type 
		max_window_iteration(snk_iterator.max_window_iteration());

	for(unsigned col = 0; 
			col < border_condition_matrix.size2();
			col++){
		temp += 
			border_condition_matrix(token_dimension,col) * max_window_iteration[col];
	}		
	

	if (temp <= (id_type)high_border_condition_vector[token_dimension]){
		return_value = 0;
	}else{
		return_value = 
			(mapping_type)(temp - (id_type)high_border_condition_vector[token_dimension]);

		mapping_type window_size = 			
			snk_iterator.iteration_max(mapping_matrix.size2() - _token_dimensions + token_dimension,
																 snk_iterator.iteration_vector());
		assert(mapping_matrix(token_dimension, mapping_matrix.size2() - _token_dimensions + token_dimension) == 1);	

		if (return_value > window_size)
			return_value = window_size;
	}

	return return_value;	
}



bool smoc_md_loop_snk_data_element_mapper::calc_window_displacement(const smoc_md_loop_iterator_kind& snk_iterator,
																																		unsigned token_dimension,
																																		mapping_type& window_displacement
																																		) const {
	id_type temp_window_displacement = 0;

	bool finished = false;

	for(int col = mapping_matrix.size2() - 1 - _token_dimensions;
			col >= 0;
			col--){
		if(mapping_matrix(token_dimension, col) != 0){
			//iteration level is relevant for given token dimension
			if (snk_iterator[col] >= 
					snk_iterator.iteration_max(col, snk_iterator.iteration_vector())){
				temp_window_displacement -= snk_iterator[col] * mapping_matrix(row,col);
			}else{
				temp_window_displacement += mapping_matrix(row,col);
				finished = true;
				break;
			}
		}
	}


	if (!finished){
		//In the given dimension, we are at the end of a schedule period
		return false;
	}else{
		assert(temp_window_displacement > 0);
		return (mapping_type)temp_window_displacement;
	}

}
#endif


bool
smoc_md_loop_snk_data_element_mapper::calc_eff_window_displacement(const smoc_md_loop_iterator_kind& snk_iterator,
																																	 unsigned token_dimension,
																																	 mapping_type& window_displacement) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
	dout << "Enter smoc_md_loop_snk_data_element_mapper::calc_eff_window_displacement" << endl;
	dout << inc_level;
	dout << "snk_iterator.iteration_vector() = " << snk_iterator.iteration_vector();
	dout << endl;
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
			if (snk_iterator[col] >= 
					snk_iterator.iteration_max(col, snk_iterator.iteration_vector())){
				//coordinate will become zero
				temp_window_displacement -= snk_iterator[col] * 
					mapping_matrix(token_dimension,col);
				border_condition_change -= snk_iterator[col] * 
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
	dout << "border_condition_change = " << border_condition_change << endl;
	dout << "temp_window_displacement = " << temp_window_displacement << endl;
#endif


	if (!finished){
		//In the given dimension, we are at the end of a schedule period
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
		dout << "End of schedule period" << endl;
		dout << "Leave smoc_md_loop_snk_data_element_mapper::calc_eff_window_displacement" << endl;
		dout << dec_level;
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
			base_border_condition(calc_base_border_condition(snk_iterator.iteration_vector(),
																											 token_dimension));
		id_type delta_low_condition = 
			low_border_condition_vector[token_dimension] - base_border_condition;
		id_type delta_high_condition =
			base_border_condition - high_border_condition_vector[token_dimension];

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 104
		dout << "delta_low_condition = " << delta_low_condition << endl;
		dout << "delta_high_condition = " << delta_high_condition << endl;
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
			base_border_condition(calc_base_border_condition(snk_iterator.iteration_vector(),
																											 token_dimension));
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
	dout << "Leave smoc_md_loop_snk_data_element_mapper::calc_eff_window_displacement" << endl;
	dout << dec_level;
#endif

	return true;
	
	
}


bool smoc_md_loop_snk_data_element_mapper::check_border_condition_matrix(const border_condition_matrix_type& border_matrix) const {
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


smoc_md_loop_snk_data_element_mapper::id_type 
smoc_md_loop_snk_data_element_mapper::calc_base_border_condition(const iter_domain_vector_type& iteration,
																																 unsigned dimension) const {
	id_type return_value = 0;

	for(unsigned col = 0;
			col < iteration.size() - _token_dimensions;
			col++){
		return_value += 
			border_condition_matrix(dimension,col) * iteration[col];
	}

	return return_value;
}

smoc_md_loop_snk_data_element_mapper::id_type 
smoc_md_loop_snk_data_element_mapper::calc_window_border_condition(id_type base_border_condition,
																																	 const iter_domain_vector_type& iteration,
																																	 unsigned dimension) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "Enter smoc_md_loop_snk_data_element_mapper::calc_window_border_condition" << endl;
	dout << inc_level;
	dout << "dimension = " << dimension << endl;
	dout << "iteration.size() = " << iteration.size() << endl;
	dout << "_token_dimensions = " << _token_dimensions << endl;
	dout << "border_condition_matrix = " << border_condition_matrix;
	dout << endl;
	dout << "iteration = " << iteration;
	dout << endl;
#endif
	for(unsigned col = iteration.size() - _token_dimensions;
			col < iteration.size();
			col++){
		base_border_condition += 
			border_condition_matrix(dimension,col) * iteration[col];
	}

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 103
	dout << "return_value = " << base_border_condition << endl;
	dout << "Leave smoc_md_loop_snk_data_element_mapper::calc_window_border_condition" << endl;
	dout << dec_level;
#endif

	return base_border_condition;
}


smoc_md_loop_snk_data_element_mapper::border_condition_vector_type
smoc_md_loop_snk_data_element_mapper::calc_base_border_condition_vector(const iter_domain_vector_type& iteration) const {
	border_condition_vector_type return_vector(_token_dimensions);
#ifdef FAST_CALC_MODE
	for(unsigned row = 0; row < _token_dimensions; row++){
		return_vector[row] = 0;
	}
	for(unsigned col = 0; col < iteration.size()-_token_dimensions; col++){
		const int row = mapping_table[col];
		if (row >= 0){
			return_vector[row] += 
				border_condition_matrix(row,col)*iteration[col];
		}
	}
#else
	for(unsigned row = 0; row < _token_dimensions; row++){
		return_vector[row] = 
			calc_base_border_condition(iteration,row);				
	}
#endif

	return return_vector;	
}

smoc_md_loop_snk_data_element_mapper::border_condition_vector_type 
smoc_md_loop_snk_data_element_mapper::calc_window_border_condition_vector(const border_condition_vector_type& base_border_condition_vector,
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


smoc_md_loop_snk_data_element_mapper::border_condition_vector_type
smoc_md_loop_snk_data_element_mapper::calc_border_condition_offset(const iter_domain_vector_type& window_iteration) const {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
	dout << "Enter smoc_md_loop_snk_data_element_mapper::calc_border_condition_offset" << endl;
	dout << inc_level;
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
	dout << "Leave smoc_md_loop_snk_data_element_mapper::calc_border_condition_offset" << endl;
	dout << dec_level;
#endif

	return return_vector;
}

smoc_md_loop_snk_data_element_mapper::border_type_vector_type 
smoc_md_loop_snk_data_element_mapper::is_border_pixel(const border_condition_vector_type& border_condition_vector,
																											bool& is_border) const{
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
	dout << "Enter smoc_md_loop_snk_data_element_mapper::is_border_pixel" << endl;
	dout << inc_level;
	dout << "border_condition_vector = " << border_condition_vector;
	dout << endl;
	dout << "low_border_condition_vector = " << low_border_condition_vector;
	dout << endl;
	dout << "high_border_condition_vector = " << high_border_condition_vector;
	dout << endl;
#endif

	border_type_vector_type return_vector(_token_dimensions);

	is_border = false;

	for(unsigned int i = 0; i < _token_dimensions; i++){
		if (border_condition_vector[i] < low_border_condition_vector[i]){
			return_vector[i] = LEFT_BORDER;
			is_border = true;
		}else if(border_condition_vector[i] > high_border_condition_vector[i]){
			return_vector[i] = RIGHT_BORDER;
			is_border = true;
		}else{
			return_vector[i] = NO_BORDER;
		}
	}

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 106
	dout << "Leavesmoc_md_loop_snk_data_element_mapper::is_border_pixel" << endl;
	dout << dec_level;
#endif

	return return_vector;
}
