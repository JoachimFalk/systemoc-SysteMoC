// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "wsdf_input_switch.hpp"
#include "wsdf_seed_switch.hpp"
#include "1D_dup2.hpp"
#include "wsdf_min_image.hpp"
#include "wsdf_adiff_image.hpp"
#include "wsdf_img_max.hpp"
#include "wsdf_img_snk.hpp"
#include "wsdf_img_src.hpp"
#include "wsdf_2D_dup2.hpp"
#include "wsdf_1dil.hpp"
#include "wsdf_output_switch.hpp"

#define CAPTURE_DILATATION
#define LARGE_BUFFER_SIZE


using namespace std;

template <typename T = unsigned char>
class m_recons 
	: public smoc_graph {

public:
	smoc_md_iport_in<T,2>  seed_in;
	smoc_md_iport_in<T,2>  image_in;

	smoc_md_iport_out<T,2> out;
    
	m_recons( sc_module_name name,
						unsigned size_x,
						unsigned size_y)
		: smoc_graph(name)
	{
		m_wsdf_seed_switch<T> &seed_switch = 
			registerNode(new m_wsdf_seed_switch<T>("seed_switch", size_x, size_y));
		m_wsdf_input_switch<T> &input_switch = 
			registerNode(new m_wsdf_input_switch<T>("input_switch", size_x, size_y));

		m_wsdf_2D_dup2<T> &seed_dup = 
			registerNode(new m_wsdf_2D_dup2<T>("seed_dup", size_x, size_y));
		m_wsdf_2D_dup2<T> &input_dup = 
			registerNode(new m_wsdf_2D_dup2<T>("input_dup", size_x, size_y));
		m_wsdf_2D_dup2<T> &min_image_dup = 
			registerNode(new m_wsdf_2D_dup2<T>("min_image_dup", size_x, size_y));
		
		m_wsdf_1dil<T> &dilatation = 
			registerNode(new m_wsdf_1dil<T>("dilatation", size_x, size_y));
		m_wsdf_min_image<T> &min_image = 
			registerNode(new m_wsdf_min_image<T>("min_image", size_x, size_y));
		m_wsdf_adiff_image<T> &diff_image = 
			registerNode(new m_wsdf_adiff_image<T>("diff_image", size_x, size_y));
		m_wsdf_img_max<T> &image_max = 
			registerNode(new m_wsdf_img_max<T>("image_max", size_x, size_y));

		m_1D_dup2<T> &dup_1D_1 = 
			registerNode(new m_1D_dup2<T>("1D_dup2_1"));
		m_1D_dup2<T> &dup_1D_2 = 
			registerNode(new m_1D_dup2<T>("1D_dup2_2"));
		
		m_wsdf_output_switch<T> &output_switch = 
			registerNode(new m_wsdf_output_switch<T>("output_switch", size_x, size_y));

#ifdef CAPTURE_DILATATION
		//debug actors
		m_wsdf_2D_dup2<T> &dil_dup = 
			registerNode(new m_wsdf_2D_dup2<T>("dilatation_dup", size_x, size_y));
		m_wsdf_img_snk<T> &dil_sink = registerNode(new m_wsdf_img_snk<T>("dil_sink","dil_out.bmp",size_x,size_y,1000));		
#endif //CAPTURE_DILATATION

		/* Connect all node outputs */
		
		seed_switch.orig_in(seed_in);
		input_switch.orig_in(image_in);
		
#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts( seed_switch.out, seed_dup.in, smoc_wsdf_edge<T>(size_y));
		indConnectNodePorts( input_switch.out, input_dup.in, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts( seed_switch.out, seed_dup.in, smoc_wsdf_edge<T>(1));
		indConnectNodePorts( input_switch.out, input_dup.in, smoc_wsdf_edge<T>(1));
#endif

#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(input_dup.out1, min_image.in2, smoc_wsdf_edge<T>(size_y));		
#else
		indConnectNodePorts(input_dup.out1, min_image.in2, smoc_wsdf_edge<T>(1));		
#endif
		indConnectNodePorts(input_dup.out2, input_switch.buffered_in, smoc_wsdf_edge<T>(size_y,ns_smoc_vector_init::ul_vector_init[0][size_y]));
		
#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(seed_dup.out1, dilatation.in, smoc_wsdf_edge<T>(size_y));
		indConnectNodePorts(seed_dup.out2, diff_image.in1, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts(seed_dup.out1, dilatation.in, smoc_wsdf_edge<T>(3));
		indConnectNodePorts(seed_dup.out2, diff_image.in1, smoc_wsdf_edge<T>(3));
#endif

#ifdef CAPTURE_DILATATION
# ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(dilatation.out, dil_dup.in, smoc_wsdf_edge<T>(size_y));

		indConnectNodePorts(dil_dup.out1, min_image.in1, smoc_wsdf_edge<T>(size_y));
		indConnectNodePorts(dil_dup.out2, dil_sink.in, smoc_wsdf_edge<T>(size_y));
# else
		indConnectNodePorts(dilatation.out, dil_dup.in, smoc_wsdf_edge<T>(1));

		indConnectNodePorts(dil_dup.out1, min_image.in1, smoc_wsdf_edge<T>(1));
		indConnectNodePorts(dil_dup.out2, dil_sink.in, smoc_wsdf_edge<T>(1));
# endif
#else
# ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(dilatation.out, min_image.in1, smoc_wsdf_edge<T>(size_y));
# else
		indConnectNodePorts(dilatation.out, min_image.in1, smoc_wsdf_edge<T>(1));
# endif
#endif
		
#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(min_image.out, min_image_dup.in, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts(min_image.out, min_image_dup.in, smoc_wsdf_edge<T>(1));
#endif

#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(min_image_dup.out1, diff_image.in2, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts(min_image_dup.out1, diff_image.in2, smoc_wsdf_edge<T>(1));
#endif
		indConnectNodePorts(min_image_dup.out2, output_switch.in, smoc_wsdf_edge<T>(size_y));
		
#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(diff_image.out,image_max.in,smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts(diff_image.out,image_max.in,smoc_wsdf_edge<T>(1));
#endif
		connectNodePorts(image_max.out, dup_1D_1.in);

		connectNodePorts(dup_1D_1.out1, dup_1D_2.in, smoc_fifo<T>(1) << 0);
		connectNodePorts(dup_1D_1.out2, output_switch.max_in);

		connectNodePorts(dup_1D_2.out1, seed_switch.max_in);
		connectNodePorts(dup_1D_2.out2, input_switch.max_in);

#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts(output_switch.intermediate_out,seed_switch.intermediate_in, smoc_wsdf_edge<T>(size_y));
#else
		indConnectNodePorts(output_switch.intermediate_out,seed_switch.intermediate_in, smoc_wsdf_edge<T>(1));
#endif
		output_switch.final_out(out);		
		

		
	}
	
};

template <typename T = unsigned char>
class m_top
	: public smoc_graph 
{
public:
	m_top( sc_module_name name,
				 const char* inputfile_image,
				 const char* inputfile_seed,
				 const char* output_filename)
		: smoc_graph(name) {
	
		m_wsdf_img_src<T> &src_image = registerNode(new m_wsdf_img_src<T>("src_image",inputfile_image));
		m_wsdf_img_src<T> &src_seed = registerNode(new m_wsdf_img_src<T>("src_seed",inputfile_seed));

		assert(src_image.size_x == src_seed.size_x);
		assert(src_image.size_y == src_seed.size_y);
	
		m_wsdf_img_snk<T> &sink = registerNode(new m_wsdf_img_snk<T>("sink",output_filename,src_image.size_x,src_image.size_y,3));
		m_recons<T> &recons = registerNode(new m_recons<T>("recons", src_image.size_x,src_image.size_y));
	
#ifndef KASCPAR_PARSING
#ifdef LARGE_BUFFER_SIZE
		indConnectNodePorts( src_image.out, recons.image_in, smoc_wsdf_edge<T>(src_image.size_y));
		indConnectNodePorts( src_seed.out, recons.seed_in, smoc_wsdf_edge<T>(src_image.size_y));
		indConnectNodePorts( recons.out, sink.in, smoc_wsdf_edge<T>(src_image.size_y));
#else
		indConnectNodePorts( src_image.out, recons.image_in, smoc_wsdf_edge<T>(1));
		indConnectNodePorts( src_seed.out, recons.seed_in, smoc_wsdf_edge<T>(1));
		indConnectNodePorts( recons.out, sink.in, smoc_wsdf_edge<T>(1));
#endif
#endif
	}
};

int sc_main (int argc, char **argv) {
	if(argc < 3) {
		cout << "design-wsdf-recons-dila <input-file><seed-file><output-file>" << endl;
		return -1;
	}

  smoc_top_moc<m_top<unsigned char> > top("top", argv[1],argv[2],argv[3]);
  
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start();
  }
#undef GENERATE
#endif
  return 0;
}
