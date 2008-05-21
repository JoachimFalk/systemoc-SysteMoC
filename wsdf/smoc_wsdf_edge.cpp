//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Math/smoc_math.hpp>

#include <systemoc/smoc_wsdf_edge.hpp>

using CoSupport::Streams::Indent;
using CoSupport::Streams::dout;
using namespace std;

#define FAST_CALC_MODE

//0: No output
///100: debug
///101: src_has_iteration_level
#ifndef VERBOSE_LEVEL_SMOC_WSDF_EDGE
#define VERBOSE_LEVEL_SMOC_WSDF_EDGE 100
#endif


/* *****************************************************************************
 *                           smoc_wsdf_iter_max                                *
 ******************************************************************************* */



smoc_wsdf_iter_max::smoc_wsdf_iter_max(const smoc_wsdf_iter_max& a)
  : next_level_iter_max(a.next_level_iter_max == NULL ? 
                        NULL :
                        &(a.next_level_iter_max->duplicate())),
    previous_level_iter_max(a.previous_level_iter_max == NULL ?
                            NULL :
                            &(a.previous_level_iter_max->duplicate()))
{}

void smoc_wsdf_iter_max::print_node(std::ostream& os) const{
  if (next_level_iter_max != NULL){
    os << "; ";
    next_level_iter_max->print_node(os);
  }
}

void smoc_wsdf_iter_max::delete_next_level_tree() {
  if (next_level_iter_max != NULL)
    delete next_level_iter_max;
}

smoc_wsdf_iter_max* 
smoc_wsdf_iter_max::set_next_level(smoc_wsdf_iter_max* next_level_iter_max){
  this->next_level_iter_max = next_level_iter_max;

  if (next_level_iter_max != NULL)
    next_level_iter_max->previous_level_iter_max = this;

  return next_level_iter_max;
}
 
smoc_wsdf_iter_max*  
smoc_wsdf_iter_max::set_previous_level(smoc_wsdf_iter_max* previous_level_iter_max){
  this->previous_level_iter_max = previous_level_iter_max;

  if (previous_level_iter_max != NULL)
    previous_level_iter_max->next_level_iter_max = this;

  return previous_level_iter_max;
}



/* *****************************************************************************
 *                           smoc_wsdf_iter_value                              *
 ******************************************************************************* */

smoc_wsdf_iter_max_value::udata_type 
smoc_wsdf_iter_max_value::get_iter_max(const bvector_type& parent_iter_max) const{
  return iter_max;
}

void smoc_wsdf_iter_max_value::print_node(std::ostream& os) const{
  os << iter_max;
  parent_type::print_node(os);
}


/* *****************************************************************************
 *                           smoc_wsdf_iter_cond                               *
 ******************************************************************************* */

smoc_wsdf_iter_max_cond::smoc_wsdf_iter_max_cond(const smoc_wsdf_iter_max_cond& a)
  : smoc_wsdf_iter_max(a),
    parent_iter_level(a.parent_iter_level),
    parent_max(a.parent_max == NULL ? NULL : &(a.parent_max->duplicate())),
    parent_not_max(a.parent_not_max == NULL ? NULL : &(a.parent_not_max->duplicate()))
{}

smoc_wsdf_iter_max_cond::udata_type 
smoc_wsdf_iter_max_cond::get_iter_max(const bvector_type& parent_iter_max) const{
  if (parent_iter_max[parent_iter_level]){
    assert(parent_max != NULL);
    return parent_max->get_iter_max(parent_iter_max);
  }else{
    assert(parent_not_max != NULL);
    return parent_not_max->get_iter_max(parent_iter_max);
  }
}

void smoc_wsdf_iter_max_cond::clean_children(){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_iter_max_cond::clean_children" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  assert(parent_max != NULL);

  smoc_wsdf_iter_max* relevant_subtree = 
    parent_max->get_relevant_subtree();

  if (relevant_subtree != parent_max){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "Replace subtree" << std::endl;
    CoSupport::Streams::dout << "parent_max : " << *parent_max << std::endl;
    CoSupport::Streams::dout << "relevant_subtree : " << *relevant_subtree << std::endl;
#endif
    //copy new relevant subtree
    relevant_subtree = &(relevant_subtree->duplicate());

    //delete previous subtree
    delete parent_max;
    parent_max = relevant_subtree;
  }else{
    //try next level
    parent_max->clean_children();
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_iter_max_cond::clean_children" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif
}

void smoc_wsdf_iter_max_cond::delete_children() {
  if(parent_max != NULL)
    delete parent_max;
  parent_max = NULL;

  if(parent_not_max != NULL)
    delete parent_not_max;
  parent_not_max = NULL;
}

smoc_wsdf_iter_max_cond* 
smoc_wsdf_iter_max_cond::insert_cond_node(smoc_wsdf_iter_max_cond* new_cond_node){

  //Move current value node to new condition node
  new_cond_node->parent_not_max = 
    dynamic_cast<smoc_wsdf_iter_max_value*>(parent_max);
  assert(new_cond_node->parent_not_max != NULL);

  parent_max = new_cond_node;
  return new_cond_node;
  
}


smoc_wsdf_iter_max*
smoc_wsdf_iter_max_cond::get_relevant_subtree() {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_iter_max_cond::get_relevant_subtree" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  smoc_wsdf_iter_max* return_value = NULL;

  assert(parent_max != NULL);

  //Try to get relevant value node
  smoc_wsdf_iter_max_value* temp_node = 
    dynamic_cast<smoc_wsdf_iter_max_value*>(parent_max->get_relevant_subtree());

  if (temp_node != NULL){
    if (temp_node->get_iter_max() == parent_not_max->get_iter_max()){
      //condition node is not necessary
      return_value = parent_not_max;      
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "Condition node not required" 
                      << std::endl; 
#endif
    }else{
      //condition node is necessary
      return_value = this;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "Children are different. Cannot eliminate node." 
                      << std::endl; 
#endif
    }
  }else{
    //condition node is necessary
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "Child is a condition node. Cannot eliminate" 
                    << std::endl; 
#endif
    return_value = this;
  }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_iter_max_cond::get_relevant_subtree" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_value;
}


void smoc_wsdf_iter_max_cond::print_node(std::ostream& os) const{

  assert(parent_max != NULL);
  assert(parent_not_max != NULL);

  parent_not_max->print_node(os);
  os << ", ";
  os << parent_iter_level << " -> ";
  parent_max->print_node(os);

  parent_type::print_node(os);
  
  
}



/* *****************************************************************************
 *                    smoc_wsdf_edge                                           *
 ******************************************************************************* */


smoc_wsdf_edge_descr::smoc_wsdf_edge_descr(unsigned token_dimensions,
                                           const u2vector_type& src_firing_blocks,
                                           const u2vector_type& snk_firing_blocks,
                                           const uvector_type& u0,
                                           const uvector_type& c,
                                           const uvector_type& delta_c,
                                           const uvector_type& d,
                                           const svector_type& bs,
                                           const svector_type& bt)
  : token_dimensions(token_dimensions),
    snk_firing_block_dimensions(snk_firing_blocks[0].size()),
    src_firing_blocks(src_firing_blocks),
    src_num_firing_levels(src_firing_blocks.size()),
    src_num_eff_token_firing_levels(1),
    snk_firing_blocks(snk_firing_blocks),
    snk_num_firing_levels(snk_firing_blocks.size()),
    snk_window_firing_blocks(c),
    p(this->src_firing_blocks[0]),
    v(u0),u0(u0),
    c(c),
    delta_c(delta_c),
    d(d),
    bs(bs),
    bt(bt)                  
{
  set_change_indicator();
  check_parameters();
  insert_snk_vtu_firing_block();
}

smoc_wsdf_edge_descr::smoc_wsdf_edge_descr(unsigned token_dimensions,
                                           const u2vector_type& src_firing_blocks,
                                           const uvector_type& snk_firing_block,
                                           const uvector_type& u0,
                                           const uvector_type& c,
                                           const uvector_type& delta_c,
                                           const uvector_type& d,
                                           const svector_type& bs,
                                           const svector_type& bt)
  : token_dimensions(token_dimensions),
    snk_firing_block_dimensions(snk_firing_block.size()),
    src_firing_blocks(src_firing_blocks),
    src_num_firing_levels(src_firing_blocks.size()),
    src_num_eff_token_firing_levels(1),
    snk_firing_blocks(u2vector_type(snk_firing_block)),
    snk_num_firing_levels(snk_firing_blocks.size()),
    snk_window_firing_blocks(c),
    p(this->src_firing_blocks[0]),
    v(u0),u0(u0),
    c(c),
    delta_c(delta_c),
    d(d),
    bs(bs),
    bt(bt)                  
{
  set_change_indicator();
  check_parameters();
  insert_snk_vtu_firing_block();
}


smoc_wsdf_edge_descr::smoc_wsdf_edge_descr(const smoc_wsdf_edge_descr& e1,
                                           const uvector_type& ext_reusage,
                                           bool optimize_borders)
  : token_dimensions(e1.token_dimensions),
    snk_firing_block_dimensions(e1.snk_firing_block_dimensions),
    src_firing_blocks(e1.src_firing_blocks),
    src_num_firing_levels(src_firing_blocks.size()),
    src_num_eff_token_firing_levels(e1.src_num_eff_token_firing_levels),
    snk_firing_blocks(e1.calc_snk_firing_block(e1.calc_c(ext_reusage),
                                               optimize_borders)),
    snk_num_firing_levels(e1.snk_num_firing_levels),
    snk_window_firing_blocks(e1.calc_c(ext_reusage)),
    p(this->src_firing_blocks[0]),
    v(e1.u0),u0(e1.u0),
    c(e1.calc_c(ext_reusage)),
    delta_c(e1.delta_c),
    d(e1.d),
    bs(e1.calc_bs(c,optimize_borders)),
    bt(e1.calc_bt(c,optimize_borders))                  
{
  set_change_indicator();
  check_parameters();
}

smoc_wsdf_edge_descr::~smoc_wsdf_edge_descr(){}




smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_src_firing_block(udata_type block_size,
                                               unsigned token_dimension) const {

  return get_scm_firing_block(src_firing_blocks,
                              block_size,
                              token_dimension);
  
}


smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_snk_firing_block(udata_type block_size,
                                               unsigned token_dimension) const {

  if (block_size < delta_c(token_dimension)){
    // We have to split sliding window
    if (block_size <= 1)
      //always possible
      return 1;
    else if (c[token_dimension] <= block_size){
      //not possible to insert iteration level
      return 0;
    }

    return get_scm_firing_block(snk_window_firing_blocks,
                                block_size,
                                token_dimension);
    
  }else{
    
    if (block_size % delta_c(token_dimension)){
      //block size must be a multiple of delta_c
      return 0;
    }
    block_size /= delta_c(token_dimension);


    if (block_size <= 1)
      //block size does not require an own iterator level
      return true;

    
    return get_scm_firing_block(snk_firing_blocks,
                                block_size,
                                token_dimension);
    
  }
  
}


void 
smoc_wsdf_edge_descr::insert_src_firing_level(udata_type block_size,
                                              unsigned token_dimension){
  set_change_indicator();
  
  //start at smallest firing level
  unsigned firing_level = 0;

  if (block_size <= 1)
    //block size does not require an own iterator level
    return;

  while((firing_level < src_num_firing_levels) &&
        (src_firing_blocks[firing_level][token_dimension] < block_size)){
    firing_level++;
  }

  if (firing_level < src_num_firing_levels){
    if (src_firing_blocks[firing_level][token_dimension] == block_size){
      //block size already exists
      return;
    }
  }

  //create new firing block
  uvector_type new_block(token_dimensions);
  for(unsigned int i = 0; i < token_dimension; i++){
    if (firing_level == src_num_firing_levels)
      new_block[i] = src_firing_blocks[firing_level-1][i];
    else
      new_block[i] = src_firing_blocks[firing_level][i];
  }
  new_block[token_dimension] = block_size;
  for(unsigned int i = token_dimension+1; i < token_dimensions; i++){    
    if (firing_level == 0)
      new_block[i] = 1;
    else
      new_block[i] = src_firing_blocks[firing_level-1][i];
  }

  //insert firing block
  src_firing_blocks.insert_item(firing_level, new_block);
  if (firing_level < src_num_eff_token_firing_levels){
    //we have split effetive token
    src_num_eff_token_firing_levels++;
  }
  src_num_firing_levels++;
  
}


bool 
smoc_wsdf_edge_descr::insert_snk_firing_level(udata_type block_size,
                                              unsigned token_dimension){

  
  set_change_indicator();

  //start with smallest firing block
  unsigned firing_level = 0;

  //calculate effective block_size
  if (block_size < delta_c(token_dimension)){
    // we have to split sliding window
    if (block_size <= 1)
      //we do not require an own iterator level
      return true;
    else if (c[token_dimension] <= block_size){
      //not possible to insert iteration level
      return false;
    }
    
    while(snk_window_firing_blocks[firing_level][token_dimension] < 
          block_size){
      //Note, that loop terminates due to 
      //c[token_dimension] > block_size
      firing_level++;
    }

    //Check, if block already exists
    if (snk_window_firing_blocks[firing_level][token_dimension] == block_size){
      //block size already exists
      return true;
    }


    //create new firing block
    uvector_type new_block(snk_firing_block_dimensions);
    for(unsigned int i = 0; i < snk_firing_block_dimensions; i++){      
      if (firing_level == snk_window_firing_blocks.size())
        new_block[i] = snk_window_firing_blocks[firing_level-1][i];
      else
        new_block[i] = snk_window_firing_blocks[firing_level][i];
    }
    new_block[token_dimension] = block_size;
    for(unsigned int i = token_dimension+1; i < snk_firing_block_dimensions; i++){
      if (firing_level == 0)
        new_block[i] = 1;
      else
        new_block[i] = snk_window_firing_blocks[firing_level-1][i];
    }
    
    //insert new firing block
    snk_window_firing_blocks.insert_item(firing_level, new_block);

    return true;
    
  }else{
    assert(block_size % delta_c(token_dimension) == 0);
    block_size /= delta_c(token_dimension);


    if (block_size <= 1)
      //block size does not require an own iterator level
      return true;

    while((firing_level < snk_num_firing_levels) &&
          (snk_firing_blocks[firing_level][token_dimension] < block_size)){
      firing_level++;
    }

    //Check, if block already exists.
    if (firing_level < snk_num_firing_levels){
      if (snk_firing_blocks[firing_level][token_dimension] == block_size){
        //block size already exists
        return true;
      }
    }

    //create new firing block
    uvector_type new_block(snk_firing_block_dimensions);
    for(unsigned int i = 0; i < token_dimension; i++){
      if (firing_level == snk_num_firing_levels)
        new_block[i] = snk_firing_blocks[firing_level-1][i];
      else
        new_block[i] = snk_firing_blocks[firing_level][i];
    }
    new_block[token_dimension] = block_size;
    for(unsigned int i = token_dimension+1; i < snk_firing_block_dimensions; i++){
      if (firing_level == 0)
        new_block[i] = 1;
      else
        new_block[i] = snk_firing_blocks[firing_level-1][i];
    }


    //Insert firing block
    snk_firing_blocks.insert_item(firing_level, new_block);
    snk_num_firing_levels++;

    return true;

  }
  
}


void 
smoc_wsdf_edge_descr::firing_levels_snk2src() {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::firing_levels_snk2src()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif
  
  /*
    Process sink window propagation
  */
  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    
    // Determine, whether sink block can
    // be represented by source.
    // If yes, determine the resulting block
    // size.
    udata_type res_block_size = 
      get_scm_src_firing_block(delta_c[token_dimension],
                               token_dimension);
    res_block_size *= 
      delta_c[token_dimension];
    
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "res_block_size = " << res_block_size << std::endl;
#endif
    
    if (get_scm_snk_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "Insert firing level" << std::endl;
#endif
      insert_src_firing_level(res_block_size,
                              token_dimension);
      insert_snk_firing_level(res_block_size,
                              token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "src_firing_blocks = " 
                      << src_firing_blocks << std::endl;
      CoSupport::Streams::dout << "snk_firing_blocks = " 
                      << snk_firing_blocks << std::endl;
#endif
      
    }else{
      //Either resulting block size cannot be integrated at all
      //or it belongs to next source firing block
    }    
  }
  

  /*
    Process sink firing blocks
   */
  for(unsigned int firing_level = 0; 
      firing_level < snk_num_firing_levels; 
      firing_level++){
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "firing_level = " << firing_level 
                      << ", token_dimension = " << token_dimension 
                      << std::endl;
      CoSupport::Streams::dout << Indent::Up;
      CoSupport::Streams::dout << "snk_firing_blocks[firing_level][token_dimension] = "
                      << snk_firing_blocks[firing_level][token_dimension]
                      << std::endl;
#endif
      
      // Determine, whether sink block can
      // be represented by source.
      // If yes, determine the resulting block
      // size.
      udata_type res_block_size = 
        get_scm_src_firing_block(snk_firing_blocks[firing_level][token_dimension]*
                                 delta_c[token_dimension],
                                 token_dimension);
      res_block_size *= 
        snk_firing_blocks[firing_level][token_dimension]*
        delta_c[token_dimension];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "res_block_size = " << res_block_size << std::endl;
#endif

      if (get_scm_snk_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "Insert firing level" << std::endl;
#endif
        insert_src_firing_level(res_block_size,
                                token_dimension);
        insert_snk_firing_level(res_block_size,
                                token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "src_firing_blocks = " 
                        << src_firing_blocks << std::endl;
        CoSupport::Streams::dout << "snk_firing_blocks = " 
                        << snk_firing_blocks << std::endl;
#endif

      }else{
        //Either resulting block size cannot be integrated at all
        //or it belongs to next source firing block
      }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << Indent::Down;
#endif
      
    }
  }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::firing_levels_snk2src()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif
  
}
  

void
smoc_wsdf_edge_descr::firing_levels_src2snk() {
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::firing_levels_src2snk()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif
  for(unsigned int firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "firing_level = " << firing_level 
                      << ", token_dimension = " << token_dimension 
                      << std::endl;
      CoSupport::Streams::dout << Indent::Up;
      CoSupport::Streams::dout << "src_firing_blocks[firing_level][token_dimension] = "
                      << src_firing_blocks[firing_level][token_dimension]
                      << std::endl;
#endif
      
      // Determine, whether source block can
      // be represented by sink.
      // If yes, determine the resulting block
      // size.
      udata_type res_block_size = 
        get_scm_snk_firing_block(src_firing_blocks[firing_level][token_dimension],
                                 token_dimension);
      res_block_size *= src_firing_blocks[firing_level][token_dimension];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "res_block_size = " << res_block_size << std::endl;
#endif

      if (get_scm_src_firing_block(res_block_size,token_dimension) == 1){       
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "Insert firing level" << std::endl;
#endif
        insert_src_firing_level(res_block_size,
                                token_dimension);
        insert_snk_firing_level(res_block_size,
                                token_dimension);
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "src_firing_blocks = " 
                        << src_firing_blocks << std::endl;
        CoSupport::Streams::dout << "snk_firing_blocks = " 
                        << snk_firing_blocks << std::endl;
#endif

      }else{
        //Either resulting block size cannot be integrated at all
        //or it belongs to next source firing block
      }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << Indent::Down;
#endif
      
    }
  }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::firing_levels_src2snk()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif
}


smoc_wsdf_edge_descr::udata_type 
smoc_wsdf_edge_descr::get_scm_firing_block(u2vector_type firing_blocks,
                                           udata_type block_size,
                                           unsigned token_dimension) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  //default value, if no block_size does does not match
  //any firing block
  udata_type return_value = 0;

  if (block_size <= 1){
    //always possible
    return_value = 1;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "return_value = " << return_value << std::endl;
    CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
    CoSupport::Streams::dout << Indent::Down;
#endif
    return return_value;
  }

  //look for block size which is dividable by block_size
  unsigned int firing_level = 0;
  udata_type prev_block_size = 1;
  while(firing_level < firing_blocks.size()){
    if (firing_blocks[firing_level][token_dimension] % block_size == 0){
      //found
      return_value = CoSupport::scm(prev_block_size, block_size);
      return_value /= block_size;
      break;
    }
    
    //Memorize previous block size
    firing_level++;
    if (firing_level < firing_blocks.size()){
      if (firing_blocks[firing_level][token_dimension] != 
          firing_blocks[firing_level-1][token_dimension]){
        prev_block_size = 
          firing_blocks[firing_level-1][token_dimension];
      }
    }
  }  


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "return_value = " << return_value << std::endl;
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::get_scm_firing_block()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_value;
}


const smoc_wsdf_edge_descr::uvector_type& 
smoc_wsdf_edge_descr::snk_iteration_max() const {

  if (cache_snk_iter_max_valid)
    return snk_iteration_max_cached;

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();
        
  uvector_type 
    iteration_max(calc_snk_iteration_max(snk_iteration_level_table));

  append_snk_window_iteration(iteration_max);

  snk_iteration_max_cached = iteration_max;
  cache_snk_iter_max_valid = true;

  return snk_iteration_max_cached;
}


const smoc_wsdf_edge_descr::uvector_type& 
smoc_wsdf_edge_descr::src_iteration_max() const {


  // return cached value, if valid.
  if (cache_src_iter_max_valid)
    return src_iteration_max_cached;

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter  smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  
  uvector_type return_vector(calc_src_iteration_levels());
  unsigned iter_level = calc_src_iteration_levels()-1;

  uvector_type current_firing_block_size(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    current_firing_block_size[i] = 1;

  for(unsigned firing_level = 0;
      firing_level < src_num_firing_levels;
      firing_level++){
    for(unsigned token_dimension = 0;
	token_dimension < token_dimensions;
	token_dimension++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "firing_level = " << firing_level;
      CoSupport::Streams::dout << " token_dimension = " << token_dimension << std::endl;
#endif
      if (src_has_iteration_level(firing_level, token_dimension)){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "Is associated to iteration level " 
                        << iter_level << std::endl;
#endif
	assert(src_firing_blocks[firing_level][token_dimension] %
	       current_firing_block_size[token_dimension] == 0);
	return_vector[iter_level] = 
	  src_firing_blocks[firing_level][token_dimension] /
	  current_firing_block_size[token_dimension] - 1;
                                
	iter_level--;
	current_firing_block_size[token_dimension] = 
	  src_firing_blocks[firing_level][token_dimension];
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Source iteration max: " 
                  << return_vector << std::endl;
#endif


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::src_iteration_max()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  cache_src_iter_max_valid = true;
  src_iteration_max_cached = return_vector;

  return src_iteration_max_cached;
}


smoc_wsdf_iter_max& 
smoc_wsdf_edge_descr::ext_src_iteration_max(unsigned int firing_level,
                                            unsigned int token_dimension
                                            ) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::ext_src_iteration_max(..,..)" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
  CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
  CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
  CoSupport::Streams::dout << "src_firing_blocks[firing_level][token_dimension] = " 
                  << src_firing_blocks[firing_level][token_dimension]
                  << std::endl;
#endif

  smoc_wsdf_iter_max* return_node = NULL;

  smoc_wsdf_iter_max_cond* current_condition_node = NULL;

  udata_type iter_max;

  //start with default case
  if (firing_level == 0){
    iter_max = 
      src_firing_blocks[firing_level][token_dimension]-1;
  }else{
    //divide and round up
    iter_max = 
      (src_firing_blocks[firing_level][token_dimension]+
       src_firing_blocks[firing_level-1][token_dimension]-1)/
      src_firing_blocks[firing_level-1][token_dimension];
    iter_max--;      
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Default iteration max: " << iter_max << std::endl;
#endif

  //create node
  return_node = 
    new smoc_wsdf_iter_max_value(iter_max);  
      
  //Check whether we have incomplete firing blocks
  for(unsigned parent_level = firing_level+1;
      parent_level < src_num_firing_levels;
      parent_level++){
    if (src_has_iteration_level(parent_level, 
                                token_dimension)){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "parent_level = " << parent_level << std::endl;
  CoSupport::Streams::dout << Indent::Up;
  CoSupport::Streams::dout << "src_firing_blocks[parent_level][token_dimension] = "
                  << src_firing_blocks[parent_level][token_dimension]
                  << std::endl;
#endif

      //Calculate effective firing block size      
      udata_type current_firing_block_size = 
        src_firing_blocks[parent_level][token_dimension];

      for(int j = parent_level-1;
          j >= (int)firing_level;
          j--){
        current_firing_block_size =
          current_firing_block_size % 
          src_firing_blocks[j][token_dimension];
      }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "current_firing_block_size = " 
                      << current_firing_block_size
                      << std::endl;
#endif

      if (current_firing_block_size != 0){
        //We require condition
        
        const sdata_type cond_iter_level =
          calc_src_iteration_level_table(true)[parent_level][token_dimension];
        assert(cond_iter_level >= 0);

        if (current_condition_node == NULL){
          //we do not already have started a condition node
          current_condition_node = 
            new smoc_wsdf_iter_max_cond(cond_iter_level,
                                        dynamic_cast<smoc_wsdf_iter_max_value*>(return_node));
          return_node = current_condition_node;
        }else{
          //we have already started a condition node.
          //Insert new condition.
          current_condition_node = 
            current_condition_node->insert_cond_node(new smoc_wsdf_iter_max_cond(cond_iter_level));
        }

        //Now append new iteration max
        if (firing_level == 0){
          iter_max = current_firing_block_size;
        }else {
          iter_max = 
            (current_firing_block_size + 
             src_firing_blocks[firing_level-1][token_dimension]-1) /
            src_firing_blocks[firing_level-1][token_dimension];
        }
        iter_max--;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        CoSupport::Streams::dout << "current_firing_block_size = "
                        << current_firing_block_size
                        << std::endl;
        if (firing_level != 0)
          CoSupport::Streams::dout << "src_firing_blocks[firing_level-1][token_dimension] = " 
                          << src_firing_blocks[firing_level-1][token_dimension]
                          << std::endl;
        CoSupport::Streams::dout << "iter_max = " 
                        << iter_max
                        << std::endl;
#endif

        current_condition_node->set_parent_max(new smoc_wsdf_iter_max_value(iter_max));

      }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << Indent::Down;
#endif
    }
  }

  //Simplify result
  return_node->clean_children();

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::ext_src_iteration_max(..,..)" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return *return_node;
}



smoc_wsdf_iter_max& 
smoc_wsdf_edge_descr::ext_src_iteration_max() const {

  smoc_wsdf_iter_max *return_value = NULL;
  smoc_wsdf_iter_max *current_node = NULL;

  for(int firing_level = src_num_firing_levels-1;
      firing_level >= 0;
      firing_level--){
    for(int token_dimension = token_dimensions-1;
        token_dimension >= 0;
        token_dimension--){
      if (src_has_iteration_level(firing_level,token_dimension)){
        if (current_node == NULL){
          return_value = 
            &(ext_src_iteration_max(firing_level,
                                    token_dimension));
          current_node = return_value;
        }else{
          current_node = 
            current_node->set_next_level(&(ext_src_iteration_max(firing_level,
                                                                 token_dimension)));
        }
      }
    }
  }

  assert(return_value != NULL);
  return *return_value;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::snk_num_iterations() const {
  uvector_type return_vector(snk_iteration_max());
  
  for(unsigned int i = 0;
      i < return_vector.size();
      i++){
    return_vector[i]++;
  }

  return return_vector;
}
 
smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::src_num_iterations() const {
  uvector_type return_vector(src_iteration_max());
  
  for(unsigned int i = 0;
      i < return_vector.size();
      i++){
    return_vector[i]++;
  }

  return return_vector;
}

smoc_wsdf_edge_descr::svector_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_vector() const {
  return -(bs);
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::snk_data_element_mapping_matrix() const {

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();

  uvector_type 
    iteration_max(calc_snk_iteration_max(snk_iteration_level_table));

  append_snk_window_iteration(iteration_max);
  umatrix_type 
    return_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
                                                       iteration_max));

#ifdef FAST_CALC_MODE  
  matrix_thin_out(return_matrix, iteration_max);
#endif  
        

  return return_matrix;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::src_data_element_mapping_vector() const {
  return d;
}

smoc_wsdf_edge_descr::smatrix_type 
smoc_wsdf_edge_descr::calc_border_condition_matrix() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix()" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  s2vector_type snk_iteration_level_table = 
    calc_snk_iteration_level_table();
        
  uvector_type 
    iteration_max(calc_snk_iteration_max(snk_iteration_level_table));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Iteration-Max (without window iteration): " 
                           << iteration_max << std::endl;
#endif

  append_snk_window_iteration(iteration_max);
  umatrix_type 
    mapping_matrix(calc_snk_data_element_mapping_matrix(snk_iteration_level_table,
                                                        iteration_max));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Mapping-matrix: " << mapping_matrix << std::endl;
#endif

  smatrix_type 
    return_matrix(mapping_matrix.size1(),mapping_matrix.size2());

  uvector_type 
    vtu_iteration_levels(get_vtu_iteration_level(snk_iteration_level_table));

  calc_border_condition_matrix(mapping_matrix,
                               vtu_iteration_levels,
			       return_matrix);

#ifdef FAST_CALC_MODE  
  matrix_thin_out(return_matrix, iteration_max);
#endif  

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif


  return return_matrix;
}


smoc_wsdf_edge_descr::svector_type
smoc_wsdf_edge_descr::calc_low_border_condition_vector() const {
  return bs;
}

smoc_wsdf_edge_descr::svector_type
smoc_wsdf_edge_descr::calc_high_border_condition_vector() const {
  svector_type return_vector(u0.size());

  for(unsigned int i = 0; i < u0.size(); i++){
    return_vector[i] = u0[i] + bs[i] - 1;
  }

  return return_vector;
}

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::max_data_element_id() const{
  uvector_type return_vector(src_firing_blocks[src_firing_blocks.size()-1]);
        
  for(unsigned int i = 0; i < return_vector.size(); i++){
    return_vector[i] --;
  }

  return return_vector;

}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::src_data_element_mapping_matrix() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::src_data_element_mapping_matrix()" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  uvector_type prev_mapping_factor(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    prev_mapping_factor[i] = 1;

  const unsigned matrix_rows = token_dimensions;
  const unsigned matrix_cols = calc_src_iteration_levels();       
  umatrix_type return_matrix(matrix_rows, matrix_cols);

  signed iter_level = calc_src_iteration_levels()-1;

  for(unsigned firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0;
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level, token_dimension)){
	//default assignment
	for(unsigned row = 0; row < matrix_rows; row++){
	  return_matrix(row, iter_level) = 0;
	}

	return_matrix(token_dimension,iter_level) =
	  prev_mapping_factor[token_dimension];
                                
	prev_mapping_factor[token_dimension] = 
	  src_firing_blocks[firing_level][token_dimension];

	iter_level--;
      }
    }
  }

  //error checking
  assert(iter_level == -1); //otherwise not all iteration levels covered

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Mapping matrix: " << return_matrix << std::endl;
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::src_data_element_mapping_matrix()" 
                  << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_matrix;
}


smoc_wsdf_edge_descr::uvector_type
smoc_wsdf_edge_descr::calc_snk_r_vtu() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  uvector_type return_vector(u0+bs+bt-c);

  for(unsigned int i = 0; i < token_dimensions; i++){
    assert(return_vector[i] % delta_c[i] == 0); //Invalid parameters

    return_vector[i] /= delta_c[i];
    return_vector[i]++;
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "snk_r_vtu = " << return_vector;
  CoSupport::Streams::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_snk_r_vtu()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_vector;
}

smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_src_r_vtu() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_src_r_vtu()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  uvector_type return_vector(token_dimensions);
        
  for(unsigned int i = 0; i < token_dimensions; i++){
    //currently, we require, that an effective token
    //exactly belongs to one virtual token union
    assert(u0[i] % p[i] == 0);

    return_vector[i] = u0[i] / p[i];
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "src_r_vtu = " << return_vector;
  CoSupport::Streams::dout << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_src_r_vtu()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_vector;
}



void smoc_wsdf_edge_descr::check_local_balance() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::check_local_balance" << std::endl;
  CoSupport::Streams::dout << Indent::Up;

  CoSupport::Streams::dout << "snk_firing_blocks = " << snk_firing_blocks;
  CoSupport::Streams::dout << std::endl;
  CoSupport::Streams::dout << "c = " << c;
  CoSupport::Streams::dout << std::endl;

  CoSupport::Streams::dout << "src_firing_blocks = " << src_firing_blocks;
  CoSupport::Streams::dout << std::endl;
#endif

  //Calculate number of invocations per virtual token union
  const uvector_type snk_r_vtu(calc_snk_r_vtu());
  const uvector_type src_r_vtu(calc_src_r_vtu());

  //calculate the number of virtual token unions in each dimension
  udata_type snk_num_vtu;
  udata_type src_num_vtu;

  for(unsigned int i = 0; i < token_dimensions; i++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "Token dimension " << i << std::endl;
    CoSupport::Streams::dout << Indent::Up;
#endif
    //Check for incomplete virtual token union
    if (snk_firing_blocks[snk_num_firing_levels-1][i] % snk_r_vtu[i] != 0){
      std::cout << "Violation of balance equation in dimension " << i << std::endl;
      std::cout << "snk_firing_blocks[snk_num_firing_levels-1][i] = " 
		<< snk_firing_blocks[snk_num_firing_levels-1][i]
		<< std::endl;
      std::cout << "snk_r_vtu[i] = " 
		<< snk_r_vtu[i]
		<< std::endl;
      print_edge_parameters(std::cout);
      assert(false);
    }

    snk_num_vtu = 
      snk_firing_blocks[snk_num_firing_levels-1][i] / snk_r_vtu[i];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "snk_num_vtu =  " << snk_num_vtu  << std::endl;
#endif

    //Check for incomplete virtual token unions
    assert((src_firing_blocks[src_num_firing_levels-1][i] / p[i]) % src_r_vtu[i] == 0);

    src_num_vtu = 
      src_firing_blocks[src_num_firing_levels-1][i] / p[i] / src_r_vtu[i];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "src_num_vtu =  " << src_num_vtu  << std::endl;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << Indent::Down;
#endif

                
    //Check, if edge balanced
    assert(snk_num_vtu == src_num_vtu);
  }       

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::check_local_balance" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif
}


void smoc_wsdf_edge_descr::check_parameters() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::check_parameters" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "token_dimensions = " << token_dimensions << std::endl;
  CoSupport::Streams::dout << "v.size() = " << v.size() << std::endl;
  CoSupport::Streams::dout << "snk_firing_block_dimensions = " << snk_firing_block_dimensions << std::endl;
#endif

  /* Check number of dimensions */
  assert((snk_firing_block_dimensions == token_dimensions) ||
         (snk_firing_block_dimensions == (token_dimensions+1)));
  assert(v.size() == token_dimensions);
  assert(u0.size() == token_dimensions);
  assert(c.size() == token_dimensions);
  assert(delta_c.size() == token_dimensions);
  assert(d.size() == token_dimensions);
  assert(bs.size() == token_dimensions);
  assert(bt.size() == token_dimensions);  

  check_local_balance();

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::check_parameters" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif
}


void smoc_wsdf_edge_descr::check_fireblocks(bool incomplete_blocks) const {
  if (!incomplete_blocks){
    /* Check, that we only have complete firing blocks */
    //source
    for(unsigned int i = 0; i < token_dimensions; i++){             
      for(unsigned int j = 0; j < src_num_firing_levels-1; j++){
        assert(src_firing_blocks[j+1][i] % src_firing_blocks[j][i] == 0);
      }
    }
    
    //sink
    for(unsigned int i = 0; i < snk_firing_block_dimensions; i++){             
      for(unsigned int j = 0; j < snk_num_firing_levels-1; j++){
        assert(snk_firing_blocks[j+1][i] % snk_firing_blocks[j][i] == 0);
      }               
    }
  }
}

bool smoc_wsdf_edge_descr::snk_has_iteration_level(unsigned firing_level,
						   unsigned token_dimension,
                                                   u2vector_type snk_firing_blocks) const {

  if (firing_level == 0){
    if (snk_firing_blocks[firing_level][token_dimension] == 1){
      //ingore firing blocks of size one except if all firing blocks
      //have the size one
      if (snk_firing_blocks[snk_firing_blocks.size()-1][token_dimension] == 1){
        return true;
      }else{
        return false;
      }
    }else{
      return true;
    }
  }else if (snk_firing_blocks[firing_level][token_dimension] == 
	    snk_firing_blocks[firing_level-1][token_dimension]){
    return false;
  }else if (snk_firing_blocks[firing_level][token_dimension] == 1){
    return false;
  }else{
    return true;
  }
}

bool smoc_wsdf_edge_descr::src_has_iteration_level(unsigned firing_level,
						   unsigned token_dimension) const {
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::src_has_iteration_level" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
  CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
  CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
  CoSupport::Streams::dout << "src_num_firing_levels = " << src_num_firing_levels << std::endl;
#endif

  bool return_value = false;

  if (firing_level <= 0){
    //Always include firing_level 0 (effective token)
    return_value = true;
  }else if ((firing_level == 1) && 
            (src_firing_blocks[firing_level][token_dimension] ==
             src_firing_blocks[src_num_firing_levels-1][token_dimension])){
    //All firing levels have the same size.
    //Return at least one not belong to the effective token.
    return_value =  true;
  }else if (src_firing_blocks[firing_level][token_dimension] == 1){
    //firing blocks of size 1 need not to be considered
    return_value = false;
  }else if (src_firing_blocks[firing_level][token_dimension] == 
	    src_firing_blocks[firing_level-1][token_dimension]){
    return_value = false;
  }else{
    return_value =  true;
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::src_has_iteration_level" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_value;


}


smoc_wsdf_edge_descr::s2vector_type 
smoc_wsdf_edge_descr::calc_src_iteration_level_table(bool include_eff_token) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
  CoSupport::Streams::dout << "Enter  smoc_wsdf_edge_descr::calc_src_iteration_level_table()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  unsigned iteration_level = 0;

  s2vector_type return_table(include_eff_token ? 
                             src_firing_blocks.size() :
                             src_firing_blocks.size() - 
                             src_num_eff_token_firing_levels,
                             svector_type(token_dimensions));
        

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
  CoSupport::Streams::dout << "Size of return-table: " << return_table.size() << std::endl;
#endif

  for(int firing_level = src_num_firing_levels-1; 
      firing_level >= (include_eff_token ? 0 : 1); //exclude effective token
      firing_level--){
    for(int token_dimension = token_dimensions-1; 
	token_dimension >= 0; 
	token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
      CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
      CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
      CoSupport::Streams::dout << "iteration_level = " << iteration_level << std::endl;
#endif
                        
      if (src_has_iteration_level(firing_level, token_dimension)){
	return_table[firing_level-(include_eff_token ? 0 : 1)][token_dimension] = 
          iteration_level;
	iteration_level++;
      }else{
	return_table[firing_level-(include_eff_token ? 0 : 1)][token_dimension] = -1;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 101
  CoSupport::Streams::dout << "Leave  smoc_wsdf_edge_descr::calc_src_iteration_level_table()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_table;
}



void smoc_wsdf_edge_descr::print_edge_parameters(std::ostream& os) const{
  os << "Token dimensions: " << token_dimensions << std::endl;
  os << "p = " << p << std::endl;
  os << "u0 = " << u0 << std::endl;
  os << "c = " << c << std::endl;
  os << "delta_c = " << delta_c << std::endl;
  os << "d = " << d << std::endl;
  os << "bs = " << bs << std::endl;
  os << "bt = " << bt << std::endl;
  os << "src_firing_blocks = " << src_firing_blocks << std::endl;
  os << "snk_firing_blocks = " << snk_firing_blocks << std::endl;
}


smoc_wsdf_edge_descr::s2vector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_level_table() const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  unsigned iteration_level = 0;

  //Generate a data structure with the same number of data elements than
  //snk_firing_blocks
  s2vector_type return_table(snk_firing_blocks.size(),
                             svector_type(snk_firing_block_dimensions));

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Size of return-table: " << return_table.size() << std::endl;
#endif

  for(int firing_level = snk_num_firing_levels-1; 
      firing_level >= 0; 
      firing_level--){
    for(int token_dimension = snk_firing_block_dimensions-1; 
	token_dimension >= 0; 
	token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
      CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
      CoSupport::Streams::dout << "iteration_level = " << iteration_level << std::endl;
#endif
                        
      if (snk_has_iteration_level(firing_level, 
                                  token_dimension,
                                  snk_firing_blocks)){
	return_table[firing_level][token_dimension] = iteration_level;
	iteration_level++;
      }else{
	return_table[firing_level][token_dimension] = -1;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave  smoc_wsdf_edge_descr::calc_snk_iteration_level_table()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_table;
}


void smoc_wsdf_edge_descr::insert_snk_vtu_firing_block() {


#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::insert_snk_vtu_firing_block" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  //Calculate the number of sink invocations per virtual token
  //union
  uvector_type snk_r_vtu(calc_snk_r_vtu());

  for(int token_dimension = snk_firing_block_dimensions-1;
      token_dimension >= 0;
      token_dimension--){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
#endif
      
      //calculate block_size
      snk_r_vtu(token_dimension) *= delta_c[token_dimension];
      if (get_scm_snk_firing_block(snk_r_vtu(token_dimension),
                                   token_dimension) != 1)
        //Cannot insert firing level for virtual token union
        assert(false);
  
      if (!insert_snk_firing_level(snk_r_vtu(token_dimension),
                                   token_dimension))
        //Cannot insert firing level
        assert(false);

  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::insert_snk_vtu_firing_block" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

}

unsigned 
smoc_wsdf_edge_descr::get_num_iteration_levels(const s2vector_type& snk_iteration_level_table) const {

  unsigned return_value;

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  return_value = snk_iteration_level_table.max_value()+1;
  
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "return_value = " << return_value << std::endl;
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::get_num_iteration_levels" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_value;
}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::get_vtu_iteration_level(const s2vector_type& snk_iteration_level_table) const{
  uvector_type snk_r_vtu(calc_snk_r_vtu());
  bvector_type found_vtu(token_dimensions,false);
  uvector_type return_vector(token_dimensions);
  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    for(unsigned int firing_level = 0;
        firing_level < snk_iteration_level_table.size();
        firing_level++){
      if (snk_firing_blocks[firing_level][token_dimension] ==
          snk_r_vtu[token_dimension]){
        return_vector[token_dimension] = 
          snk_iteration_level_table[firing_level][token_dimension];
        found_vtu[token_dimension] = true;
        break;
      }
    }
  }

  //error check
  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    assert(found_vtu[token_dimension]);
  }

  return return_vector;
}


unsigned smoc_wsdf_edge_descr::calc_src_iteration_levels() const {
  unsigned return_value = 0;

  if (cache_src_iteration_levels_valid)
    return src_iteration_levels_cached;

  for(unsigned firing_level = 0; 
      firing_level < src_num_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level,token_dimension)){
	return_value++;
      }
    }
  }

  src_iteration_levels_cached = return_value;
  cache_src_iteration_levels_valid = true;

  return return_value;
}


unsigned smoc_wsdf_edge_descr::calc_window_iteration_levels() const {
  unsigned return_value = 0;

  if (cache_num_window_iteration_levels_valid)
    return num_window_iteration_levels_cached;


  for(unsigned firing_level = 0; 
      firing_level < snk_window_firing_blocks.size(); 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){
	return_value++;
      }
    }
  }

  cache_num_window_iteration_levels_valid = true;
  num_window_iteration_levels_cached = return_value;

  return return_value;
}

unsigned smoc_wsdf_edge_descr::calc_eff_token_iteration_levels() const{

  unsigned return_value = 0;

  if (cache_eff_token_iteration_levels_valid)
    return eff_token_iteration_levels_cached;

  for(unsigned firing_level = 0; 
      firing_level < src_num_eff_token_firing_levels; 
      firing_level++){
    for(unsigned token_dimension = 0; 
	token_dimension < token_dimensions;
	token_dimension++){
      if (src_has_iteration_level(firing_level,
                                  token_dimension)){
	return_value++;
      }
    }
  }

  cache_eff_token_iteration_levels_valid = true;
  eff_token_iteration_levels_cached = return_value;

  return return_value;

}


smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_snk_iteration_max(const s2vector_type& snk_iteration_level_table) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
  CoSupport::Streams::dout << "snk_iteration_level_table.size() = " 
                           << snk_iteration_level_table.size();
#endif

  unsigned num_iteration_levels = 
    get_num_iteration_levels(snk_iteration_level_table);

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "num_iteration_levels = " << num_iteration_levels << std::endl;
#endif


  uvector_type 
    return_vector(num_iteration_levels);
  uvector_type 
    current_firing_block_size(snk_firing_block_dimensions);

  for(unsigned int i = 0; i < snk_firing_block_dimensions; i++)
    current_firing_block_size[i] = 1;

  for(unsigned token_dimension = 0;
      token_dimension < snk_firing_block_dimensions;
      token_dimension++){

    for(unsigned firing_level = 0; 
	firing_level < snk_num_firing_levels; 
	firing_level++){
      if(snk_iteration_level_table[firing_level][token_dimension] >= 0){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
	CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
#endif

	// check for complete blocks
	assert(snk_firing_blocks[firing_level][token_dimension] %
	       current_firing_block_size[token_dimension] == 0);
                                
	return_vector[snk_iteration_level_table[firing_level][token_dimension]] = 
	  snk_firing_blocks[firing_level][token_dimension] /
	  current_firing_block_size[token_dimension]
	  -1;                             

	// Update firing block size
	current_firing_block_size[token_dimension] = snk_firing_blocks[firing_level][token_dimension];
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_snk_iteration_max()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_vector;
}




void smoc_wsdf_edge_descr::append_snk_window_iteration(uvector_type& iteration_max) const {

  const unsigned old_size = iteration_max.size();
  const unsigned window_iteration_levels = calc_window_iteration_levels();
  const unsigned new_size = old_size + window_iteration_levels;
  iteration_max.resize(new_size);

  uvector_type current_block_size(token_dimensions,1);

  unsigned int iter_level = new_size-1;
  for(unsigned int firing_level = 0;
      firing_level < snk_window_firing_blocks.size();
      firing_level++){
    // Start with smallest block size.
    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){
        iteration_max[iter_level] = 
          snk_window_firing_blocks[firing_level][token_dimension]/
          current_block_size[token_dimension]
          -1;
        iter_level--;
        current_block_size[token_dimension] = 
          snk_window_firing_blocks[firing_level][token_dimension];
      }
    }
  }
}


smoc_wsdf_edge_descr::umatrix_type 
smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix(const s2vector_type& snk_iteration_level_table,
							   const uvector_type& snk_iter_max
							   ) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif

  uvector_type snk_r_vtu(calc_snk_r_vtu());

  uvector_type prev_mapping_factor(token_dimensions);
  for(unsigned int i = 0; i < token_dimensions; i++)
    prev_mapping_factor[i] = delta_c[i];

  bool found_vtu[token_dimensions];
  for(unsigned int i = 0; i < token_dimensions; i++)
    found_vtu[i] = false;

  const unsigned matrix_rows = token_dimensions;
  const unsigned matrix_cols = 
    get_num_iteration_levels(snk_iteration_level_table) +
    calc_window_iteration_levels();
        
  umatrix_type return_matrix(matrix_rows, matrix_cols);

  for(unsigned token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){
    for(unsigned firing_level = 0; 
	firing_level < snk_num_firing_levels; 
	firing_level++){                        
      if(snk_iteration_level_table[firing_level][token_dimension] >= 0){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
	CoSupport::Streams::dout << "token_dimension = " << token_dimension << std::endl;
#endif

	//default assignment
	for(unsigned row = 0; row < matrix_rows; row++){
	  return_matrix(row, 
                        snk_iteration_level_table[firing_level][token_dimension]) = 0;
	}

	return_matrix(token_dimension,snk_iteration_level_table[firing_level][token_dimension]) =
	  prev_mapping_factor[token_dimension];
                                
	//check, whether iteration level represents vtu
	if (snk_r_vtu[token_dimension] == 
            snk_firing_blocks[firing_level][token_dimension]){
	  //vtu requires special attention
	  prev_mapping_factor[token_dimension] =
	    u0[token_dimension];
	  found_vtu[token_dimension] = true;                                        
	}else{
	  prev_mapping_factor[token_dimension] *= 
	    (snk_iter_max[snk_iteration_level_table[firing_level][token_dimension]] + 1);
	}
      }
    }
  }  



  //Process virtual dimension if any
  if (token_dimensions < snk_firing_block_dimensions){
    for(unsigned firing_level = 0; 
	firing_level < snk_num_firing_levels; 
	firing_level++){                        
      if(snk_iteration_level_table[firing_level][token_dimensions] >= 0){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
	CoSupport::Streams::dout << "firing_level = " << firing_level << std::endl;
#endif

	//default assignment
	for(unsigned row = 0; row < matrix_rows; row++){
	  return_matrix(row, 
                        snk_iteration_level_table[firing_level][token_dimensions]) = 0;
	}
      }
    }
  }

        
  // Add special iterations
  insert_snk_window_mapping(return_matrix,
                            snk_iter_max);

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_snk_data_element_mapping_matrix()" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

  return return_matrix;

}


void 
smoc_wsdf_edge_descr::insert_snk_window_mapping(umatrix_type& data_element_mapping_matrix,
                                                const uvector_type& snk_iter_max) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::insert_snk_window_mapping" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
  CoSupport::Streams::dout << "Matrix-Cols: " << data_element_mapping_matrix.size2() << std::endl;
  CoSupport::Streams::dout << "snk_window_firing_blocks.size() = " 
                  << snk_window_firing_blocks.size() << std::endl;
  CoSupport::Streams::dout << "snk_iter_max.size() = " << snk_iter_max.size() << std::endl;
  CoSupport::Streams::dout.flush();
#endif


  const unsigned matrix_cols = data_element_mapping_matrix.size2();

  uvector_type current_block_size(token_dimensions,1);

  unsigned int iter_level = matrix_cols-1;
  for(unsigned int firing_level = 0;
      firing_level < snk_window_firing_blocks.size();
      firing_level++){
    // Start with smallest block size.

    for(unsigned int token_dimension = 0;
        token_dimension < token_dimensions;
        token_dimension++){
      if (snk_has_iteration_level(firing_level,
                                  token_dimension,
                                  snk_window_firing_blocks)){

        // Default value
        for(unsigned int i = 0;
            i < token_dimensions;
            i++)
          data_element_mapping_matrix(i, iter_level) = 0;

        data_element_mapping_matrix(token_dimension, iter_level) =
          current_block_size(token_dimension);
        current_block_size(token_dimension) *=
          snk_iter_max(iter_level)+1;

        iter_level--;
      }
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::insert_snk_window_mapping" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
  CoSupport::Streams::dout.flush();
#endif
}



void
smoc_wsdf_edge_descr::calc_border_condition_matrix(const umatrix_type& mapping_matrix,
                                                   const uvector_type& snk_vtu_iteration_level,
						   smatrix_type& border_cond_matrix) const {

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Enter smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
  CoSupport::Streams::dout << Indent::Up;
#endif
        
  const unsigned num_rows = mapping_matrix.size1();
  const unsigned num_cols = mapping_matrix.size2();


  for(unsigned row = 0; row < num_rows; row++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    CoSupport::Streams::dout << "row = " << row 
                             << ", vtu-iteration level = " 
                             << snk_vtu_iteration_level
                             << std::endl;
#endif
    for(unsigned col = 0; 
	col < snk_vtu_iteration_level[row];
	col++){
      border_cond_matrix(row,col) = 0;
    }
    for(unsigned col = snk_vtu_iteration_level[row];
	col < num_cols;
	col++){
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      CoSupport::Streams::dout << "col = " << col << std::endl; 
      CoSupport::Streams::dout << "mapping_matrix(row, col) = " 
		<< mapping_matrix(row, col) << std::endl;
#endif
      border_cond_matrix(row, col) = mapping_matrix(row, col);
    }
  }       

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  CoSupport::Streams::dout << "Leave smoc_wsdf_edge_descr::calc_border_condition_matrix" << std::endl;
  CoSupport::Streams::dout << Indent::Down;
#endif

}

const smoc_wsdf_edge_descr::uvector_type 
smoc_wsdf_edge_descr::calc_c(const uvector_type& ext_reusage) const{
  uvector_type return_c(c.size());
  
  for(unsigned int i = 0; i < return_c.size(); i++){
    if (c[i] > delta_c[i]){
      //check, whether remaining window is larger than zero,
      //otherwise illegal transformation
      assert(ext_reusage[i] < c[i]);

      //ext_reusage[i] lines are buffered externally
      return_c[i] = c[i] - ext_reusage[i];

      //Check for valid transformation
      assert(return_c[i] >= delta_c[i]);
    }else{
      //No data element is read twice. Hence, now reusage possible
      assert(ext_reusage[i] == 0);
      //Return enlarged window size as it simplifies analysis.
      return_c[i] = delta_c[i];
    }
  }

  return return_c;
}

bool
smoc_wsdf_edge_descr::calc_snk_firing_block(const uvector_type& new_c,
                                            u2vector_type& new_snk_firing_blocks,
                                            s2vector_type& new_snk_block_overlap,
                                            bool optimize_borders) const {
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << "Enter smoc_wsdf_edge_descr::calc_snk_firing_block" << std::endl;
  dout << Indent::Up;
  dout << "snk_firing_blocks = " << snk_firing_blocks << std::endl;
#endif
  bool return_value = true;
  new_snk_firing_blocks = snk_firing_blocks;
  
  //initialize new_snk_block_overlap
  new_snk_block_overlap = s2vector_type(snk_firing_blocks.size());
  for(unsigned int i = 0;
      i < new_snk_block_overlap.size();
      i++){
    new_snk_block_overlap[i] = 
      svector_type(token_dimensions,(sdata_type)0);
  }
  //dout << new_snk_block_overlap << std::endl;

  const uvector_type snk_r_vtu(calc_snk_r_vtu());

  //Some intermediate data
  sdata_type add_invocations[token_dimensions];

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << "Calculate number of additional invocations" << std::endl;
  dout << Indent::Up;
#endif

  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){      

    if(c[token_dimension] <= new_c[token_dimension]){
      //no external reusage.
      add_invocations[token_dimension] = 0;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      dout << "Skip dimension " << token_dimension << endl;
#endif
      continue;
    }

    /*
      Calculate the number of additional invocations
      due to window begin.
    */    
    //Number of additional data elements which must be read
    udata_type add_data_elements =
      c[token_dimension] - new_c[token_dimension];
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    dout << "add_data_elements = " << add_data_elements << endl;
#endif
    //Some of them might be situated on extended border
    if (optimize_borders && (bs[token_dimension] >= 0)){
      if (bs[token_dimension] <= (sdata_type)add_data_elements)
        add_data_elements -= bs[token_dimension];
      else
        add_data_elements = 0;
    }
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    dout << "After border-optimization: add_data_elements = " 
         << add_data_elements 
         << endl;
#endif

    //divide and round up in order to get number of invocations
    //Only the last delta_c[token_dimension] data elements are used
    //for reusage.
    add_invocations[token_dimension] =
      (add_data_elements + delta_c[token_dimension]-1)/
      delta_c[token_dimension];
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    dout << "add_invocations[token_dimension] = " 
         << add_invocations[token_dimension] << endl;
#endif


    /*
      Calculate the number of missing invocations due to
      window end
    */
    //What we are doing here is somehow a question of definition.
    //Only for common applications, it should match with the PARO 
    //system.
    //Hence we opt for the following (see also calc_bt):
    if (optimize_borders && 
        (bt[token_dimension] >= (sdata_type)new_c[token_dimension]) &&
        (c[token_dimension] > new_c[token_dimension])){
      add_invocations[token_dimension]--;
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
      dout << "Perform border optimization. add_invocations[token_dimension] = " 
           << add_invocations[token_dimension] << endl;
#endif
    }
#if 0
    udata_type missing_data_elements = 0;
    if (optimize_borders && (bt[token_dimension] >= 0))
      //Note: this could also be defined differently.
      missing_data_elements = bt[token_dimension];

    //divide and round down
    udata_type missing_invocations =
      missing_data_elements/new_c[token_dimension];


    //Overall number of additional invocations
    //Note: only valid if reading in the inner of a virtual token
    //      union is performed in raster scan order!
    add_invocations[token_dimension] -= missing_invocations;   
#endif

  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << Indent::Down;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  for(unsigned int i = 0; i < token_dimensions; i++){
    dout << "add_invocations[" << i << "]" << " = " 
         << add_invocations[i] << std::endl;
  }
#endif




  /*
    Modify firing blocks
   */
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << "Modify firing blocks" << std::endl;
  dout << Indent::Up;
#endif
  for(unsigned int token_dimension = 0;
      token_dimension < token_dimensions;
      token_dimension++){

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    dout << "token_dimension = " << token_dimension << std::endl;
#endif

    //In order to make the following steps easier
    //Note, that this does not cause a sever restriction
    //as a virtual token union of only one window is not
    //very useful.
    //In this case, we do not have any overlapping window
    //and we can just reformulate the problem.
    assert(snk_r_vtu[token_dimension] > 1);

    unsigned int firing_level = 0;
    //In the following we look for the first firing block
    //whose extension is larger than one.
    //Note, that this is not necessary, but somehow an
    //optimization process
    for(;firing_level < new_snk_firing_blocks.size();
        firing_level++){

      if (new_snk_firing_blocks[firing_level][token_dimension] > 1){
        break;
      }else{
#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
        dout << "Skip firing level " << firing_level << std::endl;
#endif
      }
    }
    if (firing_level == new_snk_firing_blocks.size()){
      firing_level--;
    }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
    dout << "Old firing blocks: " << new_snk_firing_blocks << std::endl;
    dout << "Extend firing_level " << firing_level 
         << " by " << add_invocations[token_dimension] << " invocations" 
         << std::endl;
#endif
    //Now extend size of firing block
    new_snk_firing_blocks[firing_level][token_dimension] +=
      add_invocations[token_dimension];    

    if (snk_firing_blocks[firing_level][token_dimension] !=
        snk_r_vtu[token_dimension]){
      //The new firing block consists of new_snk_firing_block invocations
      //from which the following ones are overlapping with the next
      //firing block in dimension token_dimension
      new_snk_block_overlap[firing_level][token_dimension] =
        add_invocations[token_dimension];
    }else{
      //Firing blocks do not overlap
    }

    //When we perform border optimization communication in non-raster
    //scan order can lead to firing blocks with different sizes.
    //This however is not supported by our WSDF fifo.
    if (optimize_borders){
      //worst case assumption: Doe only allow for border
      //optimization if virtual token union is read in raster scan
      //order.
      if (snk_firing_blocks[firing_level][token_dimension] !=
          snk_r_vtu[token_dimension])
        //transform failed
        return_value = false;
    }

    firing_level++;    

    //Propagate extension to all following firing blocks
    for(; firing_level < new_snk_firing_blocks.size();
        firing_level++){
      new_snk_firing_blocks[firing_level][token_dimension] =
        new_snk_firing_blocks[firing_level-1][token_dimension]*
        (snk_firing_blocks[firing_level][token_dimension]/
         snk_firing_blocks[firing_level-1][token_dimension]);     
    }
  }

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << Indent::Down;
#endif

#if VERBOSE_LEVEL_SMOC_WSDF_EDGE == 100
  dout << "Leave smoc_wsdf_edge_descr::calc_snk_firing_block" << std::endl;
  dout << Indent::Down;
#endif

  return return_value;
}


smoc_wsdf_edge_descr::u2vector_type
smoc_wsdf_edge_descr::calc_snk_firing_block(const uvector_type& new_c,
                                            bool optimize_borders) const{
  u2vector_type return_vector;
  s2vector_type temp_vector;
  bool temp = 
    calc_snk_firing_block(new_c,
                          return_vector,
                          temp_vector,
                          optimize_borders);
  assert(temp);

  for(unsigned int i = 0; i < temp_vector.size(); i++){
    for(unsigned int j = 0; j < token_dimensions; j++){
      //Currently overlapping firing blocks are not allowed
      assert(temp_vector[i][j] == 0);
    }
  }

  return return_vector;
                          
}

const smoc_wsdf_edge_descr::svector_type 
smoc_wsdf_edge_descr::calc_bs(const uvector_type& new_c,
                              bool optimize_borders
                              ) const{
  svector_type return_bs(bs.size());
  
  for(unsigned int i = 0; i < bs.size(); i++){
    
    if(new_c[i] >= c[i]){
      //no external reusage
      return_bs[i] = bs[i];
      continue;
    }    

    if (optimize_borders){
      //Number of additional data elements which must be read
      //see calc_snk_firing_block
      udata_type add_data_elements =
        c[i] - new_c[i];
      //Some of them might be situated on extended border
      if (bs[i] >= 0){
        if ((sdata_type)add_data_elements > bs[i]){
          add_data_elements -= bs[i];
        
          if (add_data_elements % delta_c[i] == 0)
            return_bs[i] = 0;
          else
            return_bs[i] = 
              delta_c[i] - (add_data_elements % delta_c[i]);
        }else{
          return_bs[i] = bs[i] - add_data_elements;
        }
      }else{
        if (add_data_elements % delta_c[i] == 0)
          return_bs[i] = bs[i];
        else
          return_bs[i] = bs[i] +
            delta_c[i] - (add_data_elements % delta_c[i]);
      }
    }else{
      //When no border optimization is performed
      //the extended border principally stays unchanged.
      //However, when the additional invocations do not
      //fit the image, the extended border has to be enlarged
      const udata_type missing_data_elements = 
        (c[i] - new_c[i]);
      if (missing_data_elements % delta_c[i] != 0)
        return_bs[i] = bs[i] + 
          delta_c[i] - (missing_data_elements % delta_c[i]);
      else
        return_bs[i] = bs[i];
    }
  }

  return return_bs;
}


const smoc_wsdf_edge_descr::svector_type 
smoc_wsdf_edge_descr::calc_bt(const uvector_type& new_c,
                              bool optimize_borders
                              ) const{
  svector_type return_bt(bt.size());
  
  for(unsigned int i = 0; i < bt.size(); i++){
    
    if(new_c[i] >= c[i]){
      //no external reusage
      return_bt[i] = bt[i];
      continue;
    }    

    if (optimize_borders){
      if ((bt[i] >= (sdata_type)new_c[i]))
        return_bt[i] = 
          bt[i] - delta_c[i];
      else
        return_bt[i] = bt[i];
    }else{
      return_bt[i] = bt[i];
    }
  }

  return return_bt;
}

