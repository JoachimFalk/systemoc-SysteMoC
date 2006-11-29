#ifndef _INCLUDED_SMOC_WSDF_EDGE_HPP
#define _INCLUDED_SMOC_WSDF_EDGE_HPP

#include "smoc_vector.hpp"
#include <boost/numeric/ublas/matrix.hpp>


/// Descriptor of a WSDF edge
class smoc_wsdf_edge_descr {
public:
	///signed data type
	typedef long sdata_type;

	///unsigned data type
	typedef unsigned long udata_type;

	/// signed vector
	typedef smoc_vector<sdata_type> svector_type;

	///vector of svector_type
	typedef smoc_vector<svector_type> s2vector_type;

	/// unsigned vector
	typedef smoc_vector<udata_type> uvector_type;

	///vector of uvector_type
	typedef smoc_vector<uvector_type> u2vector_type;

	///signed matrix
	typedef boost::numeric::ublas::matrix<sdata_type> smatrix_type;

	///unsigned matrix
	typedef boost::numeric::ublas::matrix<udata_type> umatrix_type;


public:
	/* Constructor */
	/// Input parameters
	/// token_dimensions: number of dimensions for a token
	/// src_firing_blocks   : Describes the size of the firing blocks.
  ///                       The first vector describes the smallest block
	///                       and describes the effective token size.
	/// snk_firing_blocks   : Describes the size of the firing blocks.
	/// u0                  : Size of the virtual token union (in number of data elements)
  ///                       WITHOUT extended border!
	/// c                   : Size of sliding window
	/// delta_c             : Vector describing the window displacement
	/// d                   : Initial tokens
	/// bs, bt              : Border processing	
	smoc_wsdf_edge_descr(unsigned token_dimensions,
											 const u2vector_type& src_firing_blocks,
											 const u2vector_type& snk_firing_blocks,
											 const uvector_type& u0,
											 const uvector_type& c,
											 const uvector_type& delta_c,
											 const uvector_type& d,
											 const svector_type& bs,
											 const svector_type& bt)
		: token_dimensions(token_dimensions),
			src_firing_blocks(src_firing_blocks),
			src_num_firing_levels(src_firing_blocks.size()),
			snk_firing_blocks(snk_firing_blocks),
			snk_num_firing_levels(snk_firing_blocks.size()),
			p(this->src_firing_blocks[0]),
			v(u0),u0(u0),
			c(c),
			delta_c(delta_c),
			d(d),
			bs(bs),
			bt(bt)			
	{
		check_parameters();
	}

public:

	uvector_type snk_iteration_max() const;
	uvector_type src_iteration_max() const;

	svector_type snk_data_element_mapping_vector() const;
	umatrix_type snk_data_element_mapping_matrix() const;

	uvector_type src_data_element_mapping_vector() const;
	umatrix_type src_data_element_mapping_matrix() const;

	smatrix_type calc_border_condition_matrix() const;
	svector_type calc_low_border_condition_vector() const;
	svector_type calc_high_border_condition_vector() const;

	uvector_type max_data_element_id() const;


	/// This function generates a table which assigns to
	/// each firing block the corresponding iteration level
	/// The iteration over the sliding window is NOT included.
	/// Suppose, that the function returns return_vector.
	/// Then return_vector[firing_level][token_dimension] returns the
	/// iteration level for the given firing_level and token_dimensions.
	/// Note, that the smallest firing block has the smallest firing level.
	/// If return_vector[firing_level][token_dimension] < 0, then no
	/// iteration level exists.
	s2vector_type calc_snk_iteration_level_table() const;

	/// Returns the corresponding table for the source. Note,
	/// That the iteration over the effective token is NOT
	/// included.
	s2vector_type calc_src_iteration_level_table() const;
											

public:

	/* WSDF edge parameters */
	const unsigned token_dimensions;

protected:

	/// Actor invocation order (firing blocks)
	const u2vector_type src_firing_blocks;
	const unsigned src_num_firing_levels;
	
	const u2vector_type snk_firing_blocks;
	const unsigned snk_num_firing_levels;

	/* Source */
	/// Size of effective token
	const uvector_type& p;	

	/// Size of virtual token
	const uvector_type v;

	/// Size of virtual token union (WITHOUT extended border)
	const uvector_type u0;

	/* Sink */
	/// Window size
	const uvector_type c;

	/// Window displacement
	const uvector_type delta_c;

	
	/* Edge */
	/// Initial data elements
	const uvector_type d;

	/// Border processing
	const svector_type bs;
	const svector_type bt;

private:
	
	/// Vector of bools
	typedef smoc_vector<bool> bvector_type;

private:

	///Calculate number of invocations per virtual token union
	uvector_type calc_snk_r_vtu() const;
	uvector_type calc_src_r_vtu() const;

	/// Verify local balance equation
	void check_local_balance() const;
	
	/// Check WSDF parameters
	void check_parameters() const;


	/// check for a firing level, whether it must be covered by
	/// an iteration level
	/// The smallest firing block has the smallest firing level
	bool snk_has_iteration_level(unsigned firing_level, 
															 unsigned token_dimension) const;
	bool src_has_iteration_level(unsigned firing_level, 
															 unsigned token_dimension) const;
	
	/// This function inserts int a given iteration level table
	/// the iteration for the virtual token union if necessary
	///
	/// Input parameters: 
	/// - snk_iteration_level_table: Describes the iteration 
	///   level for each firing block
	/// Output parameters:
	/// - snk_iteration_level_table: modified iteration_level_table
	/// - snk_vtu_iteration_level:   specifies the iteration levels
	///                              which cover the firing block corresponding
	///                              to a virtual token union.
	/// - new_vtu_iteration          is true, if an additional iteration
	///                              for the virtual token union 
	///                              has been inserted
	void insert_snk_vtu_iterations(s2vector_type& snk_iteration_level_table,
																 uvector_type& snk_vtu_iteration_level,
																 bvector_type& new_vtu_iteration
																 ) const;
	void insert_snk_vtu_iterations(s2vector_type& snk_iteration_level_table,
																 uvector_type& snk_vtu_iteration_level
																 ) const;
	

	
	/// The function returns, how many iteration levels are described by the
	/// iteration level table
	/// NOTE: The iteration over the sliding window is not included
	unsigned get_num_iteration_levels(const s2vector_type& snk_iteration_level_table,
																		const uvector_type& snk_vtu_iteration_level
																		) const;
	unsigned calc_src_iteration_levels() const;


	/// Calculates the maximum for each iteration level
	uvector_type calc_snk_iteration_max(const s2vector_type& snk_iteration_level_table,
																			const uvector_type& snk_vtu_iteration_level) const;


	/// Appends the iteration over the sliding window
	/// The the maximum iteration table
	void append_snk_window_iteration(uvector_type& iteration_max) const;
		

	/// Calculate the sink data element mapping matrix
	/// Includes the iterations over the window
	umatrix_type calc_snk_data_element_mapping_matrix(const s2vector_type& snk_iteration_level_table,
																										const uvector_type& snk_vtu_iteration_level,
																										const uvector_type& snk_iter_max
																										) const;

	/// Inserts the data element mapping for the snk window
	void insert_snk_window_mapping(umatrix_type& data_element_mapping_matrix) const;

	
	/// This function returns the condition matrix for the lower extended
	/// border by which we can detect whether a data element is situated on the
	/// extended border or not.
	/// A data element is situated on the lower extended border, when
	/// cond_matrix * (iteration_vector) < low_ext_border_cond_vector for one component.
	/// A data element is situated on the higher extended border, when
	/// cond_matrix * (iteration_vector) > high_ext_border_cond_vector
	void calc_border_condition_matrix(const umatrix_type& mapping_matrix,
																		const uvector_type& snk_vtu_iteration_level,
																		smatrix_type& border_cond_matrix
																		) const;


	/// This function allows to thin out a matrix
	/// To do so, the function checks, which iteration indeces have
	/// a maximum of zero. The corresponding mapping coefficients
	/// can be set to zero,
	template <typename matrix_type>
	void matrix_thin_out(matrix_type& mapping_matrix, 
											 const uvector_type& iteration_max) const {
		for(unsigned int col = 0; col < iteration_max.size(); col++){
			if (iteration_max[col] == 0){
				for(unsigned int row = 0; row < mapping_matrix.size1(); row++){
					mapping_matrix(row,col) = 0;
				}
			}
		}
	}

};


#endif
