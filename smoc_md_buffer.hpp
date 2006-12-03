#ifndef _INCLUDED_SMOC_MD_BUFFER_HPP
#define _INCLUDED_SMOC_MD_BUFFER_HPP

#include <smoc_md_loop.hpp>
#include <cosupport/commondefs.h>
#include <smoc_vector.hpp>
#include <smoc_md_array.hpp>
#include <smoc_debug_out.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_BUFFER
#define VERBOSE_LEVEL_SMOC_MD_BUFFER 0
///101: operator[]
///102: border processing
#endif

/// Add additional asserts
#define PARANOIA_MODE

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
	class smoc_md_storage_access_src{
		friend class smoc_md_buffer_mgmt_base;
	public:
		typedef smoc_md_loop_iterator_kind::data_type iteration_type;
		typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;
		
		typedef S					      storage_type;
		typedef T					      return_type;


		
	public:
		
		smoc_md_storage_access_src()
			: buffer(NULL),
				src_iterator(NULL),
				src_data_el_mapper(NULL)
		{}

		virtual ~smoc_md_storage_access_src(){};
		
	public:
		
		/* Buffer Access Setup Routines */
#ifndef NDEBUG		
		/// Set limit
		/// Dummy function
		void setLimit(size_t limit) {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_src::setLimit"
					 << endl;
			dout << inc_level;
			dout << "limit = " << limit << endl;
#endif
			assert(limit <= 1);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_src::setLimit"
					 << endl;
			dout << dec_level;
#endif
		};
#endif
		
		/// Set buffer pointer
		virtual void SetBuffer(typename smoc_md_buffer_mgmt_base::smoc_md_storage_type<storage_type>::type *storage){};

		virtual void SetIterator(const smoc_md_loop_iterator_kind& src_iterator){
			this->src_iterator = &src_iterator;
		}

		/* Data Element Access */
		virtual return_type operator[](const iter_domain_vector_type& window_iteration) { assert(false); }
		virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const { assert(false); }

		/// Returns the value of the loop iterator for the given iteration level
		iteration_type iteration(size_t iteration_level) const {
			return (*src_iterator)[iteration_level];
		}
		
		
	protected:

		smoc_md_buffer_mgmt_base *buffer;
		
		const smoc_md_loop_iterator_kind* src_iterator;
		const smoc_md_loop_src_data_element_mapper* src_data_el_mapper;

		void checkLimit(const iter_domain_vector_type& window_iteration) const{
#ifndef NDEBUG
			iter_domain_vector_type max_window_iteration(src_iterator->max_window_iteration());
			for(unsigned i = 0; i < window_iteration.size(); i++){
				assert(window_iteration[i] <= 
							 max_window_iteration[max_window_iteration.size() - 
																	 window_iteration.size()+
																	 i]);
			}
#endif
		}
	};
	
	template<class S, class T>
	class smoc_md_storage_access_snk{
		friend class smoc_md_buffer_mgmt_base;
	public:
		typedef smoc_md_loop_iterator_kind::data_type iteration_type;
		typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;


		typedef smoc_md_loop_snk_data_element_mapper::border_condition_vector_type border_condition_vector_type;
		typedef smoc_md_loop_snk_data_element_mapper::border_type border_type;
		typedef smoc_md_loop_snk_data_element_mapper::border_type_vector_type border_type_vector_type;
		
		typedef S					      storage_type;
		typedef T					      return_type;
		
	public:
		
		smoc_md_storage_access_snk()
			: buffer(NULL),
				snk_iterator(NULL),
				snk_data_el_mapper(NULL)
		{}

		virtual ~smoc_md_storage_access_snk(){};
		
	public:
		
		/* Buffer Access Setup Routines */
#ifndef NDEBUG		
		/// Set limit, how many windows can be accessed
		/// dummy function
		void setLimit(size_t limit) {
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::setLimit"
					 << endl;
			dout << inc_level;
			dout << "limit = " << limit << endl;
#endif
			assert(limit <= 1);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::setLimit"
					 << endl;
			dout << dec_level;
#endif
		};
#endif
		
		/// Set buffer pointer
		virtual void SetBuffer(typename smoc_md_buffer_mgmt_base::smoc_md_storage_type<storage_type>::type *storage){};
		
		/// Set reference to iterator
		virtual void SetIterator(const smoc_md_loop_iterator_kind& snk_iterator){
			this->snk_iterator = &snk_iterator;
		};

		
		/* Data Element Access */
		virtual return_type operator[](const iter_domain_vector_type& window_iteration) { assert(false); }
		virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const { assert(false); }

		/// Returns the value of the loop iterator for the given iteration level
		iteration_type iteration(size_t iteration_level) const {
			return (*snk_iterator)[iteration_level];
		}

		/// Check, whether data element is situated on extended border
		virtual border_type_vector_type is_ext_border(const iter_domain_vector_type& window_iteration,
																									bool& is_border) const { 
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
			dout << buffer->name() << ": Enter smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::is_ext_border" << endl;
			dout << inc_level;
			dout << "snk_iterator->iteration_vector() = " << snk_iterator->iteration_vector();
			dout << endl;
			dout << "window_iteration = " << window_iteration;
			dout << endl;
#endif

			border_condition_vector_type border_condition_vector = 
				snk_data_el_mapper->calc_base_border_condition_vector(snk_iterator->iteration_vector());
#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
			dout << "base_border_condition_vector = " << border_condition_vector;
			dout << endl;
#endif
			border_condition_vector += 
				snk_data_el_mapper->calc_border_condition_offset(window_iteration);			

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
			dout << "border_condition_vector = " << border_condition_vector;
			dout << endl;
#endif
			
			border_type_vector_type
				return_vector(snk_data_el_mapper->is_border_pixel(border_condition_vector, is_border));

#if (VERBOSE_LEVEL_SMOC_MD_BUFFER == 102) || (VERBOSE_LEVEL_SMOC_MD_BUFFER == 101)
			dout << "Leave smoc_md_buffer_mgmt_base::smoc_md_storage_access_snk::is_ext_border" << endl;
			dout << dec_level;
#endif

			return return_vector;
			
		}
		
	protected:

		smoc_md_buffer_mgmt_base *buffer;
		
		const smoc_md_loop_iterator_kind* snk_iterator;
		const smoc_md_loop_snk_data_element_mapper* snk_data_el_mapper;

		void checkLimit(const iter_domain_vector_type& window_iteration) const{
#ifndef NDEBUG
			iter_domain_vector_type max_window_iteration(snk_iterator->max_window_iteration());
			for(unsigned i = 0; i < window_iteration.size(); i++){
				assert(window_iteration[i] <= 
							 max_window_iteration[max_window_iteration.size() - 
																		window_iteration.size()+
																		i]);
			}
#endif
		}
	};
		
	
public:
	
	/// buffer init
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

  /// Free the memory read the last time by the following sink iteration
  virtual void free_buffer(const smoc_md_loop_iterator_kind& snk_iterator) = 0;

	///Checks, wether buffer can be allocated
	virtual bool unusedStorage(const smoc_md_loop_iterator_kind& src_iterator) const = 0;

  /// Init storage access
	template<class S, class T>
	void initStorageAccess(smoc_md_storage_access_src<S,T> &storage_access){
		storage_access.buffer = this;
		storage_access.src_data_el_mapper = &src_data_el_mapper;
	};

  template<class S, class T>
	void initStorageAccess(smoc_md_storage_access_snk<S,T> &storage_access){
		storage_access.buffer = this;
		storage_access.snk_data_el_mapper = &snk_data_el_mapper;
	};

 	/// Create buffer (reserve memory)
	template <typename BUFFER_TYPE>
	void createStorage(BUFFER_TYPE *& ptr) const{
		ptr = NULL;
	}

public:
	//must be mapped by the FIFO to the FIFO name
	virtual const char *name() const = 0;	


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
	typedef smoc_md_loop_src_data_element_mapper::data_element_id_type data_element_id_type;
	typedef smoc_md_buffer_mgmt_base parent_type;

public:

	template<class S, class T>
	class smoc_md_storage_access_src
		: public smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<S,T>
	{
		friend class smoc_simple_md_buffer_kind;
	public:
		typedef smoc_md_buffer_mgmt_base::smoc_md_storage_access_src<S,T> parent_type;
		typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;
		
		typedef S					      storage_type;
		typedef T					      return_type;

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
			dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_src::operator[]"
					 << endl;
			dout << inc_level;
#endif

			checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "window_iteration = " << window_iteration;
			dout << endl;
#endif
			//Allocate the memory for the current source iteration.
			simple_md_buffer->allocate_buffer();

			unsigned token_dimensions = (*this).src_data_el_mapper->token_dimensions();

			data_element_id_type base_data_element_id(token_dimensions);
			(*this).src_data_el_mapper->get_base_data_element_id((*this).src_iterator->iteration_vector(),
																													 base_data_element_id);
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "base_data_element_id = " << base_data_element_id;
			dout << endl;
#endif

			data_element_id_type data_element_id(token_dimensions);			
			this->src_data_el_mapper->get_window_data_element_offset(window_iteration,
																															 data_element_id);
			
			data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "data_element_id = " << data_element_id;
			dout << endl;
#endif

			data_element_id[token_dimensions-1] += 
				(*simple_md_buffer).wr_schedule_period_start;
			data_element_id[token_dimensions-1] = 
				data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Array element = " << data_element_id;
			dout << endl;
#endif

			return_type return_value((*storage)[data_element_id]);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_src::operator[]"
					 << endl;
			dout << dec_level;
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
		
		typedef S					      storage_type;
		typedef T					      return_type;

		typedef smoc_md_loop_snk_data_element_mapper::border_condition_vector_type border_condition_vector_type;
		typedef smoc_md_loop_snk_data_element_mapper::border_type border_type;

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
			dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
					 << endl;
			dout << inc_level;
#endif

			checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "window_iteration = " << window_iteration;
			dout << endl;
#endif

			unsigned token_dimensions = (*this).snk_data_el_mapper->token_dimensions();

			data_element_id_type base_data_element_id(token_dimensions);
			(*this).snk_data_el_mapper->get_base_data_element_id((*this).snk_iterator->iteration_vector(),
																													 base_data_element_id);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "base_data_element_id = " << base_data_element_id;
			dout << endl;
#endif

			data_element_id_type data_element_id(token_dimensions);			
			this->snk_data_el_mapper->get_window_data_element_offset(window_iteration,
																															 data_element_id);			
			data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "data_element_id = " << data_element_id;
			dout << endl;
#endif

			data_element_id[token_dimensions-1] += 
				(*simple_md_buffer).rd_schedule_period_start;
			data_element_id[token_dimensions-1] = 
				data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Array element = " << data_element_id;
			dout << endl;
#endif

			return_type return_value = (*storage)[data_element_id];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
					 << endl;
			dout << dec_level;
#endif

			return return_value;
		}
		
		virtual const return_type operator[](const iter_domain_vector_type& window_iteration) const{
#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << simple_md_buffer->name() << ": Enter smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
					 << endl;
			dout << inc_level;
#endif

			checkLimit(window_iteration);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "window_iteration = " << window_iteration;
			dout << endl;
#endif

			unsigned token_dimensions = (*this).snk_data_el_mapper->token_dimensions();

			data_element_id_type base_data_element_id(token_dimensions);
			(*this).snk_data_el_mapper->get_base_data_element_id((*this).snk_iterator->iteration_vector(),
																													 base_data_element_id);

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "base_data_element_id = " << base_data_element_id;
			dout << endl;
#endif


			data_element_id_type data_element_id(token_dimensions);			
			this->snk_data_el_mapper->get_window_data_element_offset(window_iteration,
																															 data_element_id);			
			data_element_id += base_data_element_id;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "data_element_id = " << data_element_id;
			dout << endl;
#endif

			data_element_id[token_dimensions-1] += 
				(*simple_md_buffer).rd_schedule_period_start;
			data_element_id[token_dimensions-1] = 
				data_element_id[token_dimensions-1] % (*simple_md_buffer).buffer_lines;

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Array element = " << data_element_id;
			dout << endl;
#endif

			return_type return_value = (*storage)[data_element_id];

#if VERBOSE_LEVEL_SMOC_MD_BUFFER == 101
			dout << "Leave smoc_simple_md_buffer_kind::smoc_md_storage_access_snk::operator[]"
					 << endl;
			dout << dec_level;
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
			rd_min_data_element_offset(0),
			cache_unusedStorage(false)
#ifdef PARANOIA_MODE
		,cache_src_iterator(NULL)
#endif
			
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
  void allocate_buffer();	
	void release_buffer();
  void free_buffer(const smoc_md_loop_iterator_kind& snk_iterator);
	bool unusedStorage(const smoc_md_loop_iterator_kind& src_iterator) const;

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
		data_element_id_type buffer_size(size_token_space);
		buffer_size[_token_dimensions-1] = buffer_lines;
		
		ptr = new BUFFER_TYPE(_token_dimensions, buffer_size);

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

private:
	/// The following elements help to make simulation faster
	/// by caching already calculated values
	
	/// Does there exist unusedStorage
	mutable bool cache_unusedStorage;
	
	mutable unsigned long cache_wr_schedule_period_start;
	mutable unsigned long cache_wr_max_data_element_offset;
	mutable unsigned long cache_free_lines;

#ifdef PARANOIA_MODE
	mutable const smoc_md_loop_iterator_kind* cache_src_iterator;
#endif
	
  
};



#endif // _INCLUDED_SMOC_BUFFER_HPP
