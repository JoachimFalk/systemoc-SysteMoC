//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

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
#endif

using namespace std;
using namespace ns_smoc_vector_init;

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
using namespace smoc_md_ba;
#endif

#define WSDF_EXAMPLE_NBR 3

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
#else
# error "No example configuration defined"
#endif

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  smoc_md_ba_ui_schedule buffer_schedule_analysis_obj("snk_self_schedule.txt");
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
      my_channel->commitWrite(1,dummy_event);
      if (my_channel->src_new_schedule_period())
        current_src_schedule_period++;
    }else{
      break;
    }
  }

  delete my_channel;
        

}
