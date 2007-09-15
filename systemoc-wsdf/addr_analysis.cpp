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
#include "snk2addr_table_win_iter.hpp"

using namespace std;
using namespace ns_smoc_vector_init;

#define WSDF_EXAMPLE_NBR 4



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
  const unsigned int image_width = 9;
  const unsigned int image_height = 4;
  const unsigned int block_width = 3;
  const unsigned int block_height = 2;
  smoc_wsdf_edge_descr 
    edge_e1(token_dimensions,
            ul_vector_init[1][1] << 
            ul_vector_init[image_width][image_height],   // src firing-blocks
            (ul_vector_init[block_width][block_height] <<
	     ul_vector_init[image_width][image_height]),   // snk firing-blocks
            ul_vector_init[image_width][image_height],   // u0
            ul_vector_init[1][1],   // c
            ul_vector_init[1][1],   // delta_c
            ul_vector_init[0][0],   // d
            sl_vector_init[0][0],   // bs
            sl_vector_init[0][0]    // bt
      );
#elif WSDF_EXAMPLE_NBR == 4
  //Block builder with embedded source
  // const unsigned int image_width = 27;
//   const unsigned int image_height = 9;
//   const unsigned int block_width = 3;
//   const unsigned int block_height = 3;
  const unsigned int image_width = 4096;
  const unsigned int image_height = 2140;
  //  const unsigned int block_width = 128;
  //  const unsigned int block_height = 5;
  const unsigned int block_width = 1024;
  const unsigned int block_height = 1070;
  smoc_wsdf_edge_descr 
    edge_e1(token_dimensions,
            ul_vector_init[1][1] << 
            ul_vector_init[block_width][1] << 
            ul_vector_init[image_width][block_height] << 
            ul_vector_init[image_width][image_height],   // src firing-blocks
            (ul_vector_init[block_width][block_height] <<
	     ul_vector_init[image_width][image_height]),   // snk firing-blocks
            ul_vector_init[image_width][image_height],   // u0
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

  /* Start to process reference points */ 
  snk2addr_table_ref_point ref_point_analyser(src_iter,
                                              snk_iter);
  const smoc_md_array<struct src_addr_info_struct> &
    ref_point_addr_offset_table(ref_point_analyser.get_ref_point_addr_offet_table());  

  /* Process window iterators */
  snk2addr_table_win_iter win_iter_analyser(src_iter,
                                            snk_iter,
                                            ref_point_addr_offset_table
                                            );

  const smoc_md_array<struct src_addr_info_struct> &
    window_addr_offset_table(win_iter_analyser.get_win_iter_addr_offet_table());

  CoSupport::dout << "Store tables ..." << std::endl;

  //store address offset table for window reference pixel
  {
    stringstream temp;
    temp << "snk2addr_ref_point_" << WSDF_EXAMPLE_NBR << ".txt";  
    ofstream outfile(temp.str().c_str());
    
    CoSupport::dout << "Store reference point table ..." << std::endl;
    outfile << ref_point_addr_offset_table;
    CoSupport::dout << "Close file ..." << std::endl;
    outfile.close();
  }

  //store address offset table for window iterator.
  {
    stringstream temp;
    temp << "snk2addr_win_iter_" << WSDF_EXAMPLE_NBR << ".txt";  
    ofstream outfile(temp.str().c_str());
    
    CoSupport::dout << "Store window iterator table ..." << std::endl;
    outfile << window_addr_offset_table;
    CoSupport::dout << "Close file ..." << std::endl;
    outfile.close();
  }

  CoSupport::dout << "Exit program ..." << std::endl;


}
