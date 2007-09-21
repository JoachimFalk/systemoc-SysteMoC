// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "wsdf_img_src.hpp"
#include "wsdf_img_snk.hpp"
#include "wsdf_recons_fifo.hpp"


using namespace std;

template <typename T = unsigned char>
class m_recons 
	: public smoc_graph {

public:
	smoc_md_iport_in<T,2>  seed_in;
	smoc_md_iport_in<T,2>  image_in;

	smoc_md_iport_out<T,2, smoc_storage_inout> out;
    
	m_recons( sc_module_name name,
						unsigned size_x,
						unsigned size_y)
		: smoc_graph(name)
	{
		m_wsdf_recons_fifo<T> &recons = registerNode(new m_wsdf_recons_fifo<T>("recons_fifo", size_x, size_y));


		connectInterfacePorts( seed_in, recons.seed_in );
		connectInterfacePorts( image_in, recons.mask_in );

		connectNodePorts(recons.coord_out, recons.coord_in, smoc_fifo<unsigned int>(10*size_x*size_y));

		connectInterfacePorts(out,  recons.out);
		
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
		indConnectNodePorts( src_image.out, recons.image_in, smoc_wsdf_edge<T>(src_image.size_x*src_image.size_y));
		indConnectNodePorts( src_seed.out, recons.seed_in, smoc_wsdf_edge<T>(3));
		indConnectNodePorts( recons.out, sink.in, smoc_wsdf_edge<T, smoc_storage_inout>(src_image.size_x*src_image.size_y));
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
