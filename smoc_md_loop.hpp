#ifndef _INCLUDED_SMOC_MD_LOOP_HPP
#define _INCLUDED_SMOC_MD_LOOP_HPP

#include <smoc_vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

/// Common base class for all loop iterators
/// A loop iterator describes a nested loop
class smoc_md_loop_iterator_kind {
public:
  // Typedefs
  typedef unsigned long data_type;  

  //Specification of iteration domain
  typedef smoc_vector<data_type>      iter_domain_vector_type;

  typedef iter_domain_vector_type::size_type size_type;

public:
	/// constructor
	smoc_md_loop_iterator_kind(size_type window_dimensions,
														 const iter_domain_vector_type& iteration_start
														 )
		: current_iteration(iteration_start),
			_new_schedule_period(true),			  
			_window_dimensions(window_dimensions){};

	/// Copy constructor
	smoc_md_loop_iterator_kind(const smoc_md_loop_iterator_kind& src_iterator)
		: current_iteration(src_iterator.current_iteration),
			_window_dimensions(src_iterator._window_dimensions){};

	/// Destructor
	virtual ~smoc_md_loop_iterator_kind(){};

public:
	
  /// Move to next iteration
  /// IMPORTANT: we first try to increase the HIGHEST coordinate

	/// Move to next iteration
	/// The function returns true, if a new schedule period is started.
	virtual bool inc() = 0;

  /// Access iteration vector
  data_type operator[](size_type idx) const{
		return current_iteration[idx];
	}

	/// Get whole iteration vector
	const iter_domain_vector_type& iteration_vector() const {
		return current_iteration;
	}

	/// check, whether current iteration is the first
	/// of a new schedule period
	bool is_new_schedule_period() const { return _new_schedule_period;}

	/// Return number of window dimensions
	const size_type window_dimensions() const { return _window_dimensions;}

	/// Return number of iterator dimenions
	const data_type iterator_depth() const {return current_iteration.size();}

	/* Determination of iteration borders */

	/// Calculate the maximum iteration vector possible for the
	/// current window position.
  virtual const iter_domain_vector_type max_window_iteration() const = 0;

	/// This function returns the maximum iteration value
	/// for the given dimension, supposing, that the smaller dimensions
	/// are set to the values given in fixed_iteration.
	virtual data_type iteration_max(const size_type dimension,
																	const iter_domain_vector_type& fixed_iteration) const = 0;

	/// This function returns the overall iteration maxumum
	virtual const iter_domain_vector_type iteration_max() const = 0;

protected:

	/// current iteration vector
  iter_domain_vector_type current_iteration;

	/// flag, whether current_iteration is the first of a new
	/// schedule period
	bool _new_schedule_period;


	/// Number of dimensions of window
  const size_type _window_dimensions;  
};




/// Description of a static nested loop
/// The iteration bounds are independent on dimension i are
/// independent of the other dimensions.
class smoc_md_static_loop_iterator 
	: public smoc_md_loop_iterator_kind
{
public:
  // Typedefs
  typedef smoc_md_loop_iterator_kind::data_type data_type;  

  //Specification of iteration domain
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  typedef smoc_md_loop_iterator_kind::size_type size_type;
public:
  /* Constructors */

  /// This constructor allows to declare a so called
  /// window-iteration domain. The window iteration domain
  /// Is supposed to be executed externally.
  smoc_md_static_loop_iterator(const iter_domain_vector_type& max,
															 const size_type window_dimension
															 );

  smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator);

public:

	virtual bool inc();

	/// Calculation of some special window iterations
  virtual const iter_domain_vector_type max_window_iteration() const;

	virtual data_type iteration_max(const size_type dimension,
																	const iter_domain_vector_type& fixed_iteration) const {
		return _iteration_max[dimension];
	}

	virtual const iter_domain_vector_type iteration_max() const{
		return _iteration_max;
	}

protected:  

  //Iteration bounds
  const iter_domain_vector_type _iteration_max;

  
};








/// Data element mapping for source actor
class smoc_md_loop_src_data_element_mapper 
{
public:
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  /// Data element identifier
	typedef long id_type;
  typedef smoc_vector<id_type> data_element_id_type;

	/// Mapping vector
	typedef unsigned long mapping_type;
	typedef boost::numeric::ublas::matrix<mapping_type> mapping_matrix_type;

	/// Offset vector
	typedef unsigned long offset_type;
	typedef smoc_vector<offset_type> mapping_offset_type;

public:
	///Constructor
	///Input parameters:
	/// - mapping_matrix, mapping_offset
	/// - max_data_element_id: data element with maximum ID in all dimensions, occuring
	///                        in one schedule period.
	smoc_md_loop_src_data_element_mapper(const mapping_matrix_type& mapping_matrix,
																			 const mapping_offset_type& mapping_offset,
																			 const data_element_id_type& max_data_element_id
																			 ):
		_token_dimensions(mapping_offset.size()),
		mapping_matrix(mapping_matrix),
		mapping_offset(mapping_offset),
		_max_data_element_id(max_data_element_id),
		mapping_table(calc_mapping_table(mapping_matrix))
	{
		assert(check_matrix(mapping_matrix));
	};
	
public:
	/// return the number of token_dimensions
	unsigned token_dimensions() const { return _token_dimensions;}

	/// Input parameters:
	/// - iteration_vector:       loop iteration, for which the accessed data element
	///                           shall be calculated.
	/// Output parameters:
	/// - data_element_id:        accessed data element ID
	/// - schedule_period_offset: offset to the schedule period which the
  ///                           data element belongs to.
	void get_data_element_id(const iter_domain_vector_type& iteration_vector,
													 data_element_id_type& data_element_id,
													 id_type& schedule_period_offset
													 ) const;	
	
  /// Same as above, but use a separate window iteration
	void get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
													 const iter_domain_vector_type& window_iteration,
													 data_element_id_type& data_element_id,
													 id_type& schedule_period_offset
													 ) const;

	/// Same as above, but without taken window iteration into account
	/// The schedule period offset is NOT calculated. Instead, data element
	/// identifiers might be returned which are larger than one schedule
	/// period.
	void get_base_data_element_id(const iter_domain_vector_type& iteration_vector,
																data_element_id_type& data_element_id
																) const;

	/// Calculate the data element offset which is caused by the window
	/// iteration. Note: ONLY the window iteration must be passed as
	/// argument, not the complete iteration vector
	void get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
																			data_element_id_type& data_element_offset) const;

	/// Returns the data element with the maximum coordinate for the 
	/// given loop iterator
	void max_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
													 data_element_id_type& max_data_element_id,
													 id_type& schedule_period_offset) const;	

	/// Returns the overal maximum data element ID for one schedule period
	const data_element_id_type& max_data_element_id() const;

	/// Returns the size of the token space
	const data_element_id_type size_token_space() const;	

  /// Calculate the source iteration producing the
  /// given data element. The function assumes, that this iteration
	/// is unambiguous.
	/// Function input parameters
	///  - src_data_el_id: Identifier of the source data element
	///  - loop_iterator:  Loop iterator belonging to the source actor
	/// Function output parameters
	///  - iteration_vector       : source iteration generating the source data element
	///  - schedule_period_offset : Offset in the schedule period.
	/// Return value:
	///  The function returns true, if the given data element is produced
	///  by the source actor, false otherwise.
  bool get_src_loop_iteration(const data_element_id_type& src_data_el_id,
															iter_domain_vector_type& iteration_vector,
															id_type& schedule_period_offset
															) const;

protected:
	const unsigned _token_dimensions;
	const mapping_matrix_type mapping_matrix;
public:
	const mapping_offset_type mapping_offset;
protected:
	const data_element_id_type _max_data_element_id;

	///This table indicates for each column of the
	///mapping matrix which token dimension is influenced
	const smoc_vector<int> mapping_table;

	

private:

	/// Checks matrix properties
	bool check_matrix(const mapping_matrix_type& mapping_matrix) const;

	/// builds a map which assignes to each column of the mapping
	/// matrix which token dimension is influenced
	smoc_vector<int> calc_mapping_table(const mapping_matrix_type& mapping_matrix) const;

};




/// Data element mapping for sink actor
class smoc_md_loop_snk_data_element_mapper
{
public:
	typedef smoc_md_loop_iterator_kind::data_type iter_item_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  /// Data element identifier
	typedef long id_type;
  typedef smoc_vector<id_type> data_element_id_type;

	/// Mapping vector
	typedef unsigned long mapping_type;
	typedef boost::numeric::ublas::matrix<mapping_type> mapping_matrix_type;
	typedef smoc_vector<mapping_type> mapping_vector_type;

	/// Offset vector
	typedef long offset_type;
	typedef smoc_vector<offset_type> mapping_offset_type;

	/// Condition matrix for border pixels
	typedef boost::numeric::ublas::matrix<id_type> border_condition_matrix_type;
	typedef smoc_vector<id_type> border_condition_vector_type;

	/// Specification of border
	enum border_type {NO_BORDER, LEFT_BORDER, RIGHT_BORDER};
	typedef smoc_vector<border_type> border_type_vector_type;
	
	

public:
	///Constructor
	///Input parameters:
	/// - mapping_matrix, mapping_offset
	/// - border_matrix      : Matrix by which can be detected whether a data element is situated
	///                        on the extended border or not.
	/// - high_border_vector : Vector by which can be detected whether a data element is situated
	///                        on the extended border (with large coordinates) or not.
	smoc_md_loop_snk_data_element_mapper(const mapping_matrix_type& mapping_matrix,
																			 const mapping_offset_type& mapping_offset,
																			 const border_condition_matrix_type& border_matrix,
																			 const border_condition_vector_type& low_border_vector,
																			 const border_condition_vector_type& high_border_vector
																			 ):
		_token_dimensions(mapping_offset.size()),
		mapping_matrix(mapping_matrix),
		mapping_offset(mapping_offset),
		border_condition_matrix(border_matrix),
		low_border_condition_vector(low_border_vector),
		high_border_condition_vector(high_border_vector)
	{
		assert(check_border_condition_matrix(border_matrix));
	};

public:
	/// return the number of token_dimensions
	unsigned token_dimensions() const { return _token_dimensions;}
	

	/// Input parameters:
	/// - iteration_vector:       loop iteration, for which the accessed data element
	///                           shall be calculated.
	/// Output parameters:
	/// - data_element_id:        accessed data element ID
	void get_data_element_id(const iter_domain_vector_type& iteration_vector,
													 data_element_id_type& data_element_id) const;
	
  /// Same as above, but use a separate window iteration
	void get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
													 const iter_domain_vector_type& window_iteration,
													 data_element_id_type& data_element_id
													 ) const;

	/// Same as above, but without taking window iteration into account
	void get_base_data_element_id(const iter_domain_vector_type& iteration_vector,
																data_element_id_type& data_element_id) const;

	/// Calculate the data element offset which is caused by the window
	/// iteration. Note: ONLY the window iteration must be passed as
	/// argument, not the complete iteration vector
	void get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
																			data_element_id_type& data_element_offset) const;
	
	/// This function determines the data element which is required for execution of
	/// the given sink iterator and and which is produced latest by the source actor.
	/// The function returns 'false' when no data element produced by the source actor
	/// is required. Otherwise 'true' is returned.
	bool get_req_src_data_element(const smoc_md_loop_iterator_kind& snk_iterator,
																data_element_id_type& data_element_id) const;

	/// This function determines whether the given loop iterator
	/// has its maximum position regarding the given token dimension
	/// By default, the window iteration is ignored
	bool is_iteration_max(const smoc_md_loop_iterator_kind& snk_iterator,
												unsigned token_dimension,
												bool ignore_window_iteration = true) const;

#if 0
# error "The following functions are not tested"
	/// Calculates for the given iterator the number of pixels which are situated on the low border
	mapping_type calc_num_low_border_data_elements(const smoc_md_loop_iterator_kind& snk_iterator,
																								 unsigned token_dimension) const;
	/// Calculates for the given iterator the number of pixels which are situated on the high border
	mapping_type calc_num_high_border_data_elements(const smoc_md_loop_iterator_kind& snk_iterator,
																								 unsigned token_dimension) const;
	

	/// Calculates the window displacement for the given sink iteration.
	/// If the return-value is false, the iterator is for the given dimension
	/// at the end of the schedule period and the value of window_displacement
	/// is not valid. Otherwise the return-value is true.
	bool calc_window_displacement(const smoc_md_loop_iterator_kind& snk_iterator,
																unsigned token_dimension,
																mapping_type& window_displacement) const;
#endif

	/// This function calculates the window displacement from the SOURCE 
	/// POINT of view. Due to border processing this is not identical
	/// with the displacement from the sink point of view.
	/// If the return-value is false, the iterator is for the given dimension
	/// at the end of the schedule period and the value of window_displacement
	/// is not valid. Otherwise the return-value is true.
	bool calc_eff_window_displacement(const smoc_md_loop_iterator_kind& snk_iterator,
																		unsigned token_dimension,
																		mapping_type& window_displacement) const;

	
	
	

protected:
	const unsigned _token_dimensions;
	const mapping_matrix_type mapping_matrix;
	const mapping_offset_type mapping_offset;

	const border_condition_matrix_type border_condition_matrix;
	const border_condition_vector_type low_border_condition_vector;
	const border_condition_vector_type high_border_condition_vector;

private:
	/// Due to reasons of simplicity, we restrict to special border condition
	/// matrices. This allows for instance to speed up calculation. The following
	/// function verifies, that the assumed conditions are fullfilled.
	bool check_border_condition_matrix(const border_condition_matrix_type& border_matrix) const;

public:
	/// This function multiplies the given iteration vector with the
	/// border condition matrix. However, the window iteration ARE NOT
	/// TAKEN into account.
	border_condition_vector_type calc_base_border_condition_vector(const iter_domain_vector_type& iteration) const;

	/// This function takes the border condition calculated by the above
	/// function and adds the part for the window iteration given by
	/// the iteration vector.
	border_condition_vector_type calc_window_border_condition_vector(const border_condition_vector_type& base_border_condition_vector,
																																	 const iter_domain_vector_type& iteration) const;

	///Same as above, but this time window_iteration ONLY specifies the window
	///iteration. Furthermore, the resulting offset is returned instead
	///of adding it to the base vector.
	border_condition_vector_type calc_border_condition_offset(const iter_domain_vector_type& window_iteration) const;
	
	/// Same as above, but only for specified dimension
	id_type calc_base_border_condition(const iter_domain_vector_type& iteration,
																		 unsigned dimension) const;
	id_type calc_window_border_condition(id_type base_border_condition,
																			 const iter_domain_vector_type& iteration,
																			 unsigned dimension) const;

	/// This function takes a border condition vector and determines for each 
	/// token dimension, on which border the corresponding data element is
	/// situated.
	/// is_border is true, when the pixel is situated on an extended border.
	///  On which one precisely can be determined by the return-value
	/// Otherwise is_border is false
	border_type_vector_type is_border_pixel(const border_condition_vector_type& border_condition_vector,
																					bool& is_border) const;
	

};




#endif
