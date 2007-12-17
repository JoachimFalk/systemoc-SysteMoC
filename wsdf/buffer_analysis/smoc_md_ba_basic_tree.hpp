//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_BA_BASIC_TREE_HPP
#define _INCLUDED_SMOC_MD_BA_BASIC_TREE_HPP

#ifndef ASSERT
# include <string>
# include <exception>
# define ASSERT(condition,error_string) if (!(condition)) throw std::string((error_string));
#endif

namespace smoc_md_ba 
{

  template <typename data_type>
  class basic_tree_node {

  public:
    basic_tree_node(
		    unsigned int nbr_children, 
		    const data_type *initValue = NULL);
    ~basic_tree_node();

  public:
    //get pointer to parent node
    basic_tree_node<data_type>* getParentNode(void) const;
    //get pointer to child node
    basic_tree_node<data_type>* getChildNode(unsigned int child_id) const;

    //Register a child at its parent by setting corresponding pointers
    bool setChildNode(unsigned int child_id,
		      basic_tree_node<data_type> *child_node);
    //remove child node
    bool removeChildNode(unsigned int child_id);
    //remove and delete child node (and its children)
    bool deleteChildNode(unsigned int child_id, bool recursive = false);

  public:
    //deletes all child objects
    bool delete_children(bool recursive = false);

  public:
    bool setNodeValue(data_type value);
    data_type getNodeValue(void) const;

  public:
    /*
      function findChildNode returns the child node indentified by the index_array
      index_array[0] is the child id for the child of this node
      index_array[1] is the child id for the child of the above determined child
      ...
      The search is performed over 'search_levels' levels
      'search_levels = 0' returns the current node, 'search_levels = '1' one of its children
    */  
    template <typename VECTOR_TYPE>
    basic_tree_node<data_type>* findChildNode(const VECTOR_TYPE& index_array,					      
					      unsigned int search_levels,
					      unsigned int offset = 0);


    /*
      The next function returns the right neighbour on the same leve of a given node
      If this right neighbour does not exist, we restart our search on the left side.
      In this case, the overflow flag is set.
      Note, that the function does not jump over zero-pointers. In other words,
      when a child pointer with value zero is found, the function stops
      (Hence the name DIRECT neighbour)
    */                                        
    basic_tree_node<data_type>* getDirectRightNeighbourNode(bool *overflow = NULL) const;

  private:
    bool allocate_child_array(void);

  public:
    const unsigned int nbr_children;
  private:
    basic_tree_node<data_type> *parent_node;
    unsigned int parent_child_id; //index of the parent array, 
    //where the current node is linked.
  
    basic_tree_node<data_type> **child_node;

  private:
    data_type node_value;

  };

  template <typename data_type>
  basic_tree_node<data_type>::basic_tree_node(
					      unsigned int nbr_children, 
					      const data_type *initValue
					      ):
    nbr_children(nbr_children),
    parent_node(NULL),  //no parent  
    parent_child_id(0), //no parent
    child_node(NULL)    //allocate memory only if required
  {
    //set node value
    if (initValue != NULL){
      node_value = *initValue;
    }
  
  }

  template <typename data_type>
  basic_tree_node<data_type>::~basic_tree_node(){
    //free memory for children array
    if (child_node != NULL)
      delete child_node;
  }

  template <typename data_type>
  bool basic_tree_node<data_type>::allocate_child_array(void){
    //allocate memory for children array
    child_node = new basic_tree_node<data_type>*[nbr_children];
    if (child_node == NULL){
      return false; //memory error
    }

    //init pointers
    for(unsigned int i=0; i < nbr_children; i++){
      child_node[i] = NULL;
    }
    return true;
  }

  template <typename data_type>
  basic_tree_node<data_type>* basic_tree_node<data_type>::getParentNode(void) const{
    return parent_node;
  }

  template <typename data_type>
  basic_tree_node<data_type>* basic_tree_node<data_type>::getChildNode(unsigned int child_id) const{
    if (child_id >= nbr_children){
      //child ID not valid
      return NULL;
    }else if(child_node == NULL){
      return NULL;
    }else{          
      return(child_node[child_id]);
    }
  }

  template <typename data_type>
  bool basic_tree_node<data_type>::setChildNode(unsigned int child_id,
						basic_tree_node<data_type> *child_node){
    if (child_id >= nbr_children){
      //illegal child id
      return false;
    }else{
      if (child_node->parent_node != NULL){
	//child already has parent
	return false;
      }

      if (this->child_node == NULL){
	//allocate memory for child array
	if (!allocate_child_array()){
	  //memory error
	  return false;
	}
      }
    
      //register child
      this->child_node[child_id] = child_node;
      //register parent
      child_node->parent_node = this;
      child_node->parent_child_id = child_id;
      return true;
    }
  }

  //remove child node
  template <typename data_type>
  bool basic_tree_node<data_type>::removeChildNode(unsigned int child_id){
    if (child_id >= nbr_children){
      //wrong child id
      return false;
    }else{

      //child has no parent anymore
      child_node[child_id]->parent_node = NULL;
      child_node[child_id]->parent_child_id = 0;

      //remove child from current node    
      child_node[child_id] = NULL;    

      return true;
    }
  }

  //remove and delete child node (and its children)
  template <typename data_type>
  bool basic_tree_node<data_type>::deleteChildNode(unsigned int child_id, bool recursive){
    bool return_value;

    if (child_id >= nbr_children){
      //wrong child id
      return false;
    }

    if(child_node == NULL){
      //no children
      return true;
    }

    if (child_node[child_id] == NULL){
      //already deleted
      return true;
    }

    if (recursive){
      return_value = child_node[child_id]->delete_children(recursive);
    }

    //delete child node
    delete child_node[child_id];
  
    //mark place as empty
    child_node[child_id] = NULL;

    return return_value;
  }

  template <typename data_type>
  bool basic_tree_node<data_type>::delete_children(bool recursive){
    if (child_node == NULL){
      //no children
      return true;
    }

    if (recursive){
      //delete children of all children
      for(unsigned int i = 0; i < nbr_children; i++){
	if (child_node[i] != NULL){
	  child_node[i]->delete_children(recursive);
	}
      }
    }

    //delete children themselves
    for(unsigned int i = 0; i < nbr_children; i++){
      if (child_node[i] != NULL){
	delete child_node[i];
	child_node[i] = NULL;
      }
    }

    return true;
  }

  template <typename data_type>
  bool basic_tree_node<data_type>::setNodeValue(data_type value){
    node_value = value;
    return true;
  }

  template <typename data_type>
  data_type basic_tree_node<data_type>::getNodeValue(void) const{
    return node_value;
  }

  
  template <typename data_type>
  template <typename VECTOR_TYPE>
  basic_tree_node<data_type>* 
  basic_tree_node<data_type>::findChildNode(const VECTOR_TYPE& index_array,
					    unsigned int search_levels,
					    unsigned int offset){


    basic_tree_node<data_type>* return_value = this;

    for(unsigned int i = offset; i < search_levels+offset; i++){
      if (return_value->nbr_children <= index_array[i]){
	//illegal index value
	return NULL;
      }

      if (return_value->child_node == NULL){
	//no children
	return (NULL);
      }
    
      //get child node
      return_value = return_value->child_node[index_array[i]];

      if (return_value == NULL){
	//child does not exist
	return NULL;
      }
    }
  
    return return_value;
  }

  template <typename data_type>
  basic_tree_node<data_type>* basic_tree_node<data_type>::getDirectRightNeighbourNode(
										      bool *overflow) const{
  
    bool overflow_internal; //only used, if overflow == NULL
    int level = 0;
  
    unsigned int child_id;
  
    basic_tree_node<data_type>* return_node;
  
  
    if (overflow == NULL){
      overflow = overflow_internal;
    }
  
    *overflow = false;
    return_node = this;
  
  
  
    /* move upstairs as long as necessary */  
    while(true){
      //get id of current node
      child_id = return_node->parent_child_id;
      //set to neighbour node
      child_id++;
    
      //get parent node
      return_node = return_node->getParentNode();  
      level++;
    
      //check, if neighbour exists
      if (child_id < return_node->nbr_children){
	//neighour should exist
	break;
      }else{
	if (return_node->parent_node != NULL){
	  //there still exists another parent
	  //so continue
	  continue;
	}else{
	  //we are at the top of the tree
	  child_id = 0;
	  *overflow = true;
	}
      }
    }
  
    /* move downstaires */
    while((level > 0) && (return_node != NULL)){
      return_node = return_node->child_node[child_id];
      level--;
      child_id = 0;
    }
  
  
    return return_node;
  }

};

#endif
