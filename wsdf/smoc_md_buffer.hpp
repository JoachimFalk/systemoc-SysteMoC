//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_BUFFER_HPP
#define _INCLUDED_SMOC_MD_BUFFER_HPP

#include <CoSupport/commondefs.h>
#include <CoSupport/Streams/DebugOStream.hpp>

#include <wsdf/smoc_vector.hpp>
#include "smoc_md_loop.hpp"
#include <wsdf/smoc_md_array.hpp>
#include "smoc_md_chan_if.hpp"
#include "smoc_pggen.hpp"
#include <wsdf/smoc_wsdf_edge.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BUFFER
#define VERBOSE_LEVEL_SMOC_MD_BUFFER 0
///101: operator[]
///102: border processing
///103: memory access error
///104: iteration()
#endif

/// This class represents the base class for all SMOC buffer models.
/// It is responsible for buffer management.
class smoc_md_buffer_mgmt_base {

public:
        
  /// Declaration of storage type
  template <typename DATA_TYPE>
  class smoc_md_storage_type {
  public:
    typedef void type;
  };
        
  template <typename DATA_TYPE>
  class smoc_md_storage_type<const DATA_TYPE> {
  public:
    typedef const void type;
  };

public:
  typedef smoc_src_md_loop_iterator_kind::data_element_id_type data_element_id_type;        
public:
        
  /// This class represents the common base class for access of
  /// data elements in a multi-dimensional buffer. Border processing
  /// is not performed.
  template<class S, class T>
  class smoc_md_storage_access_src
    : public smoc_md_src_port_access_if<T>
  {
    friend class smoc_md_buffer_mgmt_base;
  public:
    typedef smoc_md_storage_access_src<S,T> this_type;
    typedef smoc_md_src_port_access_if<T> parent_type;

    typedef typename parent_type::iteration_type iteration_type;
    typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;
                
    typedef S                                             storage_type;
    typedef T                                             return_type;


                
  public:
                
    smoc_md_storage_access_src()
      : buffer(NULL),
	src_loop_iterator(NULL)
    {}

    virtual ~smoc_md_storage_access_src(){};
                
  public:
                
    /* Buffer Access Setup Routines */
#if defined(SYSTEMOC_ENABLE_DEBUG)          
    /// Set limit
    /// Dummy function
   void setLimit(size_t limit) {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
     CoSupport::Streams::dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_src::setLimit"
		     << std::endl;
     CoSupport::Streams::dout << CoSupport::Indent::Up;
     CoSupport::Streams::dout << "limit = " << limit << std::endl;
#endif
     assert(limit <= 1);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
     CoSupport::Streams::dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_src::setLimit"
		     << std::endl;
     CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
   };
#endif
    
    /// Set buffer pointer
    virtual void SetBuffer(typename smoc_md_buffer_mgmt_base::smoc_md_storage_type<storage_type>::type *storage){};

    /* Data Element Access */
    virtual return_type operator[](const iter_domain_vector_type& window_iteration) { 
      assert(false); 
    }
    virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const { 
      assert(false); 
    }

    /// Returns the value of the loop iterator for the given iteration level
    virtual iteration_type iteration(size_t iteration_level) const {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 104
      CoSupport::Streams::dout << buffer->name() << ": Enter smoc_md_storage_access_src::iteration" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
      CoSupport::Streams::dout << "iteration-level = " << iteration_level << std::endl;
#endif

      iteration_type return_value = (*src_loop_iterator)[iteration_level];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 104
      CoSupport::Streams::dout << "return_value = " << return_value << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 104
      CoSupport::Streams::dout << "Leave smoc_md_storage_access_src::iteration" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
      return return_value;
    }
                
                
  protected:

    smoc_md_buffer_mgmt_base *buffer;
                
    const smoc_src_md_loop_iterator_kind* src_loop_iterator;

    void checkLimit(const iter_domain_vector_type& window_iteration) const{
#if defined(SYSTEMOC_ENABLE_DEBUG)
      const iter_domain_vector_type& max_window_iteration(src_loop_iterator->max_window_iteration());
      for(unsigned i = 0; i < window_iteration.size(); i++){
	assert(window_iteration[i] <= 
	       max_window_iteration[i]);
      }
#endif
    }
  };
        
  template<class S, class T>
  class smoc_md_storage_access_snk
    : public smoc_md_snk_port_access_if<T>
  {
    friend class smoc_md_buffer_mgmt_base;
  public:

    typedef smoc_md_storage_access_snk<S,T> this_type;
    typedef smoc_md_snk_port_access_if<T> parent_type;
    
    typedef typename parent_type::iteration_type iteration_type;
    typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;


    typedef typename parent_type::border_condition_vector_type border_condition_vector_type;
    typedef typename parent_type::border_type border_type;
    typedef typename parent_type::border_type_vector_type border_type_vector_type;
                
    typedef S                                             storage_type;
    typedef T                                             return_type;
                
  public:
                
    smoc_md_storage_access_snk()
      : buffer(NULL),
	snk_loop_iterator(NULL)
    {}

    virtual ~smoc_md_storage_access_snk(){};
                
  public:
                
    /* Buffer Access Setup Routines */
#if defined(SYSTEMOC_ENABLE_DEBUG)          
    /// Set limit, how many windows can be accessed
    /// dummy function
    void setLimit(size_t limit) {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::setLimit"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
      CoSupport::Streams::dout << "limit = " << limit << std::endl;
#endif
      assert(limit <= 1);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::setLimit"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif
    };
#endif
                
    /// Set buffer pointer
    virtual void SetBuffer(typename smoc_md_buffer_mgmt_base::smoc_md_storage_type<storage_type>::type *storage){};
                
    /* Data Element Access */
    virtual return_type operator[](const iter_domain_vector_type& window_iteration) { assert(false); }
    virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const { assert(false); }

    /// Returns the value of the loop iterator for the given iteration level
    virtual iteration_type iteration(size_t iteration_level) const {
      return (*snk_loop_iterator)[iteration_level];
    }

    /// Returns the maximum window iteration
    virtual const iter_domain_vector_type& max_window_iteration() const {
      return snk_loop_iterator->max_window_iteration();
    }

    /// Check, whether data element is situated on extended border
    virtual border_type_vector_type is_ext_border(const iter_domain_vector_type& window_iteration,
						  bool& is_border) const { 
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
      CoSupport::Streams::dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::is_ext_border" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

      border_type_vector_type 
        return_vector(snk_loop_iterator->is_ext_border(window_iteration,
                                                       is_border));

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
      CoSupport::Streams::dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::is_ext_border" << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

      return return_vector;
                        
    }
                
  protected:

    smoc_md_buffer_mgmt_base *buffer;
                
    const smoc_snk_md_loop_iterator_kind* snk_loop_iterator;

    virtual void checkLimit(const iter_domain_vector_type& window_iteration) const{
#if defined(SYSTEMOC_ENABLE_DEBUG)
      const iter_domain_vector_type& max_window_iteration(snk_loop_iterator->max_window_iteration());
      for(unsigned i = 0; i < window_iteration.size(); i++){
	assert(window_iteration[i] <= 
	       max_window_iteration[i]);
      }
#endif
    }

    virtual void 
    get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
                                   data_element_id_type& data_element_id) const {
      this->snk_loop_iterator->get_window_data_element_offset(window_iteration,
							      data_element_id);
    };

  };
                
        
public:
        
  /// buffer init
  class buffer_init {
    //friend class smoc_md_buffer_mgmt_base;
  public:
    typedef smoc_md_loop_data_element_mapper::mapping_matrix_type mapping_matrix_type;
    typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;
    typedef smoc_src_md_loop_iterator_kind::mapping_offset_type src_mapping_offset_type;
    typedef smoc_snk_md_loop_iterator_kind::mapping_offset_type snk_mapping_offset_type;
    typedef smoc_snk_md_loop_iterator_kind::border_condition_matrix_type border_condition_matrix_type;
    typedef smoc_snk_md_loop_iterator_kind::border_condition_vector_type  border_condition_vector_type;
                
  public:
    const iter_domain_vector_type& src_iteration_max() const {
      return _wsdf_edge_descr.src_iteration_max();
    }
    const mapping_matrix_type      src_mapping_matrix() const {
      return _wsdf_edge_descr.src_data_element_mapping_matrix();
    }
    const src_mapping_offset_type src_mapping_offset() const {
      return _wsdf_edge_descr.src_data_element_mapping_vector();
    }
    
    const iter_domain_vector_type& snk_iteration_max() const {
      return _wsdf_edge_descr.snk_iteration_max();
    }
    const mapping_matrix_type     snk_mapping_matrix() const {
      return _wsdf_edge_descr.snk_data_element_mapping_matrix();
    }
    const snk_mapping_offset_type snk_mapping_offset() const {
      return _wsdf_edge_descr.snk_data_element_mapping_vector();
    }
    
    const border_condition_matrix_type snk_border_matrix() const {
      return _wsdf_edge_descr.calc_border_condition_matrix();
    }
    const border_condition_vector_type snk_low_border_vector() const{
      return _wsdf_edge_descr.calc_low_border_condition_vector();
    }
    const border_condition_vector_type snk_high_border_vector() const{
      return _wsdf_edge_descr.calc_high_border_condition_vector();
    }

    const smoc_wsdf_edge_descr& wsdf_edge_params() const{
      return _wsdf_edge_descr;
    }

  private:

    const smoc_wsdf_edge_descr _wsdf_edge_descr;

  public:
    buffer_init(const smoc_wsdf_edge_descr wsdf_edge_descr)
      : _wsdf_edge_descr(wsdf_edge_descr)
    {}
  };


public:
  smoc_md_buffer_mgmt_base(const buffer_init& i)
    : src_loop_iterator(
			i.src_iteration_max(),
			i.src_mapping_matrix(),
			i.src_mapping_offset()
			),
      snk_loop_iterator(
			i.snk_iteration_max(),
			i.snk_mapping_matrix(),
			i.snk_mapping_offset(),
			i.snk_border_matrix(),
			i.snk_low_border_vector(),
			i.snk_high_border_vector()
			),                      
      _token_dimensions(src_loop_iterator.token_dimensions()),
      size_token_space(src_loop_iterator.size_token_space()),
      wsdf_edge_params(i.wsdf_edge_params())
  {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Enter smoc_md_buffer_mgmt_base::smoc_md_buffer_mgmt_base(const buffer_init& i)" << std::endl;
#endif
    assert(src_loop_iterator.token_dimensions() == 
	   snk_loop_iterator.token_dimensions());
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_buffer_mgmt_base(const buffer_init& i)" << std::endl;
#endif
  };

  virtual ~smoc_md_buffer_mgmt_base(){}


public:
  /// This function allocates the memory for the next source iteration
  /// The next source iteration is this one which has been passed
  /// previously to unusedStorage.
  /// (allocate_buffer must only be called when we are sure, that 
  ///  the buffer can be allocated). Hence, a previous call to
  ///  unusedStorage is required.
  virtual void allocate_buffer() = 0;

  /// This function is called, when the source actor has finished writing
  /// to the memory zone allocated by "allocate_buffer()".
  virtual void release_buffer() = 0;

  /// Free the memory read the last time by the current sink iteration
  virtual void free_buffer() = 0;

  ///Checks, wether buffer for the next iteration can be allocated
  virtual bool hasUnusedStorage() const = 0;

  /// Init storage access
  template<class S, class T>
  void initStorageAccess(smoc_md_storage_access_src<S,T> &storage_access){
    storage_access.buffer = this;
    storage_access.src_loop_iterator = &src_loop_iterator;
  };

  template<class S, class T>
  void initStorageAccess(smoc_md_storage_access_snk<S,T> &storage_access){
    storage_access.buffer = this;
    storage_access.snk_loop_iterator = &snk_loop_iterator;
  };

  /// Create buffer (reserve memory)
  template <typename BUFFER_TYPE>
  void createStorage(BUFFER_TYPE *& ptr) const{
    ptr = NULL;
  }

  /// Destroy storage
  template <typename BUFFER_TYPE>
  void destroyStorage(BUFFER_TYPE *& ptr) const{
    //do nothing
  }

public:
  //must be mapped by the FIFO to the FIFO name
  virtual const char *name() const = 0;   

public:
  /// Wrapper functions
  inline bool src_new_schedule_period() const {
    return src_loop_iterator.is_new_schedule_period();
  }

  inline bool snk_new_schedule_period() const {
    return snk_loop_iterator.is_new_schedule_period();
  }

public:
  /* Functions for problem graph generation */
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const;

protected:
  /// Current source and sink iteration vectors.
  /// They specify, which iteration is executed NEXT.   
  smoc_src_md_static_loop_iterator src_loop_iterator;
  smoc_snk_md_static_loop_iterator snk_loop_iterator;

  const unsigned _token_dimensions;

  typedef smoc_src_md_static_loop_iterator::id_type id_type;

// Conflicts with typedef in line 44 (GCC 4.1.2)
//typedef smoc_src_md_static_loop_iterator::data_element_id_type data_element_id_type;

  const data_element_id_type size_token_space;

private:
  // Original WSDF data. Only for SGX graph dumper (formerly problem graph dumper)
  const smoc_wsdf_edge_descr wsdf_edge_params;

  
};













/// Simple multi-dimensional FIFO for simulation in SysteMoC
/// A very simple buffer model is used avoiding memory sharing whenever
/// possible. Only in the highest dimension, a ring buffer concept has to
/// be deployed, because the data flow graph processes infinite streams of 
/// data.
class smoc_simple_md_buffer_kind
  : public smoc_md_buffer_mgmt_base
{

public:
  template<class S, class T>
  class smoc_md_storage_access_src;
        
  template<class S, class T>
  class smoc_md_storage_access_snk;

  template<class S, class T> friend class smoc_md_storage_access_src;
  template<class S, class T> friend class smoc_md_storage_access_snk;

public:

  /// Declaration of storage type
  template <typename DATA_TYPE>
  class smoc_md_storage_type {
  public:
    typedef smoc_md_array<DATA_TYPE> type;
  };

  template <typename DATA_TYPE>
  class smoc_md_storage_type<const DATA_TYPE> {
  public:
    typedef const smoc_md_array<DATA_TYPE> type;
  };

public:  
  typedef smoc_md_buffer_mgmt_base parent_type;
  typedef parent_type::data_element_id_type data_element_id_type;

public:

  template<class S, class T>
  class smoc_md_storage_access_src
    : public smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<S,T>
  {
    friend class smoc_simple_md_buffer_kind;
  public:
    typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<S,T> parent_type;
    typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;
                
    typedef S                                             storage_type;
    typedef T                                             return_type;

  public:
    smoc_md_storage_access_src()
      : smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<S,T>(),
	storage(NULL),
	simple_md_buffer(NULL)
    {}

    virtual ~smoc_md_storage_access_src(){};

  public:

    virtual void SetBuffer(typename smoc_md_storage_type<storage_type>::type *storage){
      this->storage = storage;
    }
                
    /* Data Element Access */
    virtual return_type operator[](const iter_domain_vector_type& window_iteration){
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_src::operator[]"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

      checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "window_iteration = " << window_iteration;
      CoSupport::Streams::dout << std::endl;
#endif
      //Allocate the memory for the current source iteration.
      simple_md_buffer->allocate_buffer();

      unsigned token_dimensions = (*this).src_loop_iterator->token_dimensions();

      const data_element_id_type& base_data_element_id = 
	(*this).src_loop_iterator->get_base_data_element_id();
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "base_data_element_id = " << base_data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      data_element_id_type data_element_id(token_dimensions);                 
      this->src_loop_iterator->get_window_data_element_offset(window_iteration,
							      data_element_id);
                        
      data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "data_element_id = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      data_element_id[token_dimensions-1] += 
	(*simple_md_buffer).wr_schedule_period_start;
      data_element_id[token_dimensions-1] = 
	data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Array element = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      return_type return_value((*storage)[data_element_id]);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_src::operator[]"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

      return return_value;

    }               
                
  private:
    typename smoc_md_storage_type<storage_type>::type *storage;
    smoc_simple_md_buffer_kind* simple_md_buffer;

  protected:                
    
  };  


  template<class S, class T>
  class smoc_md_storage_access_snk
    : public smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<S,T>
  {
    friend class smoc_simple_md_buffer_kind;
  public:
    typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<S,T> parent_type;
    typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;
                
    typedef S                                             storage_type;
    typedef T                                             return_type;

    typedef smoc_snk_md_loop_iterator_kind::border_condition_vector_type border_condition_vector_type;
    typedef smoc_snk_md_loop_iterator_kind::border_type border_type;

  public:
    smoc_md_storage_access_snk()
      : smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<S,T>(),
	storage(NULL),
	simple_md_buffer(NULL)
    {}

    virtual ~smoc_md_storage_access_snk(){};

  public:

    virtual void SetBuffer(typename smoc_md_storage_type<storage_type>::type *storage){
      this->storage = storage;
    }
                
    /* Data Element Access */
    virtual return_type operator[](const iter_domain_vector_type& window_iteration){
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

      checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "window_iteration = " << window_iteration;
      CoSupport::Streams::dout << std::endl;
#endif

      const unsigned token_dimensions = 
        (*this).snk_loop_iterator->token_dimensions();

      const data_element_id_type& base_data_element_id = 
	(*this).snk_loop_iterator->get_base_data_element_id();

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "base_data_element_id = " << base_data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      data_element_id_type data_element_id(token_dimensions);                 
      this->get_window_data_element_offset(window_iteration,
                                           data_element_id);
      data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "data_element_id = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      data_element_id[token_dimensions-1] += 
	(*simple_md_buffer).rd_schedule_period_start;
      data_element_id[token_dimensions-1] = 
	data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Array element = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      const return_type return_value = 
        (*storage)[data_element_id];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
                               << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

      return return_value;
    }
                
    virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const{
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
		      << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

      checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "window_iteration = " << window_iteration;
      CoSupport::Streams::dout << std::endl;
#endif

      const unsigned token_dimensions = 
        (*this).snk_loop_iterator->token_dimensions();

      const data_element_id_type& base_data_element_id = 
	(*this).snk_loop_iterator->get_base_data_element_id();

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "base_data_element_id = " << base_data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif


      data_element_id_type data_element_id(token_dimensions);
      this->get_window_data_element_offset(window_iteration,
                                           data_element_id);
      data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "data_element_id = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      data_element_id[token_dimensions-1] += 
	(*simple_md_buffer).rd_schedule_period_start;
      data_element_id[token_dimensions-1] = 
	data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Array element = " << data_element_id;
      CoSupport::Streams::dout << std::endl;
#endif

      const return_type return_value = 
        (*storage)[data_element_id];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
      CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
                               << std::endl;
      CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif

      return return_value;
                        
    }
                
                
  private:
    typename smoc_md_storage_type<storage_type>::type *storage;
    const smoc_simple_md_buffer_kind* simple_md_buffer;
                
  };


public:

  /// Buffer init
  class buffer_init 
    : public smoc_md_buffer_mgmt_base::buffer_init
  {
    friend class smoc_simple_md_buffer_kind;
  public:
    typedef smoc_md_buffer_mgmt_base::buffer_init parent_type;
    typedef parent_type::mapping_matrix_type mapping_matrix_type;
    typedef parent_type::iter_domain_vector_type iter_domain_vector_type;
    typedef parent_type::src_mapping_offset_type src_mapping_offset_type;
    typedef parent_type::snk_mapping_offset_type snk_mapping_offset_type;
    typedef parent_type::border_condition_matrix_type border_condition_matrix_type;
    typedef parent_type::border_condition_vector_type border_condition_vector_type;
                
  private:
    const size_t buffer_lines;
  public:
    buffer_init(const smoc_wsdf_edge_descr wsdf_edge_descr,                                                                
		const size_t buffer_lines = MAX_TYPE(size_t)
		)
      : parent_type(wsdf_edge_descr),
	buffer_lines(buffer_lines)
    {};
  };

public:
  smoc_simple_md_buffer_kind(const buffer_init& i)
    : smoc_md_buffer_mgmt_base(i),
      buffer_lines(i.buffer_lines),
      rd_schedule_period_start(0),
      rd_min_data_element_offset(0),
      cache_unusedStorage(i.buffer_lines == MAX_TYPE(size_t) ? true : false)
                        
  {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Enter smoc_simple_md_buffer_kind::smoc_simple_md_buffer_kind" << std::endl;
    CoSupport::Streams::dout << CoSupport::Indent::Up;
#endif

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Check initial data elements" << std::endl;
#endif
    //currently, we only support initial data elements in
    //the highest token dimension
    for(unsigned int i = 0; i < _token_dimensions-1; i++){
      assert(src_loop_iterator.mapping_offset[i] == 0);
    }

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Calculate initial schedule period start" << std::endl;
#endif
    // wr_schedule_period_start = k* buffer_lines - size_token_space[_token_dimensions-1]
    // whereas k the smallest possible integer, such that wr_schedule_period_start > 0
    wr_schedule_period_start = buffer_lines - 
      (size_token_space[_token_dimensions-1] % buffer_lines);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Calculate initial wr_max_data_element_offset" << std::endl;
#endif
    wr_max_data_element_offset = 
      size_token_space[_token_dimensions-1] + 
      src_loop_iterator.mapping_offset[_token_dimensions - 1] - 1;

                
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Check mapping offset" << std::endl;
#endif
    assert(src_loop_iterator.mapping_offset[_token_dimensions - 1] >= 0);
    assert(src_loop_iterator.mapping_offset[_token_dimensions - 1] <= buffer_lines);
    free_lines = buffer_lines - 
      src_loop_iterator.mapping_offset[_token_dimensions - 1];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 103
    CoSupport::Streams::dout << "Leave smoc_simple_md_buffer_kind::smoc_simple_md_buffer_kind" << std::endl;
    CoSupport::Streams::dout << CoSupport::Indent::Down;
#endif


  }

  virtual ~smoc_simple_md_buffer_kind(){}

public:
  void allocate_buffer();       
  void release_buffer();
  void free_buffer();
  bool hasUnusedStorage() const;

  template<class S, class T>
  void initStorageAccess(smoc_md_storage_access_snk<S,T> &storage_access){
    parent_type::initStorageAccess(storage_access);
    storage_access.simple_md_buffer = this;
  };
        
  template<class S, class T>
  void initStorageAccess(smoc_md_storage_access_src<S,T> &storage_access){
    parent_type::initStorageAccess(storage_access);
    storage_access.simple_md_buffer = this;
  };

  /// Create buffer (reserve memory)
  template <typename BUFFER_TYPE>
  void createStorage(BUFFER_TYPE *& ptr) const{

    // Check, that buffer size is not set to infinite
    assert(buffer_lines != MAX_TYPE(size_t));

    data_element_id_type buffer_size(size_token_space);
    buffer_size[_token_dimensions-1] = buffer_lines;
                
    ptr = new BUFFER_TYPE(_token_dimensions, buffer_size);

  }

  /// Destroy storage
  template <typename BUFFER_TYPE>
  void destroyStorage(BUFFER_TYPE *& ptr) const{
    delete ptr;
    ptr = NULL;
  }

  /* Functions for problem graph generation */
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const;


protected:

  const size_t buffer_lines;      

  /* Write pointers */
  /// start for data elements belonging to current schedule period.
  unsigned long wr_schedule_period_start;
  /// maximum data element offset currently stored in the buffer
  unsigned long wr_max_data_element_offset;


  /* Read pointers */
  /// start for data elements belonging to current schedule period.
  unsigned long rd_schedule_period_start;
  /// minumum data element offset which is stored in the buffer
  unsigned long rd_min_data_element_offset;
        

  /// Number of free buffer lines
  unsigned long free_lines;

private:
  /// Calculate the number of required new lines
  /// for storage of the given data element
  unsigned long calc_req_new_lines(const data_element_id_type& data_element_id, 
				   bool new_schedule_period) const;

private:
  /// The following elements help to make simulation faster
  /// by caching already calculated values
        
  /// Does there exist unusedStorage
  mutable bool cache_unusedStorage;
        
  mutable unsigned long cache_wr_schedule_period_start;
  mutable unsigned long cache_wr_max_data_element_offset;
  mutable unsigned long cache_free_lines;

};


template<>
class smoc_simple_md_buffer_kind::smoc_md_storage_access_src<void,void>
  : public smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<void,void>
{
  friend class smoc_simple_md_buffer_kind;
public:
  typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<void,void> parent_type;
  typedef parent_type::iter_domain_vector_type iter_domain_vector_type;
  
  typedef void                                             storage_type;
  typedef void                                             return_type;
  
public:
  smoc_md_storage_access_src()
    : smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<void,void>()
  {}
  
  virtual ~smoc_md_storage_access_src(){};
  
public:
  
  virtual void SetBuffer(smoc_md_storage_type<storage_type>::type *storage){
  }
  
  /* Data Element Access */
  virtual void operator[](const iter_domain_vector_type& window_iteration){
  }               
  
private:
  
protected:                
  
};



template<>
class smoc_simple_md_buffer_kind::smoc_md_storage_access_snk<void,void>
  : public smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<void,void>
{
  friend class smoc_simple_md_buffer_kind;
public:
  typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<void,void> parent_type;
  typedef parent_type::iter_domain_vector_type iter_domain_vector_type;
  
  typedef void                                             storage_type;
  typedef void                                             return_type;
  
  typedef smoc_snk_md_loop_iterator_kind::border_condition_vector_type border_condition_vector_type;
  typedef smoc_snk_md_loop_iterator_kind::border_type border_type;
  
public:
  smoc_md_storage_access_snk()
    : smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk<void,void>()
  {}
  
  virtual ~smoc_md_storage_access_snk(){};
  
public:
  
  virtual void SetBuffer(smoc_md_storage_type<storage_type>::type *storage){
  }
  
  /* Data Element Access */
  virtual void operator[](const iter_domain_vector_type& window_iteration){
  }
  
  virtual const void operator[](const iter_domain_vector_type& window_iteration) const{
  }
  
  
private:
  
};


template<>
void smoc_simple_md_buffer_kind::initStorageAccess(smoc_md_storage_access_snk<void,void> &storage_access);
template<>
void smoc_simple_md_buffer_kind::initStorageAccess(smoc_md_storage_access_src<void,void> &storage_access);
        




#endif // _INCLUDED_SMOC_BUFFER_HPP
