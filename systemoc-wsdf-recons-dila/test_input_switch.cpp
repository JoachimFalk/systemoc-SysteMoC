// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_md_fifo.hpp>
#include <smoc_wsdf_edge.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
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


using namespace std;

template <typename T = unsigned char>
class m_recons 
	: public smoc_graph {

public:
	smoc_md_iport_in<T,2>  image_in;
	smoc_port_in<T>        ctrl_in;

	smoc_md_iport_out<T,2> out;
    
	m_recons( sc_module_name name,
						unsigned size_x,
						unsigned size_y)
		: smoc_graph(name)
	{
		m_wsdf_input_switch<T> &input_switch = 
			registerNode(new m_wsdf_input_switch<T>("input_switch", size_x, size_y));

		m_wsdf_2D_dup2<T> &input_dup = 
			registerNode(new m_wsdf_2D_dup2<T>("input_dup", size_x, size_y));


		/* Connect all node outputs */
		
		connectInterfacePorts( image_in, input_switch.orig_in );
		connectInterfacePorts( ctrl_in, input_switch.max_in);
		
		indConnectNodePorts( input_switch.out, input_dup.in, smoc_wsdf_edge<T>(1));

		indConnectNodePorts(input_dup.out2, input_switch.buffered_in, smoc_wsdf_edge<T>(size_y,ns_smoc_vector_init::ul_vector_init[0][size_y]));		
		connectInterfacePorts(out , input_dup.out1 );	
		
	}
	
};

template <typename T>
class m_test_src: public smoc_actor {

public:
	
	smoc_port_out<T> out;

private:

	void generate_data() {
		out[0] = 0;
		out[1] = 1;
		out[2] = 1;
	}
	smoc_firing_state start;
	smoc_firing_state end;
	
public:
	m_test_src( sc_module_name name )
		: smoc_actor(name,start)
	{
		start = (out(3))
			>> CALL(m_test_src::generate_data) 
			>> end;
	}
	
};



template <typename T = unsigned char>
class m_top
	: public smoc_graph 
{
public:
	m_top( sc_module_name name,
				 const char* inputfile_image,
				 const char* output_filename)
		: smoc_graph(name) {
	
		m_wsdf_img_src<T> &src_image = registerNode(new m_wsdf_img_src<T>("src_image",inputfile_image));
		m_test_src<T> &src_test = registerNode(new m_test_src<T>("test_src"));
		
		m_wsdf_img_snk<T> &sink = registerNode(new m_wsdf_img_snk<T>("sink",output_filename,src_image.size_x,src_image.size_y,3));
		m_recons<T> &recons = registerNode(new m_recons<T>("recons", src_image.size_x,src_image.size_y));
	
#ifndef KASCPAR_PARSING
		indConnectNodePorts( src_image.out, recons.image_in, smoc_wsdf_edge<T>(1));
		connectNodePorts(src_test.out,recons.ctrl_in,smoc_fifo<T>(3));
		indConnectNodePorts( recons.out, sink.in, smoc_wsdf_edge<T>(1));
#endif
	}
};

int sc_main (int argc, char **argv) {
	if(argc < 2) {
		cout << "design-wsdf-recons-dila <input-file><output-file>" << endl;
		return -1;
	}

  smoc_top_moc<m_top<unsigned char> > top("top", argv[1],argv[2]);
  
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
#endif
  return 0;
}
