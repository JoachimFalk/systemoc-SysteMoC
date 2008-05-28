//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_graph_type.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
# include <systemoc/smoc_md_ba_linearized_buffer_schedule.hpp>
# include <systemoc/smoc_md_ba_linearized_buffer_size.hpp>
#endif

#define BUFFER_ANALYSIS_TYPE 1
//0 : buffer size
//1 : schedule

using namespace std;
using namespace ns_smoc_vector_init;

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
using namespace smoc_md_ba;
#endif

#define WSDF_EXAMPLE_NBR 4

int sc_main(int argc, char *argv[]) {} // dummy SystemC requires it!

int main(){

  /* Number of schedule periods to simulate */
  const unsigned long max_src_schedule_periods = 1;

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
#elif WSDF_EXAMPLE_NBR == 4
  //Block builder with embedded source
  // const unsigned int image_width = 27;
//   const unsigned int image_height = 9;
//   const unsigned int block_width = 3;
//   const unsigned int block_height = 3;
  const unsigned int image_width = 4096;
  const unsigned int image_height = 2140;
  const unsigned int block_width = 128;
  const unsigned int block_height = 5;
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

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
# if BUFFER_ANALYSIS_TYPE == 0
  smoc_md_ba_ui_lin_buffer_size buffer_schedule_analysis_obj;
# elif BUFFER_ANALYSIS_TYPE == 1
  smoc_md_ba_ui_schedule buffer_schedule_analysis_obj;
# else
# error "wrong mode"
# endif
#endif
        
  smoc_md_fifo<void> my_fifo(edge_e1,
			     MAX_TYPE(size_t)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
			     , &buffer_schedule_analysis_obj
#endif
    );

  smoc_md_fifo<void>::chan_type *my_channel = 
    new smoc_md_fifo<void>::chan_type(my_fifo);


#ifdef SYSTEMOC_ENABLE_VPC
  smoc_ref_event_p dummy_event;
#endif  

  unsigned long current_src_schedule_period = 0;

  while(true){
    while(my_channel->numAvailable() > 0){
#ifdef SYSTEMOC_ENABLE_VPC
      my_channel->commitRead(1,dummy_event);
#else
      my_channel->commitRead(1);
#endif
    }   
    if (current_src_schedule_period <= max_src_schedule_periods){
#ifdef SYSTEMOC_ENABLE_VPC
      my_channel->commitWrite(1,dummy_event);
#else
      my_channel->commitWrite(1);
#endif
      if (my_channel->src_new_schedule_period())
        current_src_schedule_period++;
    }else{
      break;
    }
  }

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS

# if BUFFER_ANALYSIS_TYPE == 0
  std::cout << "buffer size: " << buffer_schedule_analysis_obj.get_buffer_size() << std::endl;
# elif BUFFER_ANALYSIS_TYPE == 1
  
  {
    stringstream temp;
    temp << "snk2src_" << WSDF_EXAMPLE_NBR << ".txt";  
    ofstream outfile(temp.str().c_str());
    outfile << buffer_schedule_analysis_obj.get_snk2src_invocation_table();
    outfile.close();
  }

  {
    stringstream temp;
    temp << "src2snk_" << WSDF_EXAMPLE_NBR << ".txt";  
    ofstream outfile(temp.str().c_str());
    outfile << buffer_schedule_analysis_obj.get_src2snk_invocation_table();
    outfile.close();
  }
  
# endif
#endif

  delete my_channel;
        

}
