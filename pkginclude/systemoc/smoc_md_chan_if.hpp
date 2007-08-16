//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_CHAN_IF_HPP
#define _INCLUDED_SMOC_MD_CHAN_IF_HPP

#include "smoc_md_loop.hpp"


/// Interface for multi-dimensional channel access
/// This interface is used by the multi-dimensional ports
/// in order to access the data stored in the FIFO
template<class T>
class smoc_md_src_channel_access {
public:
                
  typedef T  return_type; 

  //Iteration vector
  typedef smoc_md_loop_iterator_kind::data_type iteration_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;
                
public:
  
#ifndef NDEBUG          
  virtual void setLimit(size_t limit) = 0;
#endif
    
  /* Data Element Access */
  virtual return_type operator[](const iter_domain_vector_type& window_iteration) = 0;
  virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const = 0;

  /// Returns the value of the loop iterator for the given iteration level
  virtual iteration_type iteration(size_t iteration_level) const = 0;
};

template<class T>
class smoc_md_snk_channel_access {
public:
                
  typedef T  return_type; 

  //Iteration vector
  typedef smoc_md_loop_iterator_kind::data_type iteration_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  //Border type
  typedef smoc_snk_md_loop_iterator_kind::border_condition_vector_type border_condition_vector_type;
  typedef smoc_snk_md_loop_iterator_kind::border_type border_type;
  typedef smoc_snk_md_loop_iterator_kind::border_type_vector_type border_type_vector_type;
                
public:
  
#ifndef NDEBUG          
  virtual void setLimit(size_t limit) = 0;
#endif
    
  /* Data Element Access */
  virtual return_type operator[](const iter_domain_vector_type& window_iteration) = 0;
  virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const = 0;

  /// Returns the value of the loop iterator for the given iteration level
  virtual iteration_type iteration(size_t iteration_level) const = 0;
                
  /// Check, whether data element is situated on extended border
  virtual border_type_vector_type is_ext_border(const iter_domain_vector_type& window_iteration,
						bool& is_border) const = 0;                
};


#endif
