//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "addr_analysis_data_struct.hpp"

#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_md_loop.hpp>
#include <systemoc/smoc_md_port.hpp>  // for vector init
#include <systemoc/smoc_md_array.hpp>

#include <cosupport/smoc_debug_out.hpp>

using namespace std;
using namespace ns_smoc_vector_init;

#define WSDF_EXAMPLE_NBR 2



struct src_addr_info_struct
calc_lin_addr(const smoc_snk_md_static_loop_iterator::iter_domain_vector_type& window_iterator,
              const smoc_src_md_static_loop_iterator& src_iter,
              const smoc_snk_md_static_loop_iterator& snk_iter,
              long schedule_period = 0){

  struct src_addr_info_struct return_value;

  const smoc_snk_md_static_loop_iterator::data_element_id_type&
    base_data_element_id(snk_iter.get_base_data_element_id());

  //check, whether pixel is situated on extended border
  bool is_ext_border;
  snk_iter.is_ext_border(window_iterator,
                         is_ext_border);      

  if (!is_ext_border){
    // pixel must be produced by source actor
    
    smoc_src_md_static_loop_iterator::iter_domain_vector_type 
      src_iteration(src_iter.iterator_depth());
    smoc_src_md_static_loop_iterator::id_type
      schedule_period_offset;

    //Get required data element
    smoc_snk_md_static_loop_iterator::data_element_id_type
      data_element_id(snk_iter.token_dimensions());

    snk_iter.get_window_data_element_offset(window_iterator,
                                            data_element_id);
    data_element_id += base_data_element_id;

      
    CoSupport::dout << "Required data element: " << data_element_id << std::endl;


    // Get required source iteration
    bool temp =
      src_iter.get_src_loop_iteration(data_element_id,
                                      src_iteration,
                                      schedule_period_offset
                                      );
    assert(temp);

    //Eliminate schedule period offset
    src_iteration[src_iteration.size()-1] +=
      schedule_period_offset*
      src_iter.iteration_size()[src_iteration.size()-1];

    CoSupport::dout << "src_iteration = " << src_iteration << std::endl;

    //calculate linearized address
    long lin_addr = 
      src_iter.calc_iteration_id(src_iteration,
                                 schedule_period);

    CoSupport::dout << "lin_addr = " << lin_addr << std::endl;

    return_value.abs_addr = lin_addr;
    return_value.valid = true;
        
          
  }else{
    // data element is not produced by source actor
    // Insert a don't care into the table

    CoSupport::dout << "Situated on extended border" << std::endl;

    // By default, we set the address to zero.
    // However, we have to take the schedule period into account.

    smoc_src_md_static_loop_iterator::iter_domain_vector_type 
      src_iteration(src_iter.iterator_depth(),
                    (smoc_src_md_static_loop_iterator::iter_item_type)0);
    return_value.abs_addr = 
      src_iter.calc_iteration_id(src_iteration,
                                 schedule_period);
    return_value.valid = false;
    
  }

  return return_value;

}



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

  //window iterator for reference point
  smoc_snk_md_static_loop_iterator::iter_domain_vector_type
    ref_window_iterator(snk_iter.max_window_iteration()/2);
  //smoc_snk_md_static_loop_iterator::iter_domain_vector_type
  //  ref_window_iterator(snk_iter.token_dimensions(),
  //                      (smoc_snk_md_static_loop_iterator::iter_item_type)0);

  CoSupport::dout << "Referenz Window Iterator = "
                  << ref_window_iterator
                  << std::endl;

  //default step size applied when
  //the reference address is not valid
  const long default_addr_step = 1;

  //result tables
  smoc_md_array<struct src_addr_info_struct> 
    ref_point_addr_offset_table(snk_iter.iterator_depth()-
                                snk_iter.token_dimensions(),
                                snk_iter.iteration_size());
  
  smoc_md_array<struct src_addr_info_struct> 
    window_addr_offset_table(snk_iter.iterator_depth(),
                             snk_iter.iteration_size());


  /* Start to process reference points */ 
  
  //Note: if the first reference iterator does not access
  //      a valid address, we assume, that it uses
  //      the data element produced by the source
  //      iteration zero.
  long schedule_period = 0;
  struct src_addr_info_struct prev_win_addr_info = 
    calc_lin_addr(ref_window_iterator,
                  src_iter,
                  snk_iter);
  bool finished = false;
  while(!finished){
    const smoc_snk_md_static_loop_iterator::iter_domain_vector_type
      prev_snk_iter_vector(snk_iter.iteration_vector());

    finished = snk_iter.inc();
    if (finished){
      // we have started a new schedule period.
      schedule_period++;
    }

    //Attention: For iteration (0,0,0...) we store the
    //           address offset in order to reach the NEXT
    //           iteration. We also store the absolute
    //           value of the NEXT iteration!!!
    ref_point_addr_offset_table[prev_snk_iter_vector] =
      calc_lin_addr(ref_window_iterator,
                    src_iter,
                    snk_iter,
                    schedule_period);

    //Calculate relative address
    ref_point_addr_offset_table[prev_snk_iter_vector].rel_addr =
      ref_point_addr_offset_table[prev_snk_iter_vector].abs_addr-
      prev_win_addr_info.abs_addr;
    
    prev_win_addr_info = 
      ref_point_addr_offset_table[prev_snk_iter_vector];
  }



  /* Process all window pixels */ 
  //Note: here snk_iter = (0,0,0,...) again!!
  do {

    //First, get reference address
    struct src_addr_info_struct ref_addr_info =
      calc_lin_addr(ref_window_iterator,
                    src_iter,
                    snk_iter);
    
    smoc_snk_md_static_loop_iterator::iter_domain_vector_type
      window_iterator(snk_iter.token_dimensions(),
                      (smoc_snk_md_static_loop_iterator::iter_item_type)0);
    
    // Iteration over all window elements
    bool end_of_window = false;
    while(!end_of_window){

      // Set window iterator, such that we can use the
      // iteration vector as array index
      snk_iter.set_window_iterator(window_iterator);

      CoSupport::dout << "Iteration: " << snk_iter.iteration_vector() << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;


      window_addr_offset_table[snk_iter.iteration_vector()] = 
        calc_lin_addr(window_iterator,
                      src_iter,
                      snk_iter);

      // Transform into relative address (relative to
      // reference window pixel.
      window_addr_offset_table[snk_iter.iteration_vector()].rel_addr =
        window_addr_offset_table[snk_iter.iteration_vector()].abs_addr -
        ref_addr_info.abs_addr;
        

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

    
  }while (!snk_iter.inc());


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
