
#include "smoc_md_buffer.hpp"
#include <smoc_debug_out.h>


#define VERBOSE_LEVEL 101
///101: general Debug
///102: unusedStorage

bool smoc_simple_md_buffer_kind::allocate_buffer(const smoc_md_loop_iterator_kind& src_iterator) {

	bool return_value;

#if VERBOSE_LEVEL == 101
	dout << "Enter smoc_simple_md_buffer_kind::allocate_buffer" << endl;
	dout << inc_level;
#endif

	unsigned long wr_schedule_period_start_old(wr_schedule_period_start);
	const bool new_schedule_period = src_iterator.is_new_schedule_period();

	//check, whether source iterator has started a new schedule period
	if ((new_schedule_period) && (!init)){
		wr_schedule_period_start += 
			size_token_space[_token_dimensions-1];

		wr_schedule_period_start =
			wr_schedule_period_start % buffer_lines;
	}
	
	data_element_id_type max_src_data_element_id(_token_dimensions);
	id_type schedule_period_offset;
	
	//Get data element with maximum coordinate
	src_data_el_mapper.max_data_element_id(src_iterator,
																				 max_src_data_element_id,
																				 schedule_period_offset);
	max_src_data_element_id[_token_dimensions-1] +=
		schedule_period_offset * size_token_space[_token_dimensions-1];

	// Calculate number of new lines
	unsigned long new_lines = calc_req_new_lines(max_src_data_element_id,
																							 new_schedule_period);

	if(free_lines < new_lines){
		//buffer allocation failed
		wr_schedule_period_start = wr_schedule_period_start_old;
		return_value = false;
	}else if (new_lines > 0){
		free_lines -= new_lines;
		wr_max_data_element_offset = max_src_data_element_id[_token_dimensions-1];
		init = false;
		return_value = true;		
	}else{
		// buffer line already allocated
		init = false;
		return_value = true;
	}	

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_simple_md_buffer_kind::allocate_buffer" << endl;
	dout << dec_level;
#endif

	return return_value;
}



bool smoc_simple_md_buffer_kind::unusedStorage(const smoc_md_loop_iterator_kind& src_iterator) const {
	unsigned long wr_schedule_period_start(this->wr_schedule_period_start);
	const bool new_schedule_period = src_iterator.is_new_schedule_period();

	bool return_value;

#if VERBOSE_LEVEL == 101
	dout << "Enter smoc_simple_md_buffer_kind::unusedStorage" << endl;
	dout << inc_level;
#endif

	//check, whether source iterator has started a new schedule period
	if (new_schedule_period){
		wr_schedule_period_start += 
			size_token_space[_token_dimensions-1];

		wr_schedule_period_start =
			wr_schedule_period_start % buffer_lines;
	}
	
	data_element_id_type max_src_data_element_id(_token_dimensions);
	id_type schedule_period_offset;
	
	//Get data element with maximum coordinate
	src_data_el_mapper.max_data_element_id(src_iterator,
																				 max_src_data_element_id,
																				 schedule_period_offset);
	max_src_data_element_id[_token_dimensions-1] +=
		schedule_period_offset * size_token_space[_token_dimensions-1];

#if VERBOSE_LEVEL == 102
	dout << "max_src_data_element_id = " << max_src_data_element_id;
	dout << endl;
#endif

	// Calculate number of new lines
	unsigned long new_lines = calc_req_new_lines(max_src_data_element_id,
																							 new_schedule_period);
#if VERBOSE_LEVEL == 101
	dout << "Requires " << new_lines << " new lines" << endl;
	dout << "free_lines = " << free_lines << endl;
#endif

	if(free_lines < new_lines){
		//buffer allocation failed
		return_value = false;
	}else{
		return_value = true;
	}

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_simple_md_buffer_kind::unusedStorage" << endl;
	dout << dec_level;
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

void smoc_simple_md_buffer_kind::free_buffer(const smoc_md_loop_iterator_kind& snk_iterator) {

#if VERBOSE_LEVEL == 101
	dout << "Enter smoc_simple_md_buffer_kind::free_buffer" << endl;
	dout << inc_level;
	dout << "free_lines = " << free_lines << endl;
	dout << "buffer_lines = " << buffer_lines << endl;
#endif

	for(unsigned i = 0; i < _token_dimensions-1; i++){
		if (!snk_data_el_mapper.is_iteration_max(snk_iterator,i)){
			//we cannot free complete line
#if VERBOSE_LEVEL == 101
			dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << endl;
			dout << dec_level;
#endif
			return;
		}
	}

	//If we arrived here, we can free a complete line
	smoc_md_loop_snk_data_element_mapper::mapping_type window_displacement;
	if(!snk_data_el_mapper.calc_eff_window_displacement(snk_iterator,
																										 _token_dimensions-1,
																										 window_displacement)){
		//We are at the end of a schedule period.
#if VERBOSE_LEVEL == 101
		dout << "End of schedule period" << endl;
#endif
		free_lines += 
			size_token_space[_token_dimensions-1] - rd_min_data_element_offset;
		assert(free_lines <= buffer_lines);
		rd_min_data_element_offset = 0;
		rd_schedule_period_start += size_token_space[_token_dimensions-1];
		rd_schedule_period_start %= buffer_lines;
	}else{
#if VERBOSE_LEVEL == 101
		dout << "free " << window_displacement << " lines" << endl;
		dout << endl;
#endif
		free_lines += window_displacement;
		assert(free_lines <= buffer_lines);
		rd_min_data_element_offset += window_displacement;
	}	

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << endl;
	dout << dec_level;
#endif
}


