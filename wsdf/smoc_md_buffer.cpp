//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include <systemoc/smoc_md_buffer.hpp>
#include <CoSupport/Streams/DebugOStream.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BUFFER
#define VERBOSE_LEVEL_SMOC_MD_BUFFER 0
///101: general Debug
///102: allocate_buffer
#endif

/* ********************************************************************* */
/*                      smoc_md_buffer_mgmt_base                         */
/* ********************************************************************* */

#if 0
void 
smoc_md_buffer_mgmt_base::channelAttributes(smoc_modes::PGWriter &pgw) const{
  pgw << "<attribute type=\"token_dimensions\" value=\""
      << this->_token_dimensions 
      << "\"/>"
      << std::endl;

  /* Source iterator */
  pgw << "<attribute type=\"src_iter_max\" value=\""
      << this->src_loop_iterator.iteration_max()
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"src_mapping_matrix\" value=\""
      << this->src_loop_iterator.mapping_matrix
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"src_mapping_offset\" value=\""
      << this->src_loop_iterator.mapping_offset
      << "\"/>"
      << std::endl;
  

  /* Sink iterator */
  pgw << "<attribute type=\"snk_iter_max\" value=\""
      << this->snk_loop_iterator.iteration_max()
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"snk_mapping_matrix\" value=\""
      << this->snk_loop_iterator.mapping_matrix
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"snk_mapping_offset\" value=\""
      << this->snk_loop_iterator.mapping_offset
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"snk_border_condition_matrix\" value=\""
      << this->snk_loop_iterator.border_condition_matrix
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"snk_low_border_condition_vector\" value=\""
      << this->snk_loop_iterator.low_border_condition_vector
      << "\"/>"
      << std::endl;
  pgw << "<attribute type=\"snk_high_border_condition_vector\" value=\""
      << this->snk_loop_iterator.high_border_condition_vector
      << "\"/>"
      << std::endl;  

  /* WSDF edge parameters */
  {
    std::stringstream temp;
    wsdf_edge_params.print_edge_parameters(temp,false);
    pgw << "<attribute type=\"wsdf_params\" value=\""
        << temp.str()
        << "\"/>"
        << std::endl;  
  }
}
#endif


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

  if (buffer_lines == MAX_TYPE(size_t)){
    //Assume infinite buffer size

    cache_unusedStorage = true;
    
    return true;
  }

  bool return_value = cache_unusedStorage;

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
  CoSupport::Streams::dout << "Enter smoc_simple_md_buffer_kind::hasUnusedStorage" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

  if (!return_value){
    const bool new_schedule_period = src_loop_iterator.is_new_schedule_period();

    //check, whether source iterator has started a new schedule period
    if (new_schedule_period){
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
      CoSupport::Streams::dout << "New schedule period" << std::endl;
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
    CoSupport::Streams::dout << "max_src_data_element_id = " << max_src_data_element_id;
    CoSupport::Streams::dout << std::endl;
#endif
                
    // Calculate number of new lines
    unsigned long new_lines = calc_req_new_lines(max_src_data_element_id,
						 new_schedule_period);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
    CoSupport::Streams::dout << "Required " << new_lines << " new lines" << std::endl;
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
      CoSupport::Streams::dout << "Allocation succeeded" << std::endl;
    }else{
      CoSupport::Streams::dout << "Allocation failed" << std::endl;
    }
#endif
  }else{

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 102
    CoSupport::Streams::dout << "Buffer already allocated" << std::endl;
#endif
  }
                
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
  CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::hasUnusedStorage" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
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

  if (buffer_lines == MAX_TYPE(size_t)){
    //Assume infinite storage
    return;
  }

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
  CoSupport::Streams::dout << "Enter smoc_simple_md_buffer_kind::free_buffer" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Up;
  CoSupport::Streams::dout << "free_lines = " << free_lines << std::endl;
  CoSupport::Streams::dout << "buffer_lines = " << buffer_lines << std::endl;
#endif

  for(unsigned i = 0; i < _token_dimensions-1; i++){
    if (!snk_loop_iterator.is_iteration_max(i)){
      //we cannot free complete line
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
      CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
      return;
    }
  }

  if (!snk_loop_iterator.is_virt_iteration_max()){
    //we cannot free complete line
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
    CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << std::endl;
    CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
    return;
  }
    

  //If we arrived here, we can free a complete line
  smoc_snk_md_loop_iterator_kind::mapping_type window_displacement;
  if(!snk_loop_iterator.calc_eff_window_displacement(_token_dimensions-1,
						     window_displacement)){
    //We are at the end of a schedule period.
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
    CoSupport::Streams::dout << "End of schedule period" << std::endl;
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
    CoSupport::Streams::dout << "free " << window_displacement << " lines" << std::endl;
    CoSupport::Streams::dout << std::endl;
#endif
    free_lines += window_displacement;
    cache_free_lines += window_displacement;
    assert(free_lines <= buffer_lines);
    rd_min_data_element_offset += window_displacement;
  }       

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102)
  CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::free_buffer" << std::endl;
  CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
}

#if 0
void 
smoc_simple_md_buffer_kind::channelAttributes(smoc_modes::PGWriter &pgw) const{
  parent_type::channelAttributes(pgw);
  pgw << "<attribute type=\"buffer_lines\" value=\""
      << buffer_lines
      << "\"/>"
      << std::endl;
}
#endif

template<>
void smoc_simple_md_buffer_kind::initStorageAccess(smoc_md_storage_access_snk<void,void> &storage_access){
  parent_type::initStorageAccess(storage_access);
};
        
template<>
void smoc_simple_md_buffer_kind::initStorageAccess(smoc_md_storage_access_src<void,void> &storage_access){
  parent_type::initStorageAccess(storage_access);
};
