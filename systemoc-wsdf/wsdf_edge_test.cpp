
#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_md_port.hpp>

using namespace std;
using namespace ns_smoc_vector_init;

int main() {

  cout << " ************************** Example 1 ************************** " << endl;

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
  
  cout << endl; cout << endl;

  
  

  /* Second example */
  cout << " ************************** Example 2 ************************** " << endl;
  smoc_wsdf_edge_descr edge2(2,
                             //src firing blocks
                             ul_vector_init[6][9] <<
                             ul_vector_init[36][9],
                             //snk firing blocks
                             ul_vector_init[36][9],
                             //u0
                             ul_vector_init[36][9],
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
  edge2.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge2.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge2.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge2.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge2.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge2.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge2.src_data_element_mapping_matrix() << endl;

  cout << endl;

  
  /* Modify iterator */
  cout << "Try to insert block of size 9" << std::endl;
  unsigned long multiplicity = edge2.get_scm_src_firing_block(9,0);
  cout << "Multiplicity: " << multiplicity << std::endl;
  edge2.insert_src_firing_level(multiplicity*9,0);

  /* Print new parameters */
  edge2.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge2.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge2.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge2.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge2.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge2.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge2.src_data_element_mapping_matrix() << endl;

  cout << endl;   cout << endl;   cout << endl;




  /* Third example */
  cout << " ************************** Example 3 ************************** " << endl;
  smoc_wsdf_edge_descr edge3(3,
                             //src firing blocks
                             ul_vector_init[1][1][1] <<
                             ul_vector_init[9][6][3] <<
                             ul_vector_init[81][36][9],
                             //snk firing blocks
                             ul_vector_init[81][36][9],
                             //u0
                             ul_vector_init[81][36][9],
                             //c
                             ul_vector_init[1][1][1],
                             //delta_c
                             ul_vector_init[1][1][1],
                             //d
                             ul_vector_init[0][0][0],
                             //bs
                             sl_vector_init[0][0][0],
                             //bt
                             sl_vector_init[0][0][0]);

  edge3.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge3.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge3.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge3.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge3.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge3.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge3.src_data_element_mapping_matrix() << endl;

  cout << endl;

  

  /* Modify iterator */
  edge3.firing_levels_src2snk();
  edge3.firing_levels_snk2src();

  edge3.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge3.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge3.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge3.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge3.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge3.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge3.src_data_element_mapping_matrix() << endl;  

  cout << endl; cout << endl; cout << endl;



  cout << " ************************** Example 4 ************************** " << endl;
  smoc_wsdf_edge_descr edge4(3,
                             //src firing blocks
                             ul_vector_init[1][1][1] <<
                             ul_vector_init[81][36][9],
                             //snk firing blocks
                             ul_vector_init[9][6][3] <<
                             ul_vector_init[81][36][9],
                             //u0
                             ul_vector_init[81][36][9],
                             //c
                             ul_vector_init[1][1][1],
                             //delta_c
                             ul_vector_init[1][1][1],
                             //d
                             ul_vector_init[0][0][0],
                             //bs
                             sl_vector_init[0][0][0],
                             //bt
                             sl_vector_init[0][0][0]);

  edge4.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge4.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge4.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge4.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge4.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge4.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge4.src_data_element_mapping_matrix() << endl;

  cout << endl;

  

  /* Modify iterator */
  edge4.firing_levels_src2snk();
  edge4.firing_levels_snk2src();

  edge4.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge4.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge4.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge4.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge4.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge4.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge4.src_data_element_mapping_matrix() << endl;  

  cout << endl;


  
  cout << " ************************** Example 5 ************************** " << endl;
  smoc_wsdf_edge_descr edge5(2,
                             //src firing blocks
                             ul_vector_init[1][1] <<
                             ul_vector_init[6][3] <<
                             ul_vector_init[36][12],
                             //snk firing blocks
                             ul_vector_init[1][1] <<
                             ul_vector_init[9][4] <<
                             ul_vector_init[36][12],
                             //u0
                             ul_vector_init[36][12],
                             //c
                             ul_vector_init[1][1],
                             //delta_c
                             ul_vector_init[1][1],
                             //d
                             ul_vector_init[0][0],
                             //bs
                             sl_vector_init[0][0],
                             //bt
                             sl_vector_init[0][0]);

  edge5.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge5.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge5.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge5.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge5.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge5.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge5.src_data_element_mapping_matrix() << endl;

  cout << endl; cout << endl; cout << endl;

  

  /* Modify iterator */
  edge5.firing_levels_src2snk();
  edge5.firing_levels_snk2src();

  edge5.print_edge_parameters(cout);
  
  cout << endl;

  cout << "Sink iteration max: " << edge5.snk_iteration_max() << std::endl;
  cout << "Source iteration max: " << edge5.src_iteration_max() << std::endl;

  cout << endl;
  
  cout << "Sink data element mapping vector: " 
       << edge5.snk_data_element_mapping_vector() << endl;
  cout << "Snk data element mapping matrix: " 
       << edge5.snk_data_element_mapping_matrix() << endl;

  cout << endl;

  cout << "Source data element mapping vector: " 
       << edge5.src_data_element_mapping_vector() << endl;
  cout << "Source data element mapping matrix: " 
       << edge5.src_data_element_mapping_matrix() << endl;  

  cout << endl; cout << endl; cout << endl;



  exit(0);
                             
}
