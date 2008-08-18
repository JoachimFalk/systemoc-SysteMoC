//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_BUFFER_ANALYSIS_HPP
#define _INCLUDED_SMOC_MD_BUFFER_ANALYSIS_HPP

#include "smoc_md_loop.hpp"
#include <CoSupport/commondefs.h>

namespace smoc_md_ba 
{

  /// Common Interface for buffer analysis
  /// by simulation
  class smoc_md_buffer_analysis {

  public:
    typedef smoc_md_loop_iterator_kind::data_type sg_iter_type;
    typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

    // Data element Identification
    typedef smoc_md_loop_data_element_mapper::id_type id_type;
    typedef smoc_md_loop_data_element_mapper::data_element_id_type data_element_id_type;

  public:
    ///Constructor
    smoc_md_buffer_analysis(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
			    const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator
			    )
      : src_iterator_depth(src_md_loop_iterator.iterator_depth()),
	token_dimensions(src_md_loop_iterator.token_dimensions()),
	src_iteration_max(src_md_loop_iterator.iteration_max()),
	size_token_space(src_md_loop_iterator.size_token_space()),
	src_mapping_offset(src_md_loop_iterator.mapping_offset),
	// The first schedule period shall start with zero
	// In order to count correctly, we set the
	// schedule period initially to the highest possible value
	// such that the first invocation of consumption_update
	// resp. production_update provokes an overflow.
	src_schedule_period(MAX_TYPE(unsigned long)),
	snk_schedule_period(MAX_TYPE(unsigned long)),
	src_md_loop_iterator(src_md_loop_iterator),
	snk_md_loop_iterator(snk_md_loop_iterator)
    { 
      assert(src_md_loop_iterator.token_dimensions() == 
	     snk_md_loop_iterator.token_dimensions());
    }

    virtual ~smoc_md_buffer_analysis(){};

  public:

    /// These functions are the entrance function to the buffer analysis
    /// Important data is directly obtained by the loop-iterators
    /// from which a reference is stored.
    void consumption_update();
    void production_update();

    /// The next function dumps the results of the buffer analysis
    virtual void dump_results(std::ostream& os) const = 0;

  protected:

    /// This function is called whenever a data element is read the last time
    /// Input parameters:
    /// - current_iteration: describes the current iteration
    ///   ATTENTION: current_iteration is NOT a unique identifier for the iteration.
    ///              Instead, also schedule periods must be taken int account.
    /// - new_schedule_period is set to true, if current_iteration 
    ///   is the first of a new schedule period. This is necessary, because
    ///   SysteMoC does not use unique identifiers for the iterations. Other
    ///   wise an infinite counter would be required. Hence, current_iteration
    ///   will finally be reset to the zero-vector. This is indicated by the
    ///   the new_schedule_period_flag.
    /// - consumed_window_start, consumed_window_end:
    ///   These vectors describe the window iterations which belong to data elements
    ///   which are read the last time.
    ///   If the window_propagation delta_c is larger than the window extension c,
    ///   then these window iterators might show larger values than defined by c!
    virtual void consumption_update(const iter_domain_vector_type& current_iteration,
				    bool new_schedule_period,
				    const iter_domain_vector_type& consumed_window_start,
				    const iter_domain_vector_type& consumed_window_end
				    ) = 0;

    /// This function is called, when no data element is read the last time
    virtual void consumption_update(const iter_domain_vector_type& current_iteration,
				    bool new_schedule_period
				    ) = 0;

    /// This function is called whenever an effective token is produced
    /// Input parameters:
    /// - current_iteration: describes the current iteration of the source
    ///   ATTENTION: current_iteration is NOT a unique identifier for the iteration.
    ///              Instead, also schedule periods must be taken int account.
    /// - max_window_iteration: describes the maximum window iteration which is
    ///   possible for the current iteration
    /// - new_schedule_period: As SysteMoC does not use unique 
    ///   iteration identifiers, we indicate whenever a 
    ///   the counter is reset to zero.
    virtual void production_update(const iter_domain_vector_type& current_iteration,
				   const iter_domain_vector_type& max_window_iteration,
				   bool new_schedule_period) = 0;

  protected:

    /* WSDF parameters */

    //source iterator depth
    const unsigned int src_iterator_depth;

    //token dimensions
    const unsigned int token_dimensions;

    // Stores for each iteration level the 
    // maximum occuring value
    const iter_domain_vector_type src_iteration_max;

    // size of the token space
    const data_element_id_type size_token_space;

    typedef smoc_src_md_loop_iterator_kind::offset_type src_offset_type;
    typedef smoc_src_md_loop_iterator_kind::mapping_offset_type src_mapping_offset_type;

    // Stores the mapping offset for the source actor
    // (corresponds to the initial tokens)
    // ATTENTION: Due to theoretical reasons, the source-iteration (0,0,...)
    //            writes to data element (0,0,0, ...)
    //            In other words, the token space starts with negative coordinates
    //            This is different then in SysteMoC!
    const src_mapping_offset_type src_mapping_offset;


    /* Counter for schedule periods */
    unsigned long src_schedule_period;
    unsigned long snk_schedule_period;

  protected:
    /* Mapping functions to loop-iterators */ 
    inline bool
    get_src_loop_iteration(const data_element_id_type& src_data_el_id,
			   iter_domain_vector_type& iteration_vector,
			   id_type& schedule_period_offset
			   ) const {
      return 
	src_md_loop_iterator.get_src_loop_iteration(src_data_el_id,
						    iteration_vector,
						    schedule_period_offset
						    );

    }

    inline bool 
    is_snk_iteration_max(
			 unsigned token_dimension,
			 bool ignore_window_iteration = true
			 ) const {
      return snk_md_loop_iterator.is_iteration_max(token_dimension,
						   ignore_window_iteration);
    }

  protected:
    /* Elementary functions basing on loop iterators */
    
    /// This function determines the source iteration vector
    /// which belongs to the data element described by the 
    /// sink iteration. The latter one is specified by the
    /// window iteration and the implicitly given sink iteration
    /// vector.
    /// 
    /// The return-value is a UNIQUE iteration identifier correctly
    /// taking the schedule period into account.
    /// Furthermore, it takes into account the initial data elements.
    /// See comments on "src_mapping_offset"!!!!!!!!!!!!!!!!!!
    bool
    get_src_loop_iteration(const iter_domain_vector_type& window_iteration,
			   iter_domain_vector_type& iteration_vector
			   ) const;


  private:
    // The following references are stored in order to be able to
    // perform data element mapping and other important operations.
    // The references are declared as private in order to avoid,
    // that the buffer analysis operations access them directly,
    // so that the interface stays "clean". Instead, a protected
    // wrapper function shall be instantiated.
    const smoc_src_md_loop_iterator_kind& src_md_loop_iterator;
    const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator;
  
  };








  /// Initialization of the buffer analysis class must be performed
  /// quite deep in the SysteMoC, as we need access the the loop iterators.
  /// The following class provides a user interface class, by which the
  /// user can control the buffer analysis.
  class smoc_md_ba_user_interface {
  public:
    /// This function sets up the corresponding buffer analysis class
    /// and returns a corresponding pointer
    virtual smoc_md_buffer_analysis* 
    create_buffer_analysis_object(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
				  const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator) = 0;
  };



  

}

#endif //_INCLUDED_SMOC_MD_BUFFER_ANALYSIS_HPP
