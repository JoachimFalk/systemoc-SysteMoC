// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_IMG_SNK_HPP
#define INCLUDE_WSDF_IMG_SNK_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include <CoSupport/DataTypes/SerializeHelper.hpp>

#include "CImg.h"

namespace ns_wsdf_img_snk {

  template<typename T2>
  struct m_wsdf_img_snk_type_map {
    typedef T2 type;
  };
  
  template<>
  struct m_wsdf_img_snk_type_map<CoSupport::DataTypes::Byte>{
    typedef unsigned char type;
  };
}
    

template <typename T = unsigned char>
class m_wsdf_img_snk: public smoc_actor {

private:

  cimg_library::CImg<typename ns_wsdf_img_snk::m_wsdf_img_snk_type_map<T>::type> output_image;
	
	const unsigned size_x;
	const unsigned size_y;

public:
	
	smoc_md_port_in<T,2> in;

private:

	void read_pixel(){
		//std::cout << "Read pixel (" << in.iteration(0,0) << "," << in.iteration(0,1) << ")";
		//std::cout << endl;
		output_image(in.iteration(0,0),in.iteration(0,1)) = in[0][0];
	}

	void store_image(){
		read_pixel();

		
		std::stringstream filename;

		filename << filename_base;
		filename << cur_file;
		filename << filename_ending;

		std::cout << "Store image " << filename.str() << std::endl;

		output_image.save(filename.str().c_str());
		cur_file++;
	}


	smoc_firing_state start;
	smoc_firing_state end;

	const std::string filename_base;
	const std::string filename_ending;
	
	unsigned cur_file;
	

public:
	m_wsdf_img_snk( sc_module_name name ,
									const std::string filename,
									unsigned size_x,
									unsigned size_y,
									unsigned num_files = 1)
		: smoc_actor(name,start),		
			output_image(size_x,size_y),
			size_x(size_x),
			size_y(size_y),
			in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
				 ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
				 ns_smoc_vector_init::ul_vector_init[1][1], //c
				 ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
				 ns_smoc_vector_init::sl_vector_init[0][0], //bs
				 ns_smoc_vector_init::sl_vector_init[0][0] //bt
				 ),
			filename_base(filename.substr(0,filename.rfind(".",filename.length()))),
			filename_ending(filename.substr(filename.rfind(".",filename.length()), filename.length())),
			cur_file(1)
	{
		std::cout << "num_files = " << num_files << endl;
		start = in(1) 
			>> ((in.getIteration(0,0) != (size_t)size_x-1) || (in.getIteration(0,1) != (size_t)size_y-1))
			>> CALL(m_wsdf_img_snk::read_pixel) 
			>> start
			| in(1)
			>> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1) && (VAR(this->cur_file) < num_files))
			>> CALL(m_wsdf_img_snk::store_image) 
			>> start
			| in(1)
			>> ((in.getIteration(0,0) == (size_t)size_x-1) && (in.getIteration(0,1) == (size_t)size_y-1) && (VAR(this->cur_file) == num_files))
			>> CALL(m_wsdf_img_snk::store_image) 
			>> end;	

	}
	
};


#endif //INCLUDE_WSDF_IMG_SNK_HPP
