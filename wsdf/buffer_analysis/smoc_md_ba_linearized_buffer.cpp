//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include "smoc_md_ba_linearized_buffer.hpp"

#ifndef SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF
# define SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF 0
#endif

namespace smoc_md_ba {

  smoc_mb_ba_lin_buffer::smoc_mb_ba_lin_buffer(const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
					       const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
					       unsigned int buffer_height
					       )
    : smoc_md_buffer_analysis(src_md_loop_iterator,snk_md_loop_iterator),
      lexorder_smallest_life_data_element(src_iterator_depth)
      
  {
    //Number of initial virtual token unions in highest dimension
    unsigned int num_initial_schedule_periods;
    abstract_position_id_type intial_node_value;

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF >= 1
    CoSupport::dout << "Enter smoc_mb_ba_lin_buffer::smoc_mb_ba_lin_buffer" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
	

  /* establish tree for lexicographic order */
    
  //Create root node
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Create LOT root node" << std::endl;
#endif

    //calculate the initial root value (divide and round up)
    num_initial_schedule_periods = 
      (src_mapping_offset[token_dimensions-1] + size_token_space[token_dimensions-1] - 1) / 
      (size_token_space[token_dimensions-1]);
    
    //Check, if buffer height is sufficient to represent all initial data elements
    if (num_initial_schedule_periods > buffer_height){
      //buffer height is not sufficient
      //divide and round up
      buffer_height = num_initial_schedule_periods; 
    }
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Initial buffer height: " << buffer_height << std::endl;
#endif


    //set initial root value
    intial_node_value = -num_initial_schedule_periods;
    //allocate LOT root
    lexicographic_order_tree = 
      new basic_tree_node<abstract_position_id_type>(buffer_height,
						     &intial_node_value);

    ASSERT(lexicographic_order_tree != NULL,"memory error");
    
    /* calculate the number of tree_levels (zero based!) */
    tree_levels = src_iterator_depth - 1;
    while((tree_levels >= 1) && 
	  (lot_calc_nbr_children(tree_levels) <= 1)){
      //if a node has only one child, it can be removed, because
      //of its triviality
      tree_levels--;
    }
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Create tree with " << tree_levels << " levels" << std::endl;
    CoSupport::dout << "create children" << std::endl;
#endif
    
    //create children
    if (tree_levels > 0){
      lot_create_child_nodes(lexicographic_order_tree,
			     buffer_height,0,tree_levels);
    }
    
    if (src_mapping_offset[token_dimensions-1] > 0){
      //init tree
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << "init tree due initial data elements" << std::endl;
#endif
      lot_set_root_initial_tokens(lexicographic_order_tree);
    }

    
    //Get initial lexicographically smallest data element
    get_lexorder_smallest_life_data_element(lexorder_smallest_life_data_element);

	
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF >= 1
    CoSupport::dout << "Leave smoc_mb_ba_lin_buffer::smoc_mb_ba_lin_buffer" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif

  }
  
  smoc_mb_ba_lin_buffer::~smoc_mb_ba_lin_buffer(){
    //free memory for tree
    lexicographic_order_tree->delete_children(true);

    delete lexicographic_order_tree;
  }

  
  void 
  smoc_mb_ba_lin_buffer::consumption_update(const iter_domain_vector_type& current_iteration,
					    bool new_schedule_period,
					    const iter_domain_vector_type& consumed_window_start,
					    const iter_domain_vector_type& consumed_window_end
					    ){

    consumption_tree_update(consumed_window_start,
			    consumed_window_end);

    //store smallest lexicographic data element
    get_lexorder_smallest_life_data_element(lexorder_smallest_life_data_element);
			    
  }


  void 
  smoc_mb_ba_lin_buffer::consumption_tree_update(const iter_domain_vector_type& consumed_window_start,
						 const iter_domain_vector_type& consumed_window_end
						 ){

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Enter smoc_mb_ba_lin_buffer::consumption_tree_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;

    CoSupport::dout << "consumed_window_start = " 
		    << consumed_window_start << std::endl;
    CoSupport::dout << "consumed_window_end = " 
		    << consumed_window_end << std::endl;
#endif    
    
    //in hierarchical coordinates
    //expressed by source coordinates
    iter_domain_vector_type current_src_window_element(src_iterator_depth);
    
    //lexicographic tree nodes
    basic_tree_node<abstract_position_id_type>* lot_node;
    basic_tree_node<abstract_position_id_type>* lot_node2;
  
    //currently processed level in LOT (Lexicographic Order Tree)
    int lot_level; 

    bool finished = false;

    /* 
       process all data elements "between" 
       first_consumed_src_data_element_id and last_consumed_src_data_element_id
    
       start with first_consumed_src_data_element_id
    */
                                 
    //node: the order by which we process the consumed data elements is essential for the
    //      correct function of the algorithm. In principle it would be advantageous to
    //      start with the last consumed source data element, because in this case several
    //      nodes have to be processed only once instead of several times. However, in this
    //      case, it is necessary to verify, that due to an updating-operation we do not overwrite
    //      a node value which has already been modified by a previous operation.
    //
    //      This in principle is quite simple
    //      by checking, if the node value is larger than that value to which it would be set
    //      during the considered update operation. As the node values in principle must 
    //      increase, this is not allowed and we can stop update operation. 
    //
    //      However, after having performed a reset of a node this approach
    //      does not work anymore. Hence, we start with the first consumed data element in order
    //      to keep things simple.

    //current processed window element
    iter_domain_vector_type current_window_element(consumed_window_start);
    
    while(!finished){
      //convert into UNIQUE hierarchical data element identifiers
      get_src_loop_iteration(current_window_element,
			     current_src_window_element);
      

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << "Process data element :";
      CoSupport::dout << current_src_window_element;
      CoSupport::dout << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
#endif

    
      //check, if the number of first level nodes is sufficient
      if (unsigned(current_src_window_element[0] - lexicographic_order_tree->getNodeValue() + 1) 
	  > lexicographic_order_tree->nbr_children){
	//LOT size not sufficient!
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF >= 2
	CoSupport::dout << "LOT size not sufficient. Increase size." << std::endl;
#endif
	change_LOT_size(2*lexicographic_order_tree->nbr_children);
      }
  
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << "Get lot node of data element" << std::endl;
#endif
      if (tree_levels > 0){
	//get LOT node of currently processed data element
	lot_node = lexicographic_order_tree->getChildNode(REM(current_src_window_element[0], 
							      lexicographic_order_tree->nbr_children));
	ASSERT(lot_node != NULL, "Could not find LOT node\n");
	lot_node = lot_node->findChildNode(current_src_window_element,tree_levels-1,1);
	ASSERT(lot_node != NULL, "Could not find LOT node\n");
      }else{
	lot_node = lexicographic_order_tree;
      }
  
      //modify value of LOT node and its parents if necessary
      lot_node2 = lot_node;
      for(lot_level = tree_levels; lot_level >= 0; lot_level--){
	if (lot_level == 0){
	  //we arrived at the root of the tree. Due to the modulo 
	  //access on its children a special processing is necessary

	  //we can free one (or more) LOT nodes of level 1	  
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	  CoSupport::dout << "Update LOT node value. Old value: ";
	  CoSupport::dout << lot_node2->getNodeValue();
#endif
	  lot_node2->setNodeValue(current_src_window_element[lot_level]+1);
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	  CoSupport::dout << "; new value: ";
	  CoSupport::dout << lot_node2->getNodeValue() << std::endl;
#endif
	  
	}else{
	  if((unsigned)current_src_window_element[lot_level]+1 >= lot_node2->nbr_children){
	    //current node has been terminated
	    //re-initialise for next usage
	    lot_node2->setNodeValue(0);
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	    CoSupport::dout << "node of level " << lot_level << " has been terminated." << std::endl;
#endif
	  }else{
	    //set node value to next life data elememt
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	    CoSupport::dout << "Update LOT node value for level " << lot_level << ". Old value: ";
	    CoSupport::dout << lot_node2->getNodeValue();
#endif
	    lot_node2->setNodeValue(current_src_window_element[lot_level]+1);
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	    CoSupport::dout << "; new value: ";
	    CoSupport::dout << lot_node2->getNodeValue() << std::endl;
#endif
	    break;
	  }
	} //lot_level == 0 ?
    
	//get parent
	lot_node2 = lot_node2->getParentNode();
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
	CoSupport::dout << "move to parent node" << std::endl;
#endif
      }//for lot_level

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << CoSupport::Indent::Down;
      CoSupport::dout << "Determine next data element to process" << std::endl;
#endif

      finished = true;
      for(unsigned int i = 0; i < token_dimensions; i++){
	//note: the order by which the next data element is search is essential
	//for the correct function of the program. We are committing errors,
	//if in a given dimension of the abstract token space a data element with larger
	//coordinate is processed before one with a smaller one. By the chosen search order
	//this can never happen.
	current_window_element[i]++;
	if (current_window_element[i] > consumed_window_end[i]){
	  current_window_element[i] = consumed_window_start[i];
	}else{
	  //we have found data element
	  finished = false;
	  break;
	}
      }
      
    }
  consumption_update_buffer_pointers_end:
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Leave smoc_mb_ba_lin_buffer::consumption_tree_update" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return;
  }



  unsigned int 
  smoc_mb_ba_lin_buffer::lot_calc_nbr_children(unsigned int level) const{
    if (level == 0){
      //calc number of children of root node
      //not known here
      return 0;
    }

    return src_iteration_max(level)+1;
  }
  


  bool 
  smoc_mb_ba_lin_buffer::lot_create_child_nodes(								 
						basic_tree_node<abstract_position_id_type>* parent_node,
						unsigned int nbr_children,
						unsigned int level, unsigned int tree_levels	 
						)const{
  

    unsigned int nbr_child_children;	
    basic_tree_node<abstract_position_id_type>* child_node;
    const abstract_position_id_type initial_child_value = 0;
    
    const unsigned int child_level = level + 1;

    bool return_value = true;
	
    //calculate number of children of child
    nbr_child_children = lot_calc_nbr_children(child_level);
    //note, that nbr_child_children-1 
    //represents also the maximum possible node value!
    //(except for the tree root)


    //create number of requested children
    for(unsigned int child_id = 0; child_id < nbr_children; child_id++){
      //create a new child node
      child_node = new basic_tree_node<abstract_position_id_type>(nbr_child_children,
								  &initial_child_value);
      if (child_node == NULL){
	//memory error
	return false;
      }

      //register child at parent node
      return_value |= parent_node->setChildNode(child_id,child_node);

      //create children of child node if necessary
      if ((nbr_child_children > 0) && (child_level < tree_levels)){
	//for the last tree level, we do not need to create children
	//but we let them empty
	return_value |= 
	  lot_create_child_nodes(child_node,nbr_child_children,level+1,tree_levels);
      }
    }
    
    return return_value;
    
  }  


  bool 
  smoc_mb_ba_lin_buffer::lot_set_root_initial_tokens(								 
						     basic_tree_node<abstract_position_id_type>* root_node
						     ) const {
    
    basic_tree_node<abstract_position_id_type>* child_node;

    //Initial data element with largest coordinates
    data_element_id_type last_initial_data_element(token_dimensions);
    iter_domain_vector_type abstract_last_initial_data_element(src_iterator_depth);

    bool return_value = true;

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Initialization of LOT tree due to initial value" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
	
    //convert the initial token with the largest coordinate into
    //hierarchical data element identifiers
    for(unsigned int i = 0; i < token_dimensions-1; i++){
      ASSERT(src_mapping_offset[i] == 0,
	     "only hyperplanes orthogonal to en are currently supported");
      last_initial_data_element[i] = size_token_space[i]-1;
    }
	
    id_type schedule_period_offset;
    last_initial_data_element[token_dimensions-1] = 
      src_mapping_offset[token_dimensions-1];
    last_initial_data_element[token_dimensions-1] *= -1; //Abstract data elements have negative coordinates
    get_src_loop_iteration(last_initial_data_element,
			   abstract_last_initial_data_element,
			   schedule_period_offset
			   );

    // As we need unique Data element Identifiers
    abstract_last_initial_data_element[0] += 
      schedule_period_offset * src_iteration_max[0];
    
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Last initial data element: ";
    CoSupport::dout << last_initial_data_element;
    CoSupport::dout << std::endl;

    CoSupport::dout << "Abstract last initial data element: ";
    CoSupport::dout << abstract_last_initial_data_element << std::endl;      
#endif

    ASSERT(root_node->nbr_children >= unsigned(-abstract_last_initial_data_element[0]),
	   "Tree size insufficient for initial tokens");
    root_node->setNodeValue(abstract_last_initial_data_element[0]);
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Setting root node value to " 
	 << root_node->getNodeValue() << std::endl;
#endif


    /*
      Init children
    */
    //only the firing blocks which are partly filled with initial tokens
    //need to be initialized in a special way
    child_node = root_node->getChildNode(get_lexorder_smallest_child_id(root_node,0));

    if(child_node != NULL){
      return_value = lot_set_initial_tokens(child_node,1,abstract_last_initial_data_element);
    }else{
      return_value = true;
    }

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << CoSupport::Indent::Down;
#endif

    return return_value;
    
  }



  bool 
  smoc_mb_ba_lin_buffer::lot_set_initial_tokens(								 
						basic_tree_node<abstract_position_id_type>* lot_node,
						unsigned int level,
						const iter_domain_vector_type& abstract_last_initial_data_element
						)const{
	
    basic_tree_node<abstract_position_id_type>* child_node;
    abstract_position_id_type initial_node_value;

    bool return_value = true;

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Initialization of level " << level << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
	

    if ((level % token_dimensions) == 0){
      //coordinate belongs to cartesian coordinate en
      //children might be only partially filled with initial tokens

      //set node value to first element influenced by initial tokens
      initial_node_value = abstract_last_initial_data_element[level];
      lot_node->setNodeValue(initial_node_value);
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << "Set node of level to " << initial_node_value << std::endl;		
#endif
		
      //get child, which might be only partially filled
      child_node = lot_node->getChildNode(initial_node_value);

      if (child_node != NULL){
	return_value =  lot_set_initial_tokens(child_node,level+1,
					       abstract_last_initial_data_element);
      }else{
	return_value = true;
      }
		
    }else{
      //dimensions are completely filled with initial tokens
      for(unsigned int child_id = 0; child_id < lot_node->nbr_children; child_id++){
	child_node = lot_node->getChildNode(child_id);
	if (child_node != NULL){
	  return_value &= lot_set_initial_tokens(child_node,level+1,
						 abstract_last_initial_data_element);
	}
      }
    }	

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << CoSupport::Indent::Down;
#endif
	
    return return_value;
  }

  

  unsigned int 
  smoc_mb_ba_lin_buffer::get_lexorder_smallest_child_id(
							const basic_tree_node<abstract_position_id_type>* lot_node,
							unsigned int level
							) const{
    
    
    unsigned int return_value;

    if (level == 0){
      return_value = REM(lot_node->getNodeValue(),lot_node->nbr_children);
    }else{
      return_value = lot_node->getNodeValue();
    }

    return return_value;

  }

  

  bool smoc_mb_ba_lin_buffer::get_lexorder_smallest_life_data_element(iter_domain_vector_type& abstract_id) const{
    basic_tree_node<abstract_position_id_type> *lot_node = lexicographic_order_tree;
    char debug_buffer[100];

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << "Determine lexicographically smallest data element" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
  
    for(unsigned int i=0; i <= tree_levels; i++){
      sprintf(debug_buffer,
	      "illegal LOT node during determination of lexic. smallest life data element (i=%d)",
	      i);
      ASSERT(lot_node != NULL,debug_buffer);
      abstract_id[i] = lot_node->getNodeValue();
#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
      CoSupport::dout << "node value level: " << i << ": " << abstract_id[i] << std::endl;
      //CoSupport::dout << "Corresponding child ID: " << get_lexorder_smallest_child_id(lot_node,i) << std::endl;
      //CoSupport::dout << "Number of children: " << lot_node->nbr_children << std::endl;
#endif
      lot_node = lot_node->getChildNode(get_lexorder_smallest_child_id(lot_node,i));
    }

    //the rest of the coordinates is zero!
    for(unsigned int i = tree_levels+1; i < src_iterator_depth; i++){
      abstract_id[i] = 0;
    }

#if SMOC_VERBOSE_LEVEL_MD_BA_LIN_BUF == 105
    CoSupport::dout << CoSupport::Indent::Down;
#endif

    return true;
  }



  bool smoc_mb_ba_lin_buffer::change_LOT_size(unsigned int new_buffer_height){
    basic_tree_node<abstract_position_id_type>* new_LOT_tree;
    basic_tree_node<abstract_position_id_type>* lot_node;
    abstract_position_id_type temp;
    bool return_value = true;

    ASSERT(new_buffer_height > lexicographic_order_tree->nbr_children,
	   "currently, LOT reduction is not supported");

    //create a new root node and copy value
    temp = lexicographic_order_tree->getNodeValue();
    new_LOT_tree = 
      new basic_tree_node<abstract_position_id_type>(new_buffer_height, 
						     &temp);

    //move existing child nodes
    for(unsigned int i=0; i < lexicographic_order_tree->nbr_children; i++){
      //get child node
      lot_node = lexicographic_order_tree->getChildNode(REM(temp,lexicographic_order_tree->nbr_children));

      //remove from old LOT
      lexicographic_order_tree->removeChildNode(REM(temp,lexicographic_order_tree->nbr_children));

      //add to new LOT
      return_value |= new_LOT_tree->setChildNode(REM(temp,new_LOT_tree->nbr_children),lot_node);

      //next value
      temp++;
    }
    
    //create missing childs
    if(tree_levels > 0){
      for(unsigned int i=lexicographic_order_tree->nbr_children; 
	  i < new_LOT_tree->nbr_children; 
	  i++){
	const abstract_position_id_type zero = 0;
	lot_node = new basic_tree_node<abstract_position_id_type>(lot_calc_nbr_children(1),&zero);
    
	//create children
	if(tree_levels > 1)
	  lot_create_child_nodes(lot_node,lot_calc_nbr_children(1), 1, tree_levels);

	//register node
	return_value |= new_LOT_tree->setChildNode(REM(temp,new_LOT_tree->nbr_children),lot_node);
			
	//next value
	temp++;    
      }
    }

    //destroy old root node
    delete lexicographic_order_tree;

    //and store new one
    lexicographic_order_tree = new_LOT_tree;

    return return_value;
  }
  
  
  
  




























};
