#ifndef _INCLUDED_SMOC_MD_BUFFER_HPP
#define _INCLUDED_SMOC_MD_BUFFER_HPP

#include <smoc_md_loop.hpp>
#include <cosupport/commondefs.h>
#include <smoc_vector.hpp>
#include <smoc_md_array.hpp>

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

	/// This class represents the common base class for access of
	/// data elements in a multi-dimensional buffer. Border processing
	/// is not performed.
	template<class S, class T>
	class smoc_md_storage_access{
		friend class smoc_md_buffer_mgmt_base;
	public:
		typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;
		typedef smoc_md_loop_snk_data_element_mapper::border_condition_vector_type border_condition_vector_type;
		typedef smoc_md_loop_snk_data_element_mapper::border_type_vector_type border_type_vector_type;
		
		typedef S					      storage_type;
		typedef T					      return_type;

	public:

		smoc_md_storage_access()
			: is_snk(false),
				src_data_el_mapper(NULL),
				snk_data_el_mapper(NULL)				
		{}

	public:
		
		/* Buffer Access Setup Routines */
#ifndef NDEBUG		
		/// Set window iteration limit
		virtual void setLimit(const iter_domain_vector_type& window_iteration_limit) {
			this->window_iteration_limit = window_iteration_limit;
		};
#endif

		/// Set buffer pointer
		virtual void SetBuffer(typename smoc_md_buffer_mgmt_base::smoc_md_storage_type<storage_type>::type *storage){};
		
		/// Set base iteration (without iteration levels representing window)
		/// If the iteration belongs to the sink, is_snk must be set to true, otherwise to false.
		virtual void setBaseIteration(const iter_domain_vector_type& base_iteration,
																	bool is_snk) {
			this->is_snk = is_snk;
			
			if (is_snk){
				base_border_condition_vector = 
					snk_data_el_mapper->calc_base_border_condition_vector(base_iteration);				
			}
		};
		
		/* Data Element Access */
		virtual return_type operator[](const iter_domain_vector_type& window_iteration) { assert(false); }
		virtual const return_type operator[](const iter_domain_vector_type& window_iteration) { assert(false); }

		/// Check, whether data element is situated on extended border
		virtual border_type_vector_type is_ext_border(const iter_domain_vector_type& window_iteration) const { 
			///only sink actor sees extended border
			assert(is_snk);
			border_condition_vector_type border_condition_vector = 
				snk_data_el_mapper->calc_border_condition_offset(window_iteration) +
				base_border_condition_vector;

			return snk_data_el_mapper->is_border_pixel(border_condition_vector);
		}
		
	protected:

		bool is_snk;

		const smoc_md_loop_src_data_element_mapper* src_data_el_mapper;
		const smoc_md_loop_snk_data_element_mapper* snk_data_el_mapper;

		border_condition_vector_type base_border_condition_vector;

		iter_domain_vector_type window_iteration_limit;
		
		void checkLimit(const iter_domain_vector_type& window_iteration) const{
#ifndef NDEBUG
			for(unsigned i = 0; i < window_iteration_limit.size(); i++){
				assert(window_iteration[i] < window_iteration_limit[i]);
			}
#endif
		}
	};
	
public:
	
	/// Dummy buffer init
	class buffer_init {
		friend class smoc_md_buffer_mgmt_base;
	private:
		const smoc_md_loop_src_data_element_mapper src_data_el_mapper;
		const smoc_md_loop_snk_data_element_mapper snk_data_el_mapper;
	public:
		buffer_init(const smoc_md_loop_src_data_element_mapper& src_data_el_mapper,
								const smoc_md_loop_snk_data_element_mapper& snk_data_el_mapper)
			: src_data_el_mapper(src_data_el_mapper),
				snk_data_el_mapper(snk_data_el_mapper)
		{}
	};


public:
  smoc_md_buffer_mgmt_base(const buffer_init& i)
    : src_data_el_mapper(i.src_data_el_mapper),
      snk_data_el_mapper(i.snk_data_el_mapper),
			_token_dimensions(i.src_data_el_mapper.token_dimensions()),
			size_token_space(i.src_data_el_mapper.size_token_space())
	{
		assert(src_data_el_mapper.token_dimensions() == 
					 snk_data_el_mapper.token_dimensions());
	};

	virtual ~smoc_md_buffer_mgmt_base(){}


public:
  /// Allocate the memory required by the given source iteration (effective token)
	/// Returns false, if function fails
	/// ATTENTION: once the function succeeds, it MUST NOT be called
	/// for the same loop-iteration!
  virtual bool allocate_buffer(const smoc_md_loop_iterator_kind& src_iterator) = 0;

  /// Free the memory read the last time by the following sink iteration
  virtual void free_buffer(const smoc_md_loop_iterator_kind& snk_iterator) = 0;  

	/// Check whether there is free space for the given source
	/// iteration or not
	virtual bool unusedStorage(const smoc_md_loop_iterator_kind& src_iterator) const = 0;


	/// Init storage access
	template<class S, class T>
	void initStorageAccess(smoc_md_storage_access<S,T> &storage_access){
		storage_access.src_data_el_mapper = &src_data_el_mapper;
		storage_access.snk_data_el_mapper = &snk_data_el_mapper;
		assert(src_data_el_mapper.token_dimensions() == snk_data_el_mapper.token_dimensions());
		storage_access.base_border_condition_vector.resize(src_data_el_mapper.token_dimensions(),false);
	};

 	/// Create buffer (reserve memory)
	template <typename BUFFER_TYPE>
	void createStorage(BUFFER_TYPE *& ptr) const{
		ptr = NULL;
	}
	


protected:
  const smoc_md_loop_src_data_element_mapper src_data_el_mapper;
  const smoc_md_loop_snk_data_element_mapper snk_data_el_mapper;
	const unsigned _token_dimensions;

	typedef smoc_md_loop_src_data_element_mapper::id_type id_type;
	typedef smoc_md_loop_src_data_element_mapper::data_element_id_type data_element_id_type;

	const data_element_id_type size_token_space;
  
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
	typedef smoc_md_loop_src_data_element_mapper::data_element_id_type data_element_id_type;
	typedef smoc_md_buffer_mgmt_base parent_type;

public:

	template<class S, class T>
	class smoc_md_storage_access
		: public smoc_md_buffer_mgmt_base::smoc_md_storage_access<S,T>
	{
		friend class smoc_simple_md_buffer_kind;
	public:
		typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access<S,T> parent_type;
		typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;
		
		typedef S					      storage_type;
		typedef T					      return_type;

		typedef smoc_md_loop_snk_data_element_mapper::border_condition_vector_type border_condition_vector_type;

	public:
		smoc_md_storage_access()
			: storage(NULL)
		{}

	public:
		
		/* Buffer Access Setup Routines */		
		virtual void setBaseIteration(const iter_domain_vector_type& base_iteration,
													bool is_snk){
			parent_type::setBaseIteration(base_iteration, is_snk);

			if (is_snk){
				(*this).snk_data_el_mapper->get_base_data_element_id(base_iteration,
																										 base_data_element);			
			}else{
				(*this).src_data_el_mapper->get_base_data_element_id(base_iteration,
																										 base_data_element);
			}
		}

		virtual void SetBuffer(typename smoc_md_storage_type<storage_type>::type *storage){
			this->storage = storage;
		}
		
		/* Data Element Access */
		virtual return_type operator[](const iter_domain_vector_type& window_iteration){
			check_limit(window_iteration);			
			if ((*this).is_snk){
				unsigned token_dimensions;
				token_dimensions = (*this).snk_data_el_mapper->token_dimensions();

				data_element_id_type data_element_id(token_dimensions);
				(*this).snk_data_el_mapper->get_window_data_element_offset(window_iteration,
																													 data_element_id);

				data_element_id += base_data_element;

				return (*storage)[data_element_id];
				
			}else{
				unsigned token_dimensions;
				token_dimensions = (*this).src_data_el_mapper->token_dimensions();

				data_element_id_type data_element_id(token_dimensions);
				(*this).src_data_el_mapper->get_window_data_element_offset(window_iteration,
																																	 data_element_id);

				
				data_element_id += base_data_element;

				return (*storage)[data_element_id];
			}
		}
		
		virtual const return_type operator[](const iter_domain_vector_type& window_iteration){
			check_limit(window_iteration);			
			if ((*this).is_snk){
				unsigned token_dimensions;
				token_dimensions = (*this).snk_data_el_mapper->token_dimensions();

				data_element_id_type data_element_id(token_dimensions);
				(*this).snk_data_el_mapper->get_window_data_element_offset(window_iteration,
																																	 data_element_id);

				data_element_id += base_data_element;

				return (*storage)[data_element_id];
				
			}else{
				unsigned token_dimensions;
				token_dimensions = (*this).src_data_el_mapper->token_dimensions();

				data_element_id_type data_element_id(token_dimensions);
				(*this).src_data_el_mapper->get_window_data_element_offset(window_iteration,
																													 data_element_id);

				
				data_element_id += base_data_element;

				return (*storage)[data_element_id];
			}
		}

		
	private:
		typename smoc_md_storage_type<storage_type>::type *storage;

	protected:

		data_element_id_type base_data_element;
		
	};
	



public:

	/// Buffer init
	class buffer_init 
		: public smoc_md_buffer_mgmt_base::buffer_init
	{
		friend class smoc_simple_md_buffer_kind;
	private:
		const size_t buffer_lines;
	public:
		buffer_init(const smoc_md_loop_src_data_element_mapper& src_data_el_mapper,
								const smoc_md_loop_snk_data_element_mapper& snk_data_el_mapper,
								const size_t buffer_lines = MAX_TYPE(size_t))
			: smoc_md_buffer_mgmt_base::buffer_init(src_data_el_mapper, snk_data_el_mapper),
				buffer_lines(buffer_lines)
		{};
	};

public:
  smoc_simple_md_buffer_kind(const buffer_init& i)
    : smoc_md_buffer_mgmt_base(i),
			buffer_lines(i.buffer_lines),
			rd_schedule_period_start(0),
			rd_min_data_element_offset(0)
	{
		//currently, we only support initial data elements in
		//the highest token dimension
		for(unsigned int i = 0; i < _token_dimensions-1; i++){
			assert(src_data_el_mapper.mapping_offset[i] == 0);
		}

		// wr_schedule_period_start = k* buffer_lines - size_token_space[_token_dimensions-1]
		// whereas k the smallest possible integer, such that wr_schedule_period_start > 0
		wr_schedule_period_start = buffer_lines - 
			(size_token_space[_token_dimensions-1] % buffer_lines);


		wr_max_data_element_offset = 
			size_token_space[_token_dimensions-1] + 
			 src_data_el_mapper.mapping_offset[_token_dimensions - 1] - 1;

		assert(src_data_el_mapper.mapping_offset[_token_dimensions - 1] >= 0);
		assert(src_data_el_mapper.mapping_offset[_token_dimensions - 1] <= buffer_lines);
		free_lines = buffer_lines - 
			src_data_el_mapper.mapping_offset[_token_dimensions - 1];


	}

	virtual ~smoc_simple_md_buffer_kind(){}

public:
  bool allocate_buffer(const smoc_md_loop_iterator_kind& src_iterator);	
  void free_buffer(const smoc_md_loop_iterator_kind& snk_iterator);
  bool unusedStorage(const smoc_md_loop_iterator_kind& src_iterator) const;

	template<class S, class T>
	void initStorageAccess(smoc_md_storage_access<S,T> &storage_access){
		parent_type::initStorageAccess(storage_access);
		storage_access.base_data_element.resize(src_data_el_mapper.token_dimensions(),false);
	};

	/// Create buffer (reserve memory)
	template <typename BUFFER_TYPE>
	void createStorage(BUFFER_TYPE *& ptr) const{
		data_element_id_type buffer_size(size_token_space);
		buffer_size[_token_dimensions-1] = buffer_lines;
		
		ptr = new BUFFER_TYPE(_token_dimensions, buffer_size);

	}

	template <typename T>
	void coucou(T*& ptr) {
		ptr = NULL;
	}


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
  
};



#endif // _INCLUDED_SMOC_BUFFER_HPP
