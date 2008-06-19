// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#define SIZE_X 512
#define SIZE_Y 512

#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3

using namespace std;
using namespace ns_smoc_vector_init;

class m_source: public smoc_actor {
public:
	smoc_md_port_out<int,2> out;
private:
	int i;
  
	void process() {
#ifndef NDEBUG
		cout << name() << " generating " << i << std::endl;
#endif
		out[0][0] = i++;
	}
	
	smoc_firing_state start;
public:
	m_source( sc_module_name name )
		:smoc_actor( name, start ), 
		 out(ul_vector_init[1][1] <<
				 ul_vector_init[SIZE_X][SIZE_Y]),
		 i(0)		 
	{
		start =  out(1) >> (VAR(i) < SIZE_X*SIZE_Y) >> CALL(m_source::process) >> start;
	}
};

class m_sink: public smoc_actor {
public:
	smoc_md_port_in<int,2> in;

private:

	void process() {
#ifndef NDEBUG
		cout << name() << " receiving " << in[0][0] << std::endl;
#endif
		
	}
  
	smoc_firing_state start;
public:
	m_sink( sc_module_name name )
		:smoc_actor( name, start ),
		 in(ul_vector_init[SIZE_X-FILTER_WIDTH+1][SIZE_Y-FILTER_HEIGHT+1], //firing_blocks
				ul_vector_init[SIZE_X-FILTER_WIDTH+1][SIZE_Y-FILTER_HEIGHT+1], //u0
				ul_vector_init[1][1], //c
				ul_vector_init[1][1], //delta_c
				sl_vector_init[0][0], //bs
				sl_vector_init[0][0] //bt
				)
	{
		start = in(1) >> CALL(m_sink::process) >> start;
	}
};

class m_filter: public smoc_actor {
public:
	smoc_md_port_in<int,2> in;
	smoc_md_port_out<int,2> out;
private:
	void process() {
		const int filter_size = FILTER_WIDTH * FILTER_HEIGHT;
#ifndef NDEBUG
		cout << "=======================================" << std::endl;
		cout << name() << std::endl;
		cout << "Window pixels:" << std::endl;
		for(unsigned y = 0; y < FILTER_HEIGHT; y++){
			for(unsigned x = 0; x < FILTER_WIDTH; x++){
				cout << in[x][y] << " ";
			}
			cout << std::endl;
		}
#endif
		int output_value = 0;
		for(unsigned y = 0; y < FILTER_HEIGHT; y++){
			for(unsigned x = 0; x < FILTER_WIDTH; x++){
				output_value += in[x][y];
			}
		}
		output_value /= filter_size;
#ifndef NDEBUG
		cout << "Output value: " << output_value << std::endl;
#endif
		out[0][0] = output_value;		
	}
  
	smoc_firing_state start;
public:
	m_filter( sc_module_name name )
		: smoc_actor( name, start ) ,
			in(ul_vector_init[SIZE_X-FILTER_WIDTH+1][SIZE_Y-FILTER_HEIGHT+1], //firing_blocks
				 ul_vector_init[SIZE_X][SIZE_Y], //u0
				 ul_vector_init[FILTER_WIDTH][FILTER_HEIGHT], //c
				 ul_vector_init[1][1], //delta_c
				 sl_vector_init[0][0], //bs
				 sl_vector_init[0][0] //bt
				 ),
			out(ul_vector_init[1][1] <<
					ul_vector_init[SIZE_X-FILTER_WIDTH+1][SIZE_Y-FILTER_HEIGHT+1])
		
	{
		start = (in(1) && out(1)) >> CALL(m_filter::process) >> start;
	}
};

class m_top2
  : public smoc_graph {
public:
	smoc_md_iport_in<int,2>  in;
	smoc_md_iport_out<int,2> out;
    
	m_top2( sc_module_name name )
		: smoc_graph(name)
	{
		m_filter      &filter = registerNode(new m_filter("filter"));
      
		filter.in(in);
		filter.out(out);
	}
};

class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name )
		: smoc_graph(name) {
		m_source      &src = registerNode(new m_source("src"));
		m_top2        &top2 = registerNode(new m_top2("m_top2"));
		m_sink        &sink = registerNode(new m_sink("sink"));

#ifndef KASCPAR_PARSING
		indConnectNodePorts( src.out, top2.in, smoc_wsdf_edge<int>(FILTER_HEIGHT));
		indConnectNodePorts( top2.out, sink.in,smoc_wsdf_edge<int>(1));
#endif
	}
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start();

  return 0;
}
