//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_LOOP_HPP
#define _INCLUDED_SMOC_MD_LOOP_HPP

#include "smoc_vector.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <cosupport/smoc_debug_out.hpp>

#ifndef VERBOSE_LEVEL_SMOC_MD_LOOP
#define VERBOSE_LEVEL_SMOC_MD_LOOP 0
// 100: verbose execution
// 101: general debug
// 102: memory access error
// 103: get_req_src_data_element
// 104: calc_eff_window_displacement
// 105: max_data_element_id
// 106: Border processing
#endif


/* **************************************************************************** */
/*                         smoc_md_loop_iterator_kind                           */
/* **************************************************************************** */

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
      _token_dimensions(window_dimensions){
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << "Enter smoc_md_loop_iterator_kind::smoc_md_loop_iterator_kind" << std::endl;
    CoSupport::dout << "Leave smoc_md_loop_iterator_kind::smoc_md_loop_iterator_kind" << std::endl;
#endif
  };

  /// Copy constructor
  smoc_md_loop_iterator_kind(const smoc_md_loop_iterator_kind& src_iterator)
    : current_iteration(src_iterator.current_iteration),
      _token_dimensions(src_iterator._token_dimensions){};

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
  const size_type token_dimensions() const { return _token_dimensions;}

  /// Return number of iterator dimenions
  const data_type iterator_depth() const {return current_iteration.size();}

  /* Determination of iteration borders */

  /// Gets the maximum iteration vector possible for the
  /// current window position.
  /// ONLY returns the iteration levels belonging to the window
  virtual const iter_domain_vector_type& max_window_iteration() const = 0;

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
  const size_type _token_dimensions;  
};






/* ******************************************************************************* */
/*                     smoc_md_loop_data_element_mapper                        */
/* ******************************************************************************* */

/// Common data element mapping for source and sink actor
class smoc_md_loop_data_element_mapper
{

public:

  /// Mapping vector
  typedef unsigned long mapping_type;
  typedef boost::numeric::ublas::matrix<mapping_type> mapping_matrix_type;
  typedef smoc_vector<mapping_type> mapping_vector_type;

  /// Data element identifier
  typedef long id_type;
  typedef smoc_vector<id_type> data_element_id_type;

public:
        
  smoc_md_loop_data_element_mapper(const mapping_matrix_type& mapping_matrix)
    : mapping_matrix(mapping_matrix),
      mapping_table(calc_mapping_table(mapping_matrix))
#if 0
    ,iteration_table(calc_iteration_table(mapping_matrix))
#endif
  {
    assert(check_matrix(mapping_matrix));
  }

  smoc_md_loop_data_element_mapper(const smoc_md_loop_data_element_mapper& mapper)
    : mapping_matrix(mapper.mapping_matrix),
      mapping_table(mapper.mapping_table)
#if 0
    ,iteration_table(mapper.iteration_table)
#endif
  {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << "Enter smoc_md_loop_data_element_mapper::smoc_md_loop_data_element_mapper" << std::endl;
    CoSupport::dout << "Leave smoc_md_loop_data_element_mapper::smoc_md_loop_data_element_mapper" << std::endl;
#endif
  }

protected:
  const mapping_matrix_type mapping_matrix;

protected:

  ///This table indicates for each column of the
  ///mapping matrix which token dimension is influenced
  /// When no token dimension is influenced, the corresponding entry for the
  /// column is -1.
  const smoc_vector<int> mapping_table;

private:

  /// builds a map which assignes to each column of the mapping
  /// matrix which token dimension is influenced
  /// When no token dimension is influenced, the corresponding entry for the
  /// column is -1.
  smoc_vector<int> calc_mapping_table(const mapping_matrix_type& mapping_matrix) const;

#if 0
protected:
        
  /// For each dimension, this table contains a list of columns
  /// identifing the non-zero elements in the mapping matrix.
  const smoc_vector<smoc_vector<unsigned int> > iteration_table;

private:
  smoc_vector<smoc_vector<unsigned int> > 
  calc_iteration_table(const mapping_matrix_type& mapping_matrix) const;
#endif
        

private:

  /// Checks matrix properties
  bool check_matrix(const mapping_matrix_type& mapping_matrix) const;

};




/* ******************************************************************************* */
/*                     smoc_src_md_loop_iterator_kind                        */
/* ******************************************************************************* */


/// Data element mapping for source actor
class smoc_src_md_loop_iterator_kind
  : public smoc_md_loop_iterator_kind, public smoc_md_loop_data_element_mapper
{
public:
  typedef smoc_md_loop_iterator_kind parent_type;

  typedef parent_type::iter_domain_vector_type iter_domain_vector_type;
  typedef parent_type::data_type iter_item_type;

  typedef smoc_md_loop_data_element_mapper::mapping_type mapping_type;

  /// Data element identifier
  typedef smoc_md_loop_data_element_mapper::id_type id_type;
  typedef smoc_md_loop_data_element_mapper::data_element_id_type data_element_id_type;

  /// Offset vector
  typedef unsigned long offset_type;
  typedef smoc_vector<offset_type> mapping_offset_type;

public:
  ///Constructor
  ///Input parameters:
  /// - mapping_matrix, mapping_offset
  smoc_src_md_loop_iterator_kind(const iter_domain_vector_type& iteration_start,
				 const mapping_matrix_type& mapping_matrix,
				 const mapping_offset_type& mapping_offset,
				 const data_element_id_type& max_data_element_id
				 ):
    smoc_md_loop_iterator_kind(mapping_matrix.size1(),iteration_start),
    smoc_md_loop_data_element_mapper(mapping_matrix),
    mapping_offset(mapping_offset),
    _max_data_element_id(max_data_element_id)
  {       
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << "Enter smoc_src_md_loop_iterator_kind::smoc_src_md_loop_iterator_kind" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
    CoSupport::dout << "size of mapping_offset: " << mapping_offset.size() << std::endl;
#endif

    for (unsigned int i = 0; i < mapping_offset.size(); i++){
      assert(mapping_offset[i] >= 0);
    }

    update_base_data_element_id();

#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << CoSupport::Indent::Down;
    CoSupport::dout << "Leave smoc_src_md_loop_iterator_kind::smoc_src_md_loop_iterator_kind" << std::endl;
#endif
  };

  smoc_src_md_loop_iterator_kind(const smoc_src_md_loop_iterator_kind& loop_iterator)
    : smoc_md_loop_iterator_kind(loop_iterator),
      smoc_md_loop_data_element_mapper(loop_iterator),
      mapping_offset(loop_iterator.mapping_offset),
      _max_data_element_id(loop_iterator._max_data_element_id),
      base_data_element_id(loop_iterator.base_data_element_id)
  {}

protected:

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


  /// Function takes a calculated data_element_id.
  /// If it lies beyond the maximum allowed data element of one schedule period
  /// it is corrected correspondingly.
  /// by determining a schedule_period_offset.
  void calc_schedule_period_offset(data_element_id_type& data_element_id,
				   id_type& schedule_period_offset) const;

public:

  virtual bool inc();
        
  /// Calculates the base data element ID for the current iteration.
  /// The schedule period offset is NOT calculated. Instead, data element
  /// identifiers might be returned which are larger than one schedule
  /// period.
  const data_element_id_type& get_base_data_element_id() const;


  /// Calculate the data element offset which is caused by the window
  /// iteration. Note: ONLY the window iteration must be passed as
  /// argument, not the complete iteration vector
  /// The order of the window_iteration must be the same than the
  /// iteration vector itself!
  void get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
				      data_element_id_type& data_element_offset) const;

  /// Returns the data element with the maximum coordinate for the 
  /// current loop iterator
  void max_data_element_id(
			   data_element_id_type& max_data_element_id,
			   id_type& schedule_period_offset
			   ) const;       

  /// Returns the overal maximum data element ID for one schedule period
  const data_element_id_type& schedule_period_max_data_element_id() const;

  /// Returns the size of the token space
  const data_element_id_type size_token_space() const;    

  /// Calculate the source iteration producing the
  /// given data element. The function assumes, that this iteration
  /// is unambiguous.
  /// Function input parameters
  ///  - src_data_el_id: Identifier of the source data element
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

public:
  const mapping_offset_type mapping_offset;
protected:
  const data_element_id_type _max_data_element_id;

protected:
  void update_base_data_element_id();
  data_element_id_type base_data_element_id;

};







/* ******************************************************************************* */
/*                     smoc_snk_md_loop_iterator_kind                        */
/* ******************************************************************************* */


/// Sink loop iterator
class smoc_snk_md_loop_iterator_kind
  : public smoc_md_loop_iterator_kind, public smoc_md_loop_data_element_mapper
{
public:
  typedef smoc_md_loop_iterator_kind::data_type iter_item_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;

  typedef smoc_md_loop_data_element_mapper::mapping_type mapping_type;

  /// Data element identifier
  typedef smoc_md_loop_data_element_mapper::id_type id_type;
  typedef smoc_md_loop_data_element_mapper::data_element_id_type data_element_id_type;

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
  smoc_snk_md_loop_iterator_kind(const iter_domain_vector_type& iteration_start,
				 const mapping_matrix_type& mapping_matrix,
				 const mapping_offset_type& mapping_offset,
				 const border_condition_matrix_type& border_matrix,
				 const border_condition_vector_type& low_border_vector,
				 const border_condition_vector_type& high_border_vector
				 ):
    smoc_md_loop_iterator_kind(mapping_matrix.size1(), iteration_start),
    smoc_md_loop_data_element_mapper(mapping_matrix),
    mapping_offset(mapping_offset),
    border_condition_matrix(border_matrix),
    low_border_condition_vector(low_border_vector),
    high_border_condition_vector(high_border_vector),
    base_border_condition_vector(mapping_matrix.size1())
  {
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << "Enter smoc_snk_md_loop_iterator_kind::smoc_snk_md_loop_iterator_kind" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
    CoSupport::dout << "size of mapping-offset: " << mapping_offset.size() << std::endl;
#endif
    assert(check_border_condition_matrix(border_matrix));
    update_base_data_element_id();
    update_base_border_condition_vector();
#if VERBOSE_LEVEL_SMOC_MD_LOOP == 102
    CoSupport::dout << "Leave smoc_snk_md_loop_iterator_kind::smoc_snk_md_loop_iterator_kind" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
  };

  smoc_snk_md_loop_iterator_kind(const smoc_snk_md_loop_iterator_kind& iterator)
    : smoc_md_loop_iterator_kind(iterator),
      smoc_md_loop_data_element_mapper(iterator),
      mapping_offset(iterator.mapping_offset),
      border_condition_matrix(iterator.border_condition_matrix),
      low_border_condition_vector(iterator.low_border_condition_vector),
      high_border_condition_vector(iterator.high_border_condition_vector),
      base_data_element_id(iterator.base_data_element_id),
      base_border_condition_vector(iterator.base_border_condition_vector)
  {}


protected:
  /// Input parameters:
  /// - iteration_vector:       loop iteration, for which the accessed data element
  ///                           shall be calculated.
  /// Output parameters:
  /// - data_element_id:        accessed data element ID
  void get_data_element_id(const iter_domain_vector_type& iteration_vector,
			   data_element_id_type& data_element_id) const;

public: 

  virtual bool inc();

  /// Calculates the base data element ID for the current
  /// iteration, without taking window iteration into account
  const data_element_id_type& get_base_data_element_id() const;

  /// Same as above, but only for a given token-dimension
  const id_type get_base_data_element_id(unsigned int token_dimension) const;

  /// Calculate the data element offset which is caused by the window
  /// iteration. Note: ONLY the window iteration must be passed as
  /// argument, not the complete iteration vector
  /// The order of the window_iteration must be the SAME than in the
  /// iterator itself.
  void get_window_data_element_offset(const iter_domain_vector_type& window_iteration,
				      data_element_id_type& data_element_offset) const;
        
  /// This function determines the data element which is required for execution of
  /// the current sink iteration and and which is produced latest by the source actor.
  /// The function returns 'false' when no data element produced by the source actor
  /// is required. Otherwise 'true' is returned.
  bool get_req_src_data_element(data_element_id_type& data_element_id) const;

  /// This function determines whether the current loop iterator
  /// has its maximum position regarding the given token dimension
  /// By default, the window iteration is ignored
  bool is_iteration_max(
			unsigned token_dimension,
			bool ignore_window_iteration = true
			) const;

  /// This function calculates the window displacement from the SOURCE 
  /// POINT of view. Due to border processing this is not identical
  /// with the displacement from the sink point of view.
  /// If the return-value is false, the iterator is for the given dimension
  /// at the end of the schedule period and the value of window_displacement
  /// is not valid. Otherwise the return-value is true.
  bool calc_eff_window_displacement(
				    unsigned token_dimension,
				    mapping_type& window_displacement
				    ) const;

  /// This function calculates the window iterations identifing the
  /// data elements which are read the last time.
  /// The return value is false, when no data elements are read the
  /// last time.
  ///
  /// Input parameters: 
  ///  - max_data_element_id: Maximum data element of the token space
  /// Output paraemters:
  /// - consumed_window_start, consumed_window_end:
  ///   Window iterators describing the consumed data elements.  
  bool calc_consumed_window_iterations(
				       const data_element_id_type& max_data_element_id,
				       iter_domain_vector_type& consumed_window_start,
				       iter_domain_vector_type& consumed_window_end
				       ) const;

  /// This function calculates the number of border pixels which are situated
  /// on the lower extended border
  /// ATTENTION: If the extended border is larger than the sliding window,
  /// the returned value might be larger than the sliding window extendion.
  id_type calc_num_low_border_pixels(unsigned int token_dimension) const;


  /// This function calculates the window iteration in the given dimension
  /// which would be necessary in order to obtain the specified coordinate
  iter_item_type get_window_iteration(unsigned int token_dimension, id_type coord) const;


protected:
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
  const border_condition_vector_type& get_base_border_condition_vector() const{
    return base_border_condition_vector;
  }

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

protected:
  void update_base_data_element_id();
  data_element_id_type base_data_element_id;

  void update_base_border_condition_vector();
  border_condition_vector_type base_border_condition_vector;
        

};





/* ******************************************************************************* */
/*                          smoc_md_static_loop_iterator                           */
/* ******************************************************************************* */


class smoc_md_static_loop_iterator 
{
public:

  //Specification of iteration domain
  typedef smoc_md_loop_iterator_kind::data_type iter_item_type;
  typedef smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;
  typedef smoc_md_loop_iterator_kind::size_type size_type;

public:
  /* Constructors */

  /// base_data_element_id points to the corresponding entry in the data
  /// element mapper.
  smoc_md_static_loop_iterator(const iter_domain_vector_type& iteration_max,
			       unsigned int token_dimensions);

  smoc_md_static_loop_iterator(const smoc_md_static_loop_iterator& src_iterator);

  virtual ~smoc_md_static_loop_iterator() {}

public:

  /// Calculation of some special window iterations
  virtual const iter_domain_vector_type& max_window_iteration() const;

  virtual iter_item_type iteration_max(const size_type dimension,
				       const iter_domain_vector_type& fixed_iteration) const {
    return _iteration_max[dimension];
  }

  virtual const iter_domain_vector_type iteration_max() const{
    return _iteration_max;
  }

protected:  

  //Iteration bounds
  const iter_domain_vector_type _iteration_max;

  //Maximum window iteration
  const iter_domain_vector_type _max_window_iteration;

private:
  const iter_domain_vector_type calc_max_window_iteration(unsigned int token_dimensions,
							  const iter_domain_vector_type& iteration_max);
};









/* ******************************************************************************* */
/*                           smoc_src_md_static_loop_iterator                      */
/* ******************************************************************************* */


/// Description of a static nested loop
/// The iteration bounds are independent on dimension i are
/// independent of the other dimensions.
class smoc_src_md_static_loop_iterator 
  : public smoc_src_md_loop_iterator_kind, public smoc_md_static_loop_iterator
{
public:
  // Typedefs
  typedef smoc_src_md_loop_iterator_kind parent_type;

        

  //Specification of iteration domain
  typedef parent_type::iter_item_type iter_item_type;
  typedef parent_type::iter_domain_vector_type iter_domain_vector_type;

  typedef parent_type::size_type size_type;

  typedef parent_type::mapping_type mapping_type;

public:
  /* Constructors */

  /// This constructor allows to declare a so called
  /// window-iteration domain. The window iteration domain
  /// Is supposed to be executed externally.
  smoc_src_md_static_loop_iterator(const iter_domain_vector_type& iteration_max,
				   const mapping_matrix_type& mapping_matrix,
				   const mapping_offset_type& mapping_offset
				   );

  smoc_src_md_static_loop_iterator(const smoc_src_md_static_loop_iterator& src_iterator);

private:
  data_element_id_type calc_max_data_element_id(const iter_domain_vector_type& iteration_max,
						const mapping_matrix_type& mapping_matrix
						) const;

public:

  bool inc();

  virtual iter_item_type iteration_max(const size_type dimension,
				       const iter_domain_vector_type& fixed_iteration) const {
    return smoc_md_static_loop_iterator::iteration_max(dimension,fixed_iteration);
  }

  virtual const iter_domain_vector_type iteration_max() const{
    return smoc_md_static_loop_iterator::iteration_max();
  }

  virtual const iter_domain_vector_type& max_window_iteration() const{
    return smoc_md_static_loop_iterator::max_window_iteration();
  }
        
  
};



/* ******************************************************************************* */
/*                           smoc_snk_md_static_loop_iterator                      */
/* ******************************************************************************* */

class smoc_snk_md_static_loop_iterator 
  : public smoc_snk_md_loop_iterator_kind, public smoc_md_static_loop_iterator
{
public:
  // Typedefs
  typedef smoc_snk_md_loop_iterator_kind parent_type;

  //Specification of iteration domain
  typedef parent_type::iter_item_type iter_item_type;
  typedef parent_type::iter_domain_vector_type iter_domain_vector_type;

  typedef parent_type::size_type size_type;

  typedef parent_type::mapping_type mapping_type;
public:
  /* Constructors */

  /// This constructor allows to declare a so called
  /// window-iteration domain. The window iteration domain
  /// Is supposed to be executed externally.
  smoc_snk_md_static_loop_iterator(
				   const iter_domain_vector_type& iteration_max,
				   const mapping_matrix_type& mapping_matrix,
				   const mapping_offset_type& mapping_offset,
				   const border_condition_matrix_type& border_matrix,
				   const border_condition_vector_type& low_border_vector,
				   const border_condition_vector_type& high_border_vector
				   );

  smoc_snk_md_static_loop_iterator(const smoc_snk_md_static_loop_iterator& snk_iterator);

public:

  virtual bool inc();

  virtual iter_item_type iteration_max(const size_type dimension,
				       const iter_domain_vector_type& fixed_iteration) const {
    return smoc_md_static_loop_iterator::iteration_max(dimension,fixed_iteration);
  }

  virtual const iter_domain_vector_type iteration_max() const{
    return smoc_md_static_loop_iterator::iteration_max();
  }

  virtual const iter_domain_vector_type& max_window_iteration() const{
    return smoc_md_static_loop_iterator::max_window_iteration();
  }

};



#endif
