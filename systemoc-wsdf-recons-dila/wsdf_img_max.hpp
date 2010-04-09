// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_IMG_MAX_HPP
#define INCLUDE_WSDF_IMG_MAX_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>


///Returns the maximum value of an image
template <typename T>
class m_wsdf_img_max: public smoc_actor {

private:

  const unsigned size_x;
  const unsigned size_y;

public:
        
  smoc_md_port_in<T,2> in;
#ifndef PURE_MD_MODE
  smoc_port_out<T> out;
#else
  smoc_md_port_out<T,2> out;
#endif
        

private:

#ifndef PURE_MD_MODE
  void calc_max(){
    if (!max_valid){
      max_value = in[0][0];
      max_valid = true;
    }else{
      max_value = max_value < in[0][0] ? in[0][0] : max_value;
    }
  }
  void write_max(){
    calc_max();
    out[0] = max_value;
    max_valid = false;
  }
#else
  void calc_max(){
    T max_value = in[0][0];
    for(unsigned int x = 0; x < size_x; x++){
      for(unsigned int y = 0; y < size_y; y++){
        if (max_value < in[x][y])
          max_value = in[x][y];
      }
    }
    
    for(unsigned int x = 0; x < size_x; x++){
      for(unsigned int y = 0; y < size_y; y++){
        out[x][y] = max_value;
      }
    }

  }
#endif

  smoc_firing_state start;

#ifndef PURE_MD_MODE
  T max_value;
  bool max_valid;
#endif


public:
  m_wsdf_img_max( sc_module_name name ,
                  unsigned size_x,
                  unsigned size_y)
    : smoc_actor(name,start),               
      size_x(size_x),
      size_y(size_y),
#ifndef PURE_MD_MODE
      in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
         ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
         ns_smoc_vector_init::ul_vector_init[1][1], //c
         ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
         ns_smoc_vector_init::sl_vector_init[0][0], //bs
         ns_smoc_vector_init::sl_vector_init[0][0] //bt
         ),
      max_valid(false)
#else
    in(ns_smoc_vector_init::ul_vector_init[1][1], //firing_blocks
       ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
       ns_smoc_vector_init::ul_vector_init[size_x][size_y], //c
       ns_smoc_vector_init::ul_vector_init[size_x][size_y], //delta_c
       ns_smoc_vector_init::sl_vector_init[0][0], //bs
       ns_smoc_vector_init::sl_vector_init[0][0] //bt
       ),
    out(ns_smoc_vector_init::ul_vector_init[size_x][size_y] << //firing_blocks
        ns_smoc_vector_init::ul_vector_init[size_x][size_y])
#endif
  {
#ifndef PURE_MD_MODE
    start = in(1) 
      >> ((in.getIteration(0,0) != (size_t)size_x-1) || (in.getIteration(0,1) != (size_t)size_y-1))
      >> CALL(m_wsdf_img_max::calc_max) 
      >> start
      | (in(1) && out(1))
      >> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1))
      >> CALL(m_wsdf_img_max::write_max) 
      >> start;     
#else
    start = (in(1) && out(1))
      >> CALL(m_wsdf_img_max::calc_max) 
      >> start;
#endif  
  }
        
};


#endif //INCLUDE_WSDF_IMG_MAX_HPP
