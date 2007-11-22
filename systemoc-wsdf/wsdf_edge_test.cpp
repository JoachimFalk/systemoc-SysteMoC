
#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_md_port.hpp>

using namespace std;
using namespace ns_smoc_vector_init;

int main() {
  smoc_wsdf_edge_descr edge1(2,
                             //src firing blocks
                             ul_vector_init[4][3] <<
                             ul_vector_init[16][9],
                             //snk firing blocks
                             ul_vector_init[16][9],
                             //u0
                             ul_vector_init[16][9],
                             //c
                             ul_vector_init[3][3],
                             //delta_c
                             ul_vector_init[1][1],
                             //d
                             ul_vector_init[0][0],
                             //bs
                             sl_vector_init[1][1],
                             //bt
                             sl_vector_init[1][1]);

  edge1.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge1.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge1.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge1.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge1.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge1.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge1.src_data_element_mapping_matrix() << endl;



  /* Modify source iterator */
  cout << endl;
  cout << "Insert source firing level" << endl;
  cout << endl;
  edge1.insert_src_firing_level(2,0);
  edge1.insert_src_firing_level(8,0);
  edge1.insert_snk_firing_level(4,0);


  /* Print new edge parameters */
  

  edge1.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge1.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge1.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge1.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge1.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge1.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge1.src_data_element_mapping_matrix() << endl;
  

  exit(0);
                             
}
