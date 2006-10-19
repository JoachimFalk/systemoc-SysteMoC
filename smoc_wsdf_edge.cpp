
#include "smoc_wsdf_edge.hpp"

//0: No output
///100: debug
#define VERBOSE_LEVEL 0

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::snk_iteration_max() const {

	u2vector_type snk_iteration_level_table = 
		calc_snk_iteration_level_table();
	
	uvector_type snk_vtu_iteration_level(token_dimensions);
	insert_snk_vtu_iterations(snk_iteration_level_table,
														snk_vtu_iteration_level
														);

	uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
																										snk_vtu_iteration_level));

	append_snk_window_iteration(iteration_max);

	return iteration_max;
}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::src_iteration_max() const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter  smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
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
#if VERBOSE_LEVEL == 100
			std::cout << "firing_level = " << firing_level;
			std::cout << " token_dimension = " << token_dimension << std::endl;
#endif
			if (src_has_iteration_level(firing_level, token_dimension)){
				assert(src_firing_blocks[firing_level][token_dimension] %
							 current_firing_block_size[token_dimension] == 0);
				return_vector[iter_level] = 
					src_firing_blocks[firing_level][token_dimension] /
					current_firing_block_size[token_dimension];
				
				iter_level--;
				current_firing_block_size[token_dimension] = 
					src_firing_blocks[firing_level][token_dimension];
			}
		}
	}

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
#endif

	return return_vector;
}

smoc_wsdf_edge_descr::svector_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_vector() const {
	return -(bs);
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_matrix() const {

	u2vector_type snk_iteration_level_table = 
		calc_snk_iteration_level_table();
	
	uvector_type snk_vtu_iteration_level(token_dimensions);
	insert_snk_vtu_iterations(snk_iteration_level_table,
														snk_vtu_iteration_level
														);

	uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
																										snk_vtu_iteration_level));

	umatrix_type return_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
																																	snk_vtu_iteration_level,
																																	iteration_max));


	return return_matrix;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::src_data_element_mapping_vector() const {
	return d;
}

smoc_wsdf_edge_descr::smatrix_type 
smoc_wsdf_edge_descr::calc_border_condition_matrix() const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix()" << std::endl;
#endif

	u2vector_type snk_iteration_level_table = 
		calc_snk_iteration_level_table();
	
	uvector_type snk_vtu_iteration_level(token_dimensions);
	insert_snk_vtu_iterations(snk_iteration_level_table,
														snk_vtu_iteration_level
														);

	uvector_type iteration_max(calc_snk_iteration_max(snk_iteration_level_table,
																										snk_vtu_iteration_level));

#if VERBOSE_LEVEL == 100
	std::cout << "Iteration-Max (without window iteration): " << iteration_max << std::endl;
#endif

	umatrix_type mapping_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
																																	 snk_vtu_iteration_level,
																																	 iteration_max));

#if VERBOSE_LEVEL == 100
	std::cout << "Mapping-matrix: " << mapping_matrix << std::endl;
#endif


	smatrix_type return_matrix(mapping_matrix.size1(),mapping_matrix.size2());

	calc_border_condition_matrix(mapping_matrix,
															 snk_vtu_iteration_level,
															 return_matrix);

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix()" << std::endl;
#endif


	return return_matrix;
}


smoc_wsdf_edge_descr::svector_type
smoc_wsdf_edge_descr::calc_border_condition_vector() const {
	svector_type return_vector(u0.size());

	for(unsigned int i = 0; i < u0.size(); i++){
		return_vector[i] = u0[i] - 1;
	}

	return return_vector;
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::src_data_element_mapping_matrix() const {

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

	return return_matrix;
}


smoc_wsdf_edge_descr::uvector_type
smoc_wsdf_edge_descr::calc_snk_r_vtu() const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
#endif

	uvector_type return_vector(u0+bs+bt-c);

	for(unsigned int i = 0; i < token_dimensions; i++){
		assert(return_vector[i] % delta_c[i] == 0); //Invalid parameters

		return_vector[i] /= delta_c[i];
		return_vector[i]++;
	}

#if VERBOSE_LEVEL == 100
	std::cout << "snk_r_vtu = " << return_vector << std::endl;
#endif

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
#endif

	return return_vector;
}

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_src_r_vtu() const {
	uvector_type return_vector(token_dimensions);
	
	for(unsigned int i = 0; i < token_dimensions; i++){
		//currently, we require, that an effective token
		//exactly belongs to one virtual token union
		assert(u0[i] % p[i] == 0);

		return_vector[i] = u0[i] / p[i];
	}

	return return_vector;
}



void smoc_wsdf_edge_descr::check_local_balance() const {

	//Calculate number of invocations per virtual token union
	const uvector_type snk_r_vtu(calc_snk_r_vtu());
	const uvector_type src_r_vtu(calc_src_r_vtu());

	//calculate the number of virtual token unions in each dimension
	udata_type snk_num_vtu;
	udata_type src_num_vtu;

	for(unsigned int i = 0; i < token_dimensions; i++){
		//Check for incomplete virtual token union
		assert(snk_firing_blocks[snk_num_firing_levels-1][i] % snk_r_vtu[i] == 0);

		snk_num_vtu = 
			snk_firing_blocks[snk_num_firing_levels-1][i] / snk_r_vtu[i];

		//Check for incomplete virtual token unions
		assert(src_firing_blocks[src_num_firing_levels-1][i] % src_r_vtu[i] == 0);

		src_num_vtu = 
			src_firing_blocks[src_num_firing_levels-1][i] / src_r_vtu[i];

		
		//Check, if edge balanced
		assert(snk_num_vtu == src_num_vtu);
	}	
}


void smoc_wsdf_edge_descr::check_parameters() const {
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
}


bool smoc_wsdf_edge_descr::snk_has_iteration_level(unsigned firing_level,
																									 unsigned token_dimension) const {
	if (firing_level == 0){
		//Always include firing_level 0
		return true;
	}else if (snk_firing_blocks[firing_level][token_dimension] == 
						snk_firing_blocks[firing_level-1][token_dimension]){
		return false;
	}else{
		return true;
	}
}

bool smoc_wsdf_edge_descr::src_has_iteration_level(unsigned firing_level,
																									 unsigned token_dimension) const {
	if (firing_level == 0){
		//Always include firing_level 0
		return true;
	}else if (src_firing_blocks[firing_level][token_dimension] == 
						src_firing_blocks[firing_level-1][token_dimension]){
		return false;
	}else{
		return true;
	}
}


smoc_wsdf_edge_descr::u2vector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_level_table() const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
#endif

	unsigned iteration_level = 0;

	//Generate a data structure with the same number of data elements than
	//snk_firing_blocks
	u2vector_type return_table(snk_firing_blocks);

#if VERBOSE_LEVEL == 100
	std::cout << "Size of return-table: " << return_table.size() << std::endl;
#endif

	for(int firing_level = snk_num_firing_levels-1; 
			firing_level >= 0; 
			firing_level--){
		for(int token_dimension = token_dimensions-1; 
				token_dimension >= 0; 
				token_dimension--){

#if VERBOSE_LEVEL == 100
			std::cout << "firing_level = " << firing_level << std::endl;
			std::cout << "token_dimension = " << token_dimension << std::endl;
			std::cout << "iteration_level = " << iteration_level << std::endl;
#endif
			return_table[firing_level][token_dimension] = iteration_level;
			
			if (snk_has_iteration_level(firing_level, token_dimension)){
				iteration_level++;
			}
		}
	}

#if VERBOSE_LEVEL == 100
	std::cout << "Leave  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
#endif

	return return_table;
}


void smoc_wsdf_edge_descr::insert_snk_vtu_iterations(u2vector_type& iteration_level_table,
																										 uvector_type& vtu_iteration_level,
																										 bvector_type& new_vtu_iteration) const {


#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::insert_snk_vtu_iterations" << std::endl;
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

#if VERBOSE_LEVEL == 100
			std::cout << "firing_level = " << firing_level << std::endl;
			std::cout << "token_dimension = " << token_dimension << std::endl;
#endif

			//update iteration level table in order to take
			//previous modifications into account
			iteration_level_table[firing_level][token_dimension] += level_inc;

						
			if (!found[token_dimension]){
				if (snk_has_iteration_level(firing_level, token_dimension)){
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

#if VERBOSE_LEVEL == 100
	std::cout << "iteration_level_table = " << iteration_level_table << std::endl;
	std::cout << "vtu_iteration_level = " << vtu_iteration_level << std::endl;
	std::cout << "Leave smoc_wsdf_edge_descr::insert_snk_vtu_iterations" << std::endl;
#endif

}

void smoc_wsdf_edge_descr::insert_snk_vtu_iterations(u2vector_type& snk_iteration_level_table,
																										 uvector_type& snk_vtu_iteration_level
																										 ) const {
	bvector_type dummy(snk_vtu_iteration_level.size());
	
	insert_snk_vtu_iterations(snk_iteration_level_table,
														snk_vtu_iteration_level,
														dummy);
}

unsigned 
smoc_wsdf_edge_descr::get_num_iteration_levels(const u2vector_type& snk_iteration_level_table,
																							 const uvector_type& snk_vtu_iteration_level) const {

	unsigned return_value;

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
#endif

	if(snk_iteration_level_table[0][0] > snk_vtu_iteration_level[0])
		return_value = snk_iteration_level_table[0][0]+1;
	else
		return_value = snk_vtu_iteration_level[0] + 1;

#if VERBOSE_LEVEL == 100
	std::cout << "return_value = " << return_value << std::endl;
	std::cout << "Leave smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
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


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_max(const u2vector_type& snk_iteration_level_table,
																						 const uvector_type& snk_vtu_iteration_level
																						 ) const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
#endif

	unsigned num_iteration_levels = get_num_iteration_levels(snk_iteration_level_table,
																													 snk_vtu_iteration_level
																													 );

#if VERBOSE_LEVEL == 100
	std::cout << "num_iteration_levels = " << num_iteration_levels << std::endl;
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
			if (snk_has_iteration_level(firing_level, token_dimension)){
#if VERBOSE_LEVEL == 100
				std::cout << "firing_level = " << firing_level << std::endl;
				std::cout << "token_dimension = " << token_dimension << std::endl;
#endif

				// check for complete blocks
				assert(snk_firing_blocks[firing_level][token_dimension] %
							 current_firing_block_size[token_dimension] == 0);
				
				return_vector[snk_iteration_level_table[firing_level][token_dimension]] = 
					snk_firing_blocks[firing_level][token_dimension] /
					current_firing_block_size[token_dimension]
					-1;				

				// Check, if vtu is covered
				if (snk_vtu_iteration_level[token_dimension] ==
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
						assert(snk_r_vtu[token_dimension] %	current_firing_block_size[token_dimension] == 0);
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

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
#endif

	return return_vector;
}




void smoc_wsdf_edge_descr::append_snk_window_iteration(uvector_type& iteration_max) const {

	unsigned old_size = iteration_max.size();
	unsigned new_size = old_size + token_dimensions;
	iteration_max.resize(new_size);

	for(int token_dimension = token_dimensions-1;
			token_dimension >= 0;
			token_dimension--){
		iteration_max[old_size+token_dimension] = c[token_dimension]-1;
	}
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix(const u2vector_type& snk_iteration_level_table,
																													 const uvector_type& snk_vtu_iteration_level,
																													 const uvector_type& snk_iter_max
																													 ) const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
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
		token_dimensions;
	
	umatrix_type return_matrix(matrix_rows, matrix_cols);

	for(unsigned token_dimension = 0;
			token_dimension < token_dimensions;
			token_dimension++){
		for(unsigned firing_level = 0; 
				firing_level < snk_num_firing_levels; 
				firing_level++){
			if (snk_has_iteration_level(firing_level, token_dimension)){
#if VERBOSE_LEVEL == 100
				std::cout << "firing_level = " << firing_level << std::endl;
				std::cout << "token_dimension = " << token_dimension << std::endl;
#endif

				//default assignment
				for(unsigned row = 0; row < matrix_rows; row++){
					return_matrix(row, snk_iteration_level_table[firing_level][token_dimension]) = 0;
				}

				return_matrix(token_dimension,snk_iteration_level_table[firing_level][token_dimension]) =
					prev_mapping_factor[token_dimension];
				
				//check, whether iteration level represents vtu
				if (snk_vtu_iteration_level[token_dimension] == 
						snk_iteration_level_table[firing_level][token_dimension]){
					//vtu requires special attention
					prev_mapping_factor[token_dimension] =
						u0[token_dimension];
					found_vtu[token_dimension] = true;
				}else if((snk_vtu_iteration_level[token_dimension] > 
									snk_iteration_level_table[firing_level][token_dimension]) &&
								 (!found_vtu[token_dimension])){
					//Firing block is already larger than vtu. However, vtu has not already
					//been covered

#if VERBOSE_LEVEL == 100
					std::cout << "Insert vtu" << std::endl;
#endif

					//Insert data element mapping for vtu
					for(unsigned row = 0; row < matrix_rows; row++){
						return_matrix(row, snk_vtu_iteration_level[token_dimension]) = 0;
					}
					return_matrix(token_dimension,snk_vtu_iteration_level[token_dimension]) =
						prev_mapping_factor[token_dimension];

#if VERBOSE_LEVEL == 100
					std::cout << "prev_mapping_factor[token_dimension] = " 
										<< prev_mapping_factor[token_dimension] << std::endl;
#endif
					
					//Correct data mapping weighths
					return_matrix(token_dimension,snk_iteration_level_table[firing_level][token_dimension]) =
						u0[token_dimension];

					//Calculate new data element mapping factor
					prev_mapping_factor[token_dimension] = u0[token_dimension] * 
						(snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] + 1);

#if VERBOSE_LEVEL == 100
					std::cout << "snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] = " 
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
	insert_snk_window_mapping(return_matrix);

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
#endif

	return return_matrix;

}


void 
smoc_wsdf_edge_descr::insert_snk_window_mapping(umatrix_type& data_element_mapping_matrix) const {
	const unsigned matrix_cols = data_element_mapping_matrix.size2();

	for(unsigned token_dimension = 0;
			token_dimension < token_dimensions;
			token_dimension++){
		for(unsigned delta_col = 0;
				delta_col < token_dimensions;
				delta_col++){
			//default assingment
			data_element_mapping_matrix(token_dimension,delta_col+matrix_cols-token_dimensions) = 0;
		}
		
		data_element_mapping_matrix(token_dimension,matrix_cols - token_dimension - 1) = 1;
	}
}



void
smoc_wsdf_edge_descr::calc_border_condition_matrix(const umatrix_type& mapping_matrix,
																									 const uvector_type& snk_vtu_iteration_level,
																									 smatrix_type& border_cond_matrix) const {

#if VERBOSE_LEVEL == 100
	std::cout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
#endif
	
	const unsigned num_rows = mapping_matrix.size1();
	const unsigned num_cols = mapping_matrix.size2();


	for(unsigned row = 0; row < num_rows; row++){
#if VERBOSE_LEVEL == 100
		std::cout << "row = " << row 
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
#if VERBOSE_LEVEL == 100
			std::cout << "col = " << col << std::endl; 
			std::cout << "mapping_matrix(row, col) = " 
								<< mapping_matrix(row, col) << std::endl;
#endif
			border_cond_matrix(row, col) = mapping_matrix(row, col);
		}
	}	

#if VERBOSE_LEVEL == 100
	std::cout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
#endif

}
