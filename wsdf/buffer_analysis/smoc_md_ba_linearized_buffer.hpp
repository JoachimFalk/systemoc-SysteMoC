//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_HPP
#define _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_HPP

#include "smoc_md_buffer_analysis.hpp"
#include "smoc_md_ba_linearized_buffer.hpp"
#include "smoc_md_ba_basic_tree.hpp"

#define MAX_VALUE(a,b) ((a) > (b) ? (a) : (b))
#define MIN_VALUE(a,b) ((a) < (b) ? (a) : (b))
#define REM(a,b) ((a) % (b) >= 0 ? (a) % (b) : (a) % (b) + (b))

namespace smoc_md_ba 
{

  class smoc_mb_ba_lin_buffer 
    : public smoc_md_buffer_analysis
  {

  public:
    typedef smoc_md_buffer_analysis parent_type;

  public:
    /// Parameters:
    /// - buffer_height: defines, for how many schedule periods the
    ///   lexicographic order tree shall be initialized
    smoc_mb_ba_lin_buffer(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
			  const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
			  unsigned int buffer_height = 1
			  );

    virtual ~smoc_mb_ba_lin_buffer();


  protected:

    /// Implementation of interface, inhereted from smoc_md_buffer_analysis
    void consumption_update(const iter_domain_vector_type& current_iteration,
			    bool new_schedule_period,
			    const iter_domain_vector_type& consumed_window_start,
			    const iter_domain_vector_type& consumed_window_end
			    );    


  protected:
    iter_domain_vector_type lexorder_smallest_life_data_element;


  private:
    
    /// This function updates the lexicographic order tree due to consumption
    void consumption_tree_update(const iter_domain_vector_type& consumed_window_start,
				 const iter_domain_vector_type& consumed_window_end
				 ); 
    


  private:
    
    typedef signed int abstract_position_id_type;
    typedef long int position_id_type;

  private:

    basic_tree_node<abstract_position_id_type> *lexicographic_order_tree;
    unsigned int tree_levels; //number of tree levels (zero based!)

    //lexicographic order tree: calc number of children
    //level: level of parent node, for which the number 
    //of child nodes shall be calculated (zero-based)
    unsigned int lot_calc_nbr_children(unsigned int level) const;

    
    /* 
       lexicographic order tree: create_child_nodes

       input parameters:
       parent_node: pointer to parent node
       nbr_children: number of children which shall be created
       level: level of parent node (zero-based)
       tree_levels: overall number of tree levels (zero-based)
    */
    bool lot_create_child_nodes(
				basic_tree_node<abstract_position_id_type>* parent_node,
				unsigned int nbr_children,
				unsigned int level, unsigned int tree_levels
				) const;

    /*
      Functions in order to set up the lexicographic order tree, so that
      it correctly represents initial tokens.
     */
    bool lot_set_root_initial_tokens(								 
				     basic_tree_node<abstract_position_id_type>* root_node
				     ) const;

    /// Input parameters:
    ///  lot_node: node to process
    ///  level: level of node in tree, zero based
    ///  abstract_last_initial_data_element: vector specifiying the initial data element
    ///  having the smallest (negative coordinate) in dimension 0 and
    ///  the largest coordinates in the other dimensions
    bool lot_set_initial_tokens(								 
				basic_tree_node<abstract_position_id_type>* lot_node,
				unsigned int level,
				const iter_domain_vector_type& abstract_last_initial_data_element
				)const;    

    // the function returns for a given LOT node the child ID which is
    // required for determination of lexicographically smallest data element
    // Parameters: 
    // lot_node: pointer to node in LOT tree
    // level:    level of node (zero based)
    inline unsigned int 
    get_lexorder_smallest_child_id(const basic_tree_node<abstract_position_id_type>* lot_node,
				   unsigned int level) const;
    
    // the function returns the abstract ID for the lexicographically smallest data element
    bool get_lexorder_smallest_life_data_element(iter_domain_vector_type& abstract_id) const;

    // function creates a new LOT with a new size and moves 
    // the existing one into the new data structure
    bool change_LOT_size(unsigned int new_buffer_height); 

  };

};

#endif //_INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_HPP
