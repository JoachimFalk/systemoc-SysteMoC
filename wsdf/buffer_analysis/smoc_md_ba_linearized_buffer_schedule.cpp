//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include "smoc_md_ba_linearized_buffer_schedule.hpp"
#include <cosupport/smoc_debug_out.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE
#define VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE 101
// 101: general debug
#endif

namespace smoc_md_ba 
{

  smoc_mb_ba_lin_buffer_schedule::smoc_mb_ba_lin_buffer_schedule(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
								 const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
								 unsigned int buffer_height,
								 const std::string& outfilename)
    : smoc_mb_ba_lin_buffer(src_md_loop_iterator,
			    snk_md_loop_iterator,
			    buffer_height),
      outfile(outfilename.c_str())
  {
    init_order_vector();

    //write dimensions of sink iterator
    outfile << src_md_loop_iterator.iterator_depth() << std::endl;
    outfile << src_md_loop_iterator.iteration_max() << std::endl;
  }

  smoc_mb_ba_lin_buffer_schedule::~smoc_mb_ba_lin_buffer_schedule(){
    outfile << std::endl;
    outfile.close();
    delete[] order_vector;
  }


  void 
  smoc_mb_ba_lin_buffer_schedule::consumption_update(const iter_domain_vector_type& current_iteration,
						     bool new_schedule_period,
						     const iter_domain_vector_type& consumed_window_start,
						     const iter_domain_vector_type& consumed_window_end
						     ){
#if VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE == 101
    CoSupport::dout << "Enter smoc_mb_ba_lin_buffer_schedule::consumption_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif

    //back-up previous lexicographically smallest data element
    iter_domain_vector_type 
      lexorder_smallest_life_data_element_old(lexorder_smallest_life_data_element);

    
    //update LOT
    parent_type::consumption_update(current_iteration,
				    new_schedule_period,
				    consumed_window_start,
				    consumed_window_end);

#if VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE == 101
    CoSupport::dout << "previous lexorder smallest life data element: ";
    CoSupport::dout << lexorder_smallest_life_data_element_old;
    CoSupport::dout << std::endl;

    CoSupport::dout << "current lexorder smallest life data element: ";
    CoSupport::dout << lexorder_smallest_life_data_element;
    CoSupport::dout << std::endl;
#endif



    //calculate number of invocations
    unsigned long num_invocations = 
      calc_num_src_invocations(lexorder_smallest_life_data_element_old,
			       lexorder_smallest_life_data_element);

    //Write to result file
    outfile << num_invocations << " ";
#if VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE == 101
    CoSupport::dout << "num_invocations = " << num_invocations << std::endl;
#endif

#if 0
    for(unsigned int i = 0; i < token_dimensions; i++){
      if (is_snk_iteration_max(i))
	outfile << std::endl;
      else
	break;
    }
#endif

#if VERBOSE_LEVEL_SMOC_MD_BA_LIN_BUF_SCHEDULE == 101
    CoSupport::dout << "Leave smoc_mb_ba_lin_buffer_schedule::consumption_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    
    
  }


  void 
  smoc_mb_ba_lin_buffer_schedule::consumption_update(const iter_domain_vector_type& current_iteration,
						     bool new_schedule_period
						     ){
    //For this sink iteration, no data element 
    //has been consumed the last time

    //Write to result file
    outfile << 0 << " ";

#if 0
    for(unsigned int i = 0; i < token_dimensions; i++){
      if (is_snk_iteration_max(i))
	outfile << std::endl;
      else
	break;
    }
#endif
    
  }

  void 
  smoc_mb_ba_lin_buffer_schedule::production_update(const iter_domain_vector_type& current_iteration,
						    const iter_domain_vector_type& max_window_iteration,
						    bool new_schedule_period){
    // do nothing
  }

  void 
  smoc_mb_ba_lin_buffer_schedule::init_order_vector(){
    order_vector = new unsigned long[src_iterator_depth-token_dimensions];

    order_vector[src_iterator_depth-token_dimensions-1] = 1;
    for(int i = src_iterator_depth-token_dimensions-2; i >= 0; i--){
      order_vector[i] = order_vector[i+1] * (src_iteration_max[i+1]+1);
    }
  }

  unsigned long 
  smoc_mb_ba_lin_buffer_schedule::calc_num_src_invocations(const iter_domain_vector_type& previous_src_iter,
							   const iter_domain_vector_type& current_src_iter
							   ) const{
    long return_value = 0;

    for(unsigned int i = 0; i < src_iterator_depth-token_dimensions; i++){
      return_value +=
	((long)current_src_iter[i] - (long)previous_src_iter[i]) * (long)order_vector[i];
    }

    assert(return_value >= 0);

    return (unsigned long)return_value;
  }












  /* ******************************************************************** */
  /*                            User Interface                            */
  /* ******************************************************************** */
  smoc_md_buffer_analysis*
  smoc_md_ba_ui_schedule::create_buffer_analysis_object(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
							const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator) const {

    unsigned int buffer_height = // 1;
      src_md_loop_iterator.iteration_max()[0];

    smoc_md_buffer_analysis* return_value = 
      new smoc_mb_ba_lin_buffer_schedule(src_md_loop_iterator,
					 snk_md_loop_iterator,
					 buffer_height,
					 output_file
					 );

    return return_value;
  }


};
