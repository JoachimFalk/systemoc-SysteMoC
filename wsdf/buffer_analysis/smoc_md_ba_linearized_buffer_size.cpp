//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include "smoc_md_ba_linearized_buffer_size.hpp"

#ifndef SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL
# define SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL 0
#endif


namespace smoc_md_ba 
{


  smoc_mb_ba_lin_buffer_size::smoc_mb_ba_lin_buffer_size(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
							 const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
							 unsigned int buffer_height)
    : smoc_mb_ba_lin_buffer(src_md_loop_iterator,
			    snk_md_loop_iterator,
			    buffer_height),
      number_buffer_elements(0)
  {

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL >= 1
    CoSupport::dout << "Initialize data structures ..." << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;;
#endif

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "init address factor table" << std::endl;
#endif

    //init table for address calculation
    init_address_factors_table();
    
    
    //Update buffer size (for the case where initial tokens are existing)
    const sg_iter_type zero = 0;
    production_update(iter_domain_vector_type(src_iterator_depth,zero),
		      iter_domain_vector_type(token_dimensions,zero),
		      true);
    //as zero element is not already produced
    number_buffer_elements--;
	
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "Initial buffer size: " << number_buffer_elements << std::endl;
#endif

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL >= 1
    CoSupport::dout << CoSupport::Indent::Down;;
#endif

    
  }

  smoc_mb_ba_lin_buffer_size::~smoc_mb_ba_lin_buffer_size(){
    delete[] address_factors;
  }


  void 
  smoc_mb_ba_lin_buffer_size::production_update(const iter_domain_vector_type& current_iteration,
						const iter_domain_vector_type& max_window_iteration,
						bool new_schedule_period){
    
    //we only have to analyse the lexicographically largest data element
    //this is exactly that one given in last_produced_data_element_id

    long current_buffer_size;

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "Update of buffer size due to production" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;;    
#endif

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "Current iteration: " << current_iteration << std::endl;
    CoSupport::dout << "Current src-schedule period: " 
                    << src_schedule_period 
                    << std::endl;
    CoSupport::dout << "Lexicographically smallest data element: ";
    CoSupport::dout << lexorder_smallest_life_data_element << std::endl;
#endif

    current_buffer_size = 
      calc_src_linear_buffer_address(current_iteration,max_window_iteration)
      - calc_linear_buffer_address(lexorder_smallest_life_data_element)+1;

#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "calc_src_linear_buffer_address(current_iteration,max_window_iteration) = "
                    << calc_src_linear_buffer_address(current_iteration,max_window_iteration)
                    << std::endl;
    CoSupport::dout << "calc_linear_buffer_address(lexorder_smallest_life_data_element) = "
                    << calc_linear_buffer_address(lexorder_smallest_life_data_element)
                    << std::endl;
    CoSupport::dout << "Current buffer size: ";
    CoSupport::dout << current_buffer_size << std::endl;
#endif


  //calculate minimal required buffer size
    if (current_buffer_size > number_buffer_elements){
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
      CoSupport::dout << "update buffer size from " << number_buffer_elements;
      CoSupport::dout << " to " << current_buffer_size << std::endl;
#endif
      number_buffer_elements = current_buffer_size;
    }
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << CoSupport::Indent::Down;;
#endif
  }


  void
  smoc_mb_ba_lin_buffer_size::init_address_factors_table(){
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << "Initialisation of address factor table" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;;
#endif
    address_factors = new long[src_iterator_depth];
    
    for(unsigned int i = 1; i <= src_iterator_depth; i++){
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
      CoSupport::dout << "i = " << i << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
#endif
      address_factors[i-1] = 1;
      for(unsigned int j = i+1; j <= src_iterator_depth; j++){
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
      CoSupport::dout << "j = " << j << std::endl;
      CoSupport::dout << "src_iteration_max[j-1] = " 
                      << src_iteration_max[j-1]
                      << std::endl;
#endif
      address_factors[i-1] *= (src_iteration_max[j-1]+1);
      }
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
      CoSupport::dout << "i=" << i << " : " << address_factors[i-1] << std::endl;
      CoSupport::dout << CoSupport::Indent::Down;
#endif
    }
#if SMOC_MD_BA_LIN_BUFFER_SIZE_VERBOSE_LEVEL == 105
    CoSupport::dout << CoSupport::Indent::Down;;
#endif
  }
  
  
  long 
  smoc_mb_ba_lin_buffer_size::calc_linear_buffer_address(const iter_domain_vector_type& abstract_id) const {
    long return_value=0;
  
    for(unsigned int i=0; i < src_iterator_depth; i++){
      return_value += address_factors[i] * abstract_id[i];
    }

    return return_value;
  }

  long 
  smoc_mb_ba_lin_buffer_size::calc_src_linear_buffer_address(const iter_domain_vector_type& src_iteration,
                                                             const iter_domain_vector_type& window_iteration
                                                             ) const {
    long return_value = 0;
    
    for(unsigned int i = 0; i < src_iterator_depth - token_dimensions; i++){
      return_value += address_factors[i] * src_iteration[i];
    }

    for(unsigned int i = 0; i < token_dimensions; i++){
      return_value += 
	address_factors[i+src_iterator_depth-token_dimensions] * window_iteration[i];
    }


    //take schedule period into account
    return_value += 
      address_factors[0] * src_schedule_period * (src_iteration_max[0]+1);

    return return_value;
  }

  
  long 
  smoc_mb_ba_lin_buffer_size::calc_modulo_buffer_address(const iter_domain_vector_type& abstract_id, 
							 unsigned long buffer_size) const {
    long return_value;

    return_value = calc_linear_buffer_address(abstract_id);
    return_value = REM(return_value,buffer_size);

    return return_value;
  }



  /* ******************************************************************** */
  /*                            User Interface                            */
  /* ******************************************************************** */
  smoc_md_buffer_analysis*
  smoc_md_ba_ui_lin_buffer_size::create_buffer_analysis_object(
    const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
    const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator)
  {
    unsigned int buffer_height = // 1;
      src_md_loop_iterator.iteration_max()[0];
    
    smoc_mb_ba_lin_buffer_size* ba_obj = 
      new smoc_mb_ba_lin_buffer_size(src_md_loop_iterator,
                                     snk_md_loop_iterator,
                                     buffer_height
                                     );
    
    //store pointer for further use
    this->ba_obj = ba_obj;
    
    return ba_obj;
  }



};
