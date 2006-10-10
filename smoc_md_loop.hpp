#ifndef _INCLUDED_SMOC_MD_LOOP_HPP
#define _INCLUDED_SMOC_MD_LOOP_HPP

#include <smoc_vector.hpp>

/// Common base class for all loop iterators
/// A loop iterator describes a nested loop
class smoc_md_loop_iterator_kind {
public:
  // Typedefs
  typedef long data_type;  

  //Specification of iteration domain
  typedef smoc_vector<data_type>      iter_domain_vector_type;

  typedef iter_domain_vector_type::size_type size_type;

public:
	/// constructor
	smoc_md_loop_iterator_kind(size_type window_dimensions,
														 const iter_domain_vector_type& iteration_start,
														 bool start_iteration_flag = false
														 )
		: current_iteration(iteration_start),
			_iteration_flag(start_iteration_flag),
			_window_dimensions(window_dimensions){};

	/// Copy constructor
	smoc_md_loop_iterator_kind(const smoc_md_loop_iterator_kind& src_iterator)
		: current_iteration(src_iterator.current_iteration),
			_iteration_flag(src_iterator._iteration_flag),
			_window_dimensions(src_iterator._window_dimensions){};

	/// Destructor
	virtual ~smoc_md_loop_iterator_kind(){};

public:
	
  /// Move to next iteration
  /// IMPORTANT: we first try to increase the HIGHEST coordinate

  // prefix operator  
	virtual smoc_md_loop_iterator_kind& operator++() = 0;
	// postfix operator not defined, because we have a virtual class.

  /// Access iteration vector
  data_type operator[](size_type idx) const{
		return current_iteration[idx];
	}

	/// Get whole iteration vector
	const iter_domain_vector_type& iteration_vector() const {
		return current_iteration;
	}

  /// Access iteration flag
  bool iteration_flag() const {
    return _iteration_flag;
  } 	

	/// Return number of window dimensions
	const size_type window_dimensions() const { return _window_dimensions;}

	/// Return number of iterator dimenions
	const data_type iterator_depth() const {return current_iteration.size();}


	/// Calculation of some special window iterations
  virtual const iter_domain_vector_type max_window_iteration() const = 0;
  virtual const iter_domain_vector_type min_window_iteration() const = 0;

	/// Calculation of iteration domain
  virtual const iter_domain_vector_type& iteration_min() const = 0;
  virtual const iter_domain_vector_type& iteration_max() const = 0;

	/// This function returns the maximum iteration value
	/// for the given dimension, supposing, that the smaller dimensions
	/// are set to the values given in fixed_iteration.
	virtual data_type iteration_max(const size_type dimension,
																	const iter_domain_vector_type& fixed_iteration) const = 0;

protected:

	//current iteration vector
  iter_domain_vector_type current_iteration;

  /// MD data flow graphs execute infinite streams of data.
  /// However, due to implementation reasons, the iterators
  /// are bounded by a finite number. This iteration space
	/// corresponds to the schedule period. In order to avoid
  /// ambiguity, we introduce an iteration flags. Each
  /// time a loop iterator has traversed the complete
  /// iteration space, the iterator flag is toggled.
	/// 
  /// The iterators must be such, that a snk iterator
  /// only reads data elements originated by a source
  /// iteration with the same flag value!	
  bool _iteration_flag;

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
  smoc_md_static_loop_iterator(const iter_domain_vector_type& min,
															 const iter_domain_vector_type& max,
															 const size_type window_dimension
															 );

  smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator);

public:
  /// Move to next iteration
  /// IMPORTANT: we first try to increase the HIGHEST coordinate
  virtual smoc_md_static_loop_iterator& operator++(); //prefix operator

	/// Calculation of some special window iterations
  virtual const iter_domain_vector_type max_window_iteration() const;
  virtual const iter_domain_vector_type min_window_iteration() const;

	/// Calculation of iteration domain
  virtual const iter_domain_vector_type& iteration_min() const { return _iteration_min; }
  virtual const iter_domain_vector_type& iteration_max() const { return _iteration_max; }

	
	virtual data_type iteration_max(const size_type dimension,
																	const iter_domain_vector_type& fixed_iteration) const {
		return _iteration_max[dimension];
	}

protected:  

  //Iteration bounds
  const iter_domain_vector_type _iteration_min;
  const iter_domain_vector_type _iteration_max;

  
};








/// This class performs the mapping between
/// a loop iteration and the corresponding data element
class smoc_md_loop_data_element_mapper {
public:
  typedef long id_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  /// Data element identifier
  typedef smoc_vector<id_type> data_element_id_type;

	/// Mapping vector
	typedef unsigned mapping_type;	
	typedef smoc_vector<mapping_type> mapping_vector_type;

	/// Offset vector
	typedef long offset_type;
	typedef smoc_vector<offset_type> offset_vector_type;

public:
  /* constructors */
	smoc_md_loop_data_element_mapper(const mapping_vector_type& mapping_weights,
																	 const offset_vector_type& mapping_offset)
		: token_dimensions(mapping_offset.size()),
			mapping_weights(mapping_weights),
			mapping_offset_vector(mapping_offset)
	{
	};
	

public:
  /// Calculate the data element accessed by the given loop-iterator
	data_element_id_type get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator) const {
		return get_data_element_id(loop_iterator.iteration_vector());
	}
	data_element_id_type get_data_element_id(const iter_domain_vector_type& iteration_vector) const;
	
  /// Same as above, but use a separate window iteration
	data_element_id_type get_data_element_id(const smoc_md_loop_iterator_kind& loop_iterator,
																					 const iter_domain_vector_type& window_iteration
																					 ) const;
protected:

	/// Number of dimensions of a token
	const unsigned token_dimensions;
	
	/// The dimensions of the iteration vector are cyclically assigned
	/// to the different token dimensions. Having for example an
	/// iteration vector with 6 dimensions and window of two dimensions,
	/// then iter[0],iter[2] and iter[4] determine the position of the data
	/// element in the first dimension. iter[1], iter[3] and iter[5] in the
	/// second dimension.
	/// The weights, by which these iteration values must be multiplied
	/// in order to determine the corresponding data element is given
	/// by the following vector.
	const mapping_vector_type mapping_weights;

	/// Initial data elements cause an offset between the iteration
	/// vectors and the produced data elements. This can be specified
	/// by the following vector.
	/// For a sink actor, the following vector describe for instance
	/// extended borders.
	const offset_vector_type mapping_offset_vector;	

};

/// Data element mapping for source actor
class smoc_md_loop_src_data_element_mapper 
  : public smoc_md_loop_data_element_mapper
{

public:
	typedef smoc_md_loop_data_element_mapper::id_type id_type;
  typedef smoc_md_loop_data_element_mapper::iter_domain_vector_type iter_domain_vector_type;
  typedef smoc_md_loop_data_element_mapper::data_element_id_type data_element_id_type;
	typedef smoc_md_loop_data_element_mapper::mapping_type mapping_type;
	typedef smoc_md_loop_data_element_mapper::mapping_vector_type mapping_vector_type;
	typedef smoc_md_loop_data_element_mapper::offset_type offset_type;
	typedef smoc_md_loop_data_element_mapper::offset_vector_type offset_vector_type;

public:
	smoc_md_loop_src_data_element_mapper(const mapping_vector_type& mapping_weights,
																			 const offset_vector_type& mapping_offset
																			 )
		: smoc_md_loop_data_element_mapper(mapping_weights,
																			 mapping_offset)
	{};

public:

  /// Calculate the source iteration producing the
  /// given data element. The function assumes, that this iteration
	/// is unambiguous.
	/// Function input parameters
	///  - src_data_el_id: Identifier of the source data element
	///  - loop_iterator:  Loop iterator belonging to the source actor
	/// Function output parameters
	///  - iteration_vector: source iteration generating the source data element
	///  - prev_flag       : Is set to true, if the source iteration 
	///                      belongs to the previous schedule period.
	///                      (relative to the current sink actor schedule period!!!!)
	/// Return value: false, if data element has not been produced by
	///               source actor, otherwise true.
  bool get_src_loop_vector(const data_element_id_type& src_data_el_id,
													 const smoc_md_loop_iterator_kind& loop_iterator,
													 iter_domain_vector_type& iteration_vector,
													 bool& prev_flag
													 ) const;

};

/// Data element mapping for sink actor
class smoc_md_loop_snk_data_element_mapper 
  : public smoc_md_loop_data_element_mapper
{

public:
	typedef smoc_md_loop_data_element_mapper::id_type id_type;
  typedef smoc_md_loop_data_element_mapper::iter_domain_vector_type iter_domain_vector_type;
  typedef smoc_md_loop_data_element_mapper::data_element_id_type data_element_id_type;
	typedef smoc_md_loop_data_element_mapper::mapping_type mapping_type;
	typedef smoc_md_loop_data_element_mapper::mapping_vector_type mapping_vector_type;
	typedef smoc_md_loop_data_element_mapper::offset_type offset_type;
	typedef smoc_md_loop_data_element_mapper::offset_vector_type offset_vector_type;

public:
	/// Parameters
	/// mapping_weights, mapping_offset: see parent class
	/// min_data_el_id, max_data_el_id: Description of the token space.
	smoc_md_loop_snk_data_element_mapper(const mapping_vector_type& mapping_weights,
																			 const offset_vector_type& mapping_offset,
																			 const offset_vector_type& min_data_el_id,
																			 const offset_vector_type& max_data_el_id)
		: smoc_md_loop_data_element_mapper(mapping_weights,
																			 mapping_offset),
		min_data_el_id(min_data_el_id),
		max_data_el_id(max_data_el_id)
	{};

public:
  /// Calculate the data element accessed by the given loop-iterator
  /// If this data element is situated on the extended border, 
  /// return the "nearest" non-border data element	
	/// The return value is false, if the data element is a border
	/// data element, else true.
	bool get_src_data_element_id(const smoc_md_loop_iterator_kind& snk_loop_iterator,
															 const iter_domain_vector_type& window_iteration,
															 data_element_id_type& src_data_el_id
															 ) const;

private:
	
	/// Description of the token space
	offset_vector_type min_data_el_id;
	offset_vector_type max_data_el_id;
	
};




#endif
