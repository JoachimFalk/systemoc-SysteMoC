//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_md_loop.hpp>
#include <systemoc/smoc_md_port.hpp>  // for vector init
#include <systemoc/smoc_md_array.hpp>

#include <cosupport/smoc_debug_out.hpp>

#include "snk2addr_table_ref_point.hpp"

using namespace std;
using namespace ns_smoc_vector_init;

#define WSDF_EXAMPLE_NBR 2



int main(){

  /* WSDF edge description */

  const unsigned token_dimensions = 2;  
        
        
#if WSDF_EXAMPLE_NBR == 1
  smoc_wsdf_edge_descr 
    edge_e1(token_dimensions,
            ul_vector_init[1][1] << 
            ul_vector_init[9][9],   // src firing-blocks
            ul_vector_init[9][9],   // snk firing-blocks
            ul_vector_init[9][9],   // u0
            ul_vector_init[1][1],   // c
            ul_vector_init[1][1],   // delta_c
            ul_vector_init[0][0],   // d
            sl_vector_init[0][0],   // bs
            sl_vector_init[0][0]    // bt
      );
#elif WSDF_EXAMPLE_NBR == 2
  smoc_wsdf_edge_descr 
    edge_e1(token_dimensions,
            ul_vector_init[1][1] << 
            ul_vector_init[9][9],   // src firing-blocks
            ul_vector_init[9][9],   // snk firing-blocks
            ul_vector_init[9][9],   // u0
            ul_vector_init[3][3],   // c
            ul_vector_init[1][1],   // delta_c
            ul_vector_init[0][0],   // d
            sl_vector_init[1][1],   // bs
            sl_vector_init[1][1]    // bt
      );
#elif WSDF_EXAMPLE_NBR == 3
  smoc_wsdf_edge_descr 
    edge_e1(token_dimensions,
            ul_vector_init[1][1] << 
            ul_vector_init[9][9],   // src firing-blocks
            (ul_vector_init[3][3] <<
	     ul_vector_init[9][9]),   // snk firing-blocks
            ul_vector_init[9][9],   // u0
            ul_vector_init[1][1],   // c
            ul_vector_init[1][1],   // delta_c
            ul_vector_init[0][0],   // d
            sl_vector_init[0][0],   // bs
            sl_vector_init[0][0]    // bt
      );
#else
# error "No example configuration defined"
#endif


  /* Source and sink iterators */
  smoc_snk_md_static_loop_iterator 
    snk_iter(edge_e1.snk_iteration_max(),
             edge_e1.snk_data_element_mapping_matrix(),
             edge_e1.snk_data_element_mapping_vector(),
             edge_e1.calc_border_condition_matrix(),
             edge_e1.calc_low_border_condition_vector(),
             edge_e1.calc_high_border_condition_vector());


  smoc_src_md_static_loop_iterator
    src_iter(edge_e1.src_iteration_max(),
             edge_e1.src_data_element_mapping_matrix(),
             edge_e1.src_data_element_mapping_vector()
             );

  // Address analysis is performed in two steps.
  // Therefore, we identify a window reference point.
  // For each sink loop iterator we identify then
  // then the address offset required in order to
  // obtain the address of the next sliding window.
  //
  // In a second step, we then identify the addresses
  // of all sliding windows pixels relative to the 
  // reference pixel.

  //result tables
  smoc_md_array<struct src_addr_info_struct> 
    window_addr_offset_table(snk_iter.iterator_depth(),
                             snk_iter.iteration_size());


  /* Start to process reference points */ 
  snk2addr_table_ref_point ref_point_analyser(src_iter,
                                              snk_iter);
  const smoc_md_array<struct src_addr_info_struct> &
    ref_point_addr_offset_table(ref_point_analyser.get_ref_point_addr_offet_table());  

#if 0
  /* Process all window pixels */ 
  //Note: here snk_iter = (0,0,0,...) again!!
  do {
    smoc_snk_md_static_loop_iterator::iter_domain_vector_type
      window_iterator(snk_iter.token_dimensions(),
                      (smoc_snk_md_static_loop_iterator::iter_item_type)0);
    
    // Iteration over all window elements
    bool end_of_window = false;
    while(!end_of_window){

      // Set window iterator, such that we can use the
      // iteration vector as array index
      snk_iter.set_window_iterator(window_iterator);

      CoSupport::dout << "Iteration: " 
                      << snk_iter.iteration_vector() 
                      << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;


      bool addr_valid;
      long lin_addr = 
        calc_lin_addr(window_iterator,
                      src_iter,
                      snk_iter,
                      addr_valid);

      //Note, that here the structure names are not very nice.
      //Perhaps it would be better to introduce a new structure.
      //Store absolute address
      window_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr =
        lin_addr;
      //Calculate relative address to reference pixel
      window_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr =
        lin_addr - 
        ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr;
      
      window_addr_offset_table[snk_iter.iteration_vector()].curr_addr_valid =
        window_addr_offset_table[snk_iter.iteration_vector()].next_addr_valid =
        addr_valid;

      CoSupport::dout << "Absolute address: " << lin_addr << std::endl;
      CoSupport::dout << "Relative address: " 
                      << window_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr 
                      << std::endl;
      CoSupport::dout << (addr_valid ? "address valid" : "address not valid")
                      << std::endl;

      CoSupport::dout << "Move to next window ..." << std::endl;

      end_of_window = true;
      //Move to next window position
      for(int i = snk_iter.token_dimensions()-1;
          i >= 0;
          i--){
        window_iterator[i]++;
        if (window_iterator[i] > snk_iter.max_window_iteration()[i]){
          window_iterator[i] = 0;
        }else{
          end_of_window = false;
          break;
        }
      }

      CoSupport::dout << CoSupport::Indent::Down;

    } //iteration of window

    
  }while(!snk_iter.inc());

#endif

  //store table
  {
    stringstream temp;
    temp << "snk2addr_" << WSDF_EXAMPLE_NBR << ".txt";  
    ofstream outfile(temp.str().c_str());
    outfile << ref_point_addr_offset_table;
    outfile << window_addr_offset_table;
    outfile.close();
  }



}
