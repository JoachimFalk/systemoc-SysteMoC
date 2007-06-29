//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include "smoc_md_buffer_analysis.hpp"

#include <cosupport/smoc_debug_out.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BA
#define VERBOSE_LEVEL_SMOC_MD_BA 101
#endif


namespace smoc_md_ba {


  void smoc_md_buffer_analysis::consumption_update(){

#if VERBOSE_LEVEL_SMOC_MD_BA == 101
    CoSupport::dout << "Enter smoc_md_buffer_analysis::consumption_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif

    if (snk_md_loop_iterator.is_new_schedule_period()){
      snk_schedule_period++;
    }

    /* Determine the pixels of the current window which are read the last time */
    iter_domain_vector_type consumed_window_start(token_dimensions);
    iter_domain_vector_type consumed_window_end(token_dimensions);

    bool consumed = 
      snk_md_loop_iterator.calc_consumed_window_iterations(src_md_loop_iterator.schedule_period_max_data_element_id(),
							   consumed_window_start,
							   consumed_window_end
							   );

    if (consumed)
      consumption_update(snk_md_loop_iterator.iteration_vector(),
			 snk_md_loop_iterator.is_new_schedule_period(),
			 consumed_window_start,
			 consumed_window_end
			 );
    else
      // no data element read the last time
      consumption_update(snk_md_loop_iterator.iteration_vector(),
			 snk_md_loop_iterator.is_new_schedule_period()
			 );
#if VERBOSE_LEVEL_SMOC_MD_BA == 101
    CoSupport::dout << "Leave smoc_md_buffer_analysis::consumption_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif		       
		       
  }

  void smoc_md_buffer_analysis::production_update(){

    if (src_md_loop_iterator.is_new_schedule_period()){
      src_schedule_period++;
    }

    production_update(src_md_loop_iterator.iteration_vector(),
		      src_md_loop_iterator.max_window_iteration(),
		      src_md_loop_iterator.is_new_schedule_period());
		      
  }
  

  bool
  smoc_md_buffer_analysis::get_src_loop_iteration(const iter_domain_vector_type& window_iteration,
						  iter_domain_vector_type& iteration_vector
						  ) const {
#if VERBOSE_LEVEL_SMOC_MD_BA == 101
    CoSupport::dout << "Enter smoc_md_buffer_analysis::get_src_loop_iteration" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;

    CoSupport::dout << "window_iteration = " << window_iteration << std::endl;

    CoSupport::dout << "snk_schedule_period = " << snk_schedule_period << std::endl;
#endif

    bool return_value;
    
    id_type schedule_period_offset;
      
    data_element_id_type data_element(token_dimensions);
    snk_md_loop_iterator.get_window_data_element_offset(window_iteration,
							data_element);

    data_element += 
      snk_md_loop_iterator.get_base_data_element_id();

    return_value =  
      src_md_loop_iterator.get_src_loop_iteration(data_element,
						  iteration_vector,
						  schedule_period_offset);
    
    //make iteration-vector unique
    schedule_period_offset += snk_schedule_period;
    iteration_vector[0] += 
      schedule_period_offset * (src_iteration_max[0]+1);

    
    

#if VERBOSE_LEVEL_SMOC_MD_BA == 101
    CoSupport::dout << "iteration_vector = " << iteration_vector << std::endl;
    CoSupport::dout << "Leave smoc_md_buffer_analysis::get_src_loop_iteration" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    
    return return_value;
  }














};
