
#include <systemoc/smoc_md_buffer.hpp>
#include <cosupport/smoc_debug_out.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BUFFER
#define VERBOSE_LEVEL_SMOC_MD_BUFFER 0
///101: general Debug
///102: allocate_buffer
#endif

/* ********************************************************************* */
/*                      smoc_md_buffer_mgmt_base                         */
/* ********************************************************************* */

/* ********************************************************************* */
/*                    smoc_simple_md_buffer_kind                         */
/* ********************************************************************* */

void smoc_simple_md_buffer_kind::allocate_buffer() {

	/// we must have checked previously, whether memory can be allocated
	assert(cache_unusedStorage);

	wr_schedule_period_start = cache_wr_schedule_period_start;
	wr_max_data_element_offset = cache_wr_max_data_element_offset;
	free_lines = cache_free_lines;
}
	
void smoc_simple_md_buffer_kind::release_buffer(){
	cache_unusedStorage = false;
}

bool smoc_simple_md_buffer_kind::hasUnusedStorage() const {

	bool return_value = cache_unusedStorage;

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
	CoSupport::dout << "Enter smoc_simple_md_buffer_kind::hasUnusedStorage" << std::endl;
	CoSupport::dout << CoSupport::Indent::Up;
#endif

	if (!return_value){
		const bool new_schedule_period = src_loop_iterator.is_new_schedule_period();

		//check, whether source iterator has started a new schedule period
		if (new_schedule_period){
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
			CoSupport::dout << "New schedule period" << std::endl;
#endif
			cache_wr_schedule_period_start = 
				wr_schedule_period_start +
				size_token_space[_token_dimensions-1];
			
			cache_wr_schedule_period_start =
				cache_wr_schedule_period_start % buffer_lines;
		}
		
		data_element_id_type max_src_data_element_id(_token_dimensions);
		id_type schedule_period_offset;
		
		//Get data element with maximum coordinate
		src_loop_iterator.max_data_element_id(max_src_data_element_id,
																			schedule_period_offset);
		max_src_data_element_id[_token_dimensions-1] +=
			schedule_period_offset * size_token_space[_token_dimensions-1];
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
		CoSupport::dout << "max_src_data_element_id = " << max_src_data_element_id;
		CoSupport::dout << std::endl;
#endif
		
		// Calculate number of new lines
		unsigned long new_lines = calc_req_new_lines(max_src_data_element_id,
																								 new_schedule_period);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
		CoSupport::dout << "Required " << new_lines << " new lines" << std::endl;
#endif
		
		if(free_lines < new_lines){
			//buffer allocation failed			
			return_value = false;
		}else if (new_lines > 0){
			cache_free_lines = free_lines - new_lines;
			cache_wr_max_data_element_offset = max_src_data_element_id[_token_dimensions-1];
			return_value = true;		
			cache_unusedStorage = true;
		}else{
			// buffer line already allocated
			return_value = true;
			cache_unusedStorage = true;
		}			
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
		if (return_value){
			CoSupport::dout << "Allocation succeeded" << std::endl;
		}else{
			CoSupport::dout << "Allocation failed" << std::endl;
		}
#endif
	}else{

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
		CoSupport::dout << "Buffer already allocated" << std::endl;
#endif
	}
		
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
	CoSupport::dout << "Leave smoc_simple_md_buffer_kind::hasUnusedStorage" << std::endl;
	CoSupport::dout << CoSupport::Indent::Down;
#endif

	return return_value;
}


unsigned long smoc_simple_md_buffer_kind::calc_req_new_lines(const data_element_id_type& data_element_id, 
																														 bool new_schedule_period) const {
	unsigned long new_lines = 0;
	
	if (new_schedule_period){
		new_lines = data_element_id[_token_dimensions-1] + 1 +
			size_token_space[_token_dimensions-1] - wr_max_data_element_offset - 1;
	}else if (data_element_id[_token_dimensions-1] > id_type(wr_max_data_element_offset)){
		new_lines = data_element_id[_token_dimensions-1] - 
			wr_max_data_element_offset;
	}

	return new_lines;
}

void smoc_simple_md_buffer_kind::free_buffer() {

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
	CoSupport::dout << "Enter smoc_simple_md_buffer_kind::free_buffer" << std::endl;
	CoSupport::dout << CoSupport::Indent::Up;
	CoSupport::dout << "free_lines = " << free_lines << std::endl;
	CoSupport::dout << "buffer_lines = " << buffer_lines << std::endl;
#endif

	for(unsigned i = 0; i < _token_dimensions-1; i++){
		if (!snk_loop_iterator.is_iteration_max(i)){
			//we cannot free complete line
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
			CoSupport::dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << std::endl;
			CoSupport::dout << CoSupport::Indent::Down;
#endif
			return;
		}
	}

	//If we arrived here, we can free a complete line
	smoc_snk_md_loop_iterator_kind::mapping_type window_displacement;
	if(!snk_loop_iterator.calc_eff_window_displacement(_token_dimensions-1,
																								window_displacement)){
		//We are at the end of a schedule period.
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
		CoSupport::dout << "End of schedule period" << std::endl;
#endif
		free_lines += 
			size_token_space[_token_dimensions-1] - rd_min_data_element_offset;
		cache_free_lines += 
			size_token_space[_token_dimensions-1] - rd_min_data_element_offset;
		assert(free_lines <= buffer_lines);
		rd_min_data_element_offset = 0;
		rd_schedule_period_start += size_token_space[_token_dimensions-1];
		rd_schedule_period_start %= buffer_lines;
	}else{
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
		CoSupport::dout << "free " << window_displacement << " lines" << std::endl;
		CoSupport::dout << std::endl;
#endif
		free_lines += window_displacement;
		cache_free_lines += window_displacement;
		assert(free_lines <= buffer_lines);
		rd_min_data_element_offset += window_displacement;
	}	

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
	CoSupport::dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << std::endl;
	CoSupport::dout << CoSupport::Indent::Down;
#endif
}


