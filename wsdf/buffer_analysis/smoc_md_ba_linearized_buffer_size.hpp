//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:


#ifndef _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_SIZE_HPP
#define _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_SIZE_HPP

#include "smoc_md_ba_linearized_buffer.hpp"


namespace smoc_md_ba 
{

  /// This class can be used to calculate a self timed schedule
  class smoc_mb_ba_lin_buffer_size
    : public smoc_mb_ba_lin_buffer
  {

  public:
    smoc_mb_ba_lin_buffer_size(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
			       const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
			       unsigned int buffer_height = 1);
    virtual ~smoc_mb_ba_lin_buffer_size();

  public:

    void production_update(const iter_domain_vector_type& current_iteration,
			   const iter_domain_vector_type& max_window_iteration,
			   bool new_schedule_period);

  private:
    // number of buffer elements
    long number_buffer_elements;

    // the following table is intended to speed up the two following functions
    long *address_factors;
    
    // initialise the address factor table
    virtual void init_address_factors_table(void);

    // returns the linearized buffer address (without modulo operation!)
    // It is assumed, that the abstract_id already contains 
    // the schedule period offset!!
    long calc_linear_buffer_address(const iter_domain_vector_type& abstract_id) const;
    // same as above, but with separate window iteration
    // Furthermore, the schedule period is explicitly taken into account!
    long calc_linear_buffer_address(const iter_domain_vector_type& snk_iteration,
				    const iter_domain_vector_type& window_iteration
				    ) const;

    // calculates the buffer address including modulo operation
    long calc_modulo_buffer_address(const iter_domain_vector_type& abstract_id, 
				    unsigned long buffer_size) const;
    
  };

};



#endif
