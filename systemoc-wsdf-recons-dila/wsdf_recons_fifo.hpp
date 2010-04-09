// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_RECONS_FIFO_HPP
#define INCLUDE_WSDF_RECONS_FIFO_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>

//#define VERBOSE_WSDF_RECONS_FIFO

template <typename T>
class m_wsdf_recons_fifo: public smoc_actor {

public:

	smoc_md_port_in<T,2> mask_in;
	smoc_md_port_in<T,2> seed_in;
	
	smoc_md_port_out<T,2, smoc_storage_inout> out;

	smoc_port_out<unsigned int> coord_out;
	smoc_port_in<unsigned int> coord_in;
	
	

private:

	smoc_firing_state read_seed;
	smoc_firing_state do_recons;
	smoc_firing_state stuck;

	const unsigned size_x;
	const unsigned size_y;
	

	long cur_x;
	unsigned int window_pixel_id_x;
	long cur_y;
	unsigned int window_pixel_id_y;



	//checks, whether current seed pixel shall be add to FIFO
	bool init_guard() const {
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter init_guard" << std::endl;
		std::cout << " seed_in[1][1] = " << (unsigned int)seed_in[1][1] << std::endl;
#endif
		if (seed_in[1][1] == 0){
			return false;
		}

		unsigned cur_x = seed_in.iteration(0,0);
		unsigned cur_y = seed_in.iteration(0,1);
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << " cur_x = " << cur_x << std::endl;
		std::cout << " cur_y = " << cur_y << std::endl;
#endif

#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "mask_in[cur_x][cur_y] = " << (unsigned int)mask_in[cur_x][cur_y] << std::endl;
#endif
		if (mask_in[cur_x][cur_y] == 0)
			return false;		
		
		for(unsigned int i = 0; i < 3; i++){
			for(unsigned int j = 0; j < 3; j++){
				if ((i == 1) && (j == 1))
					continue;
				if (seed_in[i][j] == 0)
					return true;
			}
		}

		return false;
	}

	//adds input pixel to FIFO
	void input_add2fifo(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter input_add2fifo" << std::endl;
#endif
		coord_out[0] = seed_in.iteration(0,0);
		coord_out[1] = seed_in.iteration(0,1);

		out[seed_in.iteration(0,0)][seed_in.iteration(0,1)] = seed_in[1][1];
	}

	//called, if error occurs
	void failed(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "failed called!" << std::endl;
#endif
	}

	//only propagate input image
	void input_ignore(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter input_ignore" << std::endl;		
#endif
		unsigned int x = seed_in.iteration(0,0);
		unsigned int y = seed_in.iteration(0,1);
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << " x = " << x << std::endl;
		std::cout << " y = " << y << std::endl;
#endif

#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << " read mask" << std::endl;
#endif
		T mask_pixel = mask_in[x][y];

		if(mask_pixel != 0){
#ifdef VERBOSE_WSDF_RECONS_FIFO
			std::cout << " read seed" << std::endl;
#endif
			T output_value = seed_in[1][1];
#ifdef VERBOSE_WSDF_RECONS_FIFO
			std::cout << " set output-value" << std::endl;
#endif
			out[x][y] = output_value;
		}else{
#ifdef VERBOSE_WSDF_RECONS_FIFO
			std::cout << " set output-value" << std::endl;
#endif
			out[x][y] = 0;
		}

#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << " window_pixel_id_x = " << window_pixel_id_x << std::endl;
		std::cout << " window_pixel_id_y = " << window_pixel_id_y << std::endl;

		std::cout << "Leave input_ignore" << std::endl;
#endif
	}	

	// checks, whether current pixel shall be add to FIFO
	bool propagate_first_guard() const {
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter propagate_first_guard" << std::endl;
#endif
		long cur_x = coord_in[0];
		long cur_y = coord_in[1];

		cur_x--;
		cur_y--;

		if (cur_x < 0)
			//out of image
			return false;
		
		if (cur_y < 0)
			//out of image
			return false;

		if (out[cur_x][cur_y] != 0)
			return false;

		if (mask_in[cur_x][cur_y] == 0)
			return false;

		return true;
		
	}

	//adds the first pixel of the neighborhood to FIFO
	void propagate_first() {
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter propagate_first" << std::endl;		
#endif
		cur_x = coord_in[0];
		cur_y = coord_in[1];
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << " cur_x = " << cur_x << std::endl;
		std::cout << " cur_y = " << cur_y << std::endl;		
#endif

		cur_x--;
		cur_y--;

		assert(cur_x >= 0);
		assert(cur_y >= 0);

		out[cur_x][cur_y] = ~((T)0);
		coord_out[0] = cur_x;
		coord_out[1] = cur_y;

		cur_x++;
		window_pixel_id_x = 1;
		window_pixel_id_y = 0;
		
	}

	//ignores the first pixel of the neighborhood
	void move2second(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter move2second" << std::endl;
#endif
		cur_x = coord_in[0];
		cur_y = coord_in[1];	

		cur_y--;
		window_pixel_id_x = 1;
		window_pixel_id_y = 0;
	}

	
	//checks, whether current pixel of neighborhood shall be added to FIFO
	bool prop_guard() const {
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter prop_guard" << std::endl;
#endif
		if ((cur_x < 0) || ((unsigned)cur_x >= size_x))
			//out of image
			return false;

		if ((cur_y < 0) || ((unsigned)cur_y >= size_y))
			//out of image
			return false;

		if (out[cur_x][cur_y] != 0)
			return false;

		if (mask_in[cur_x][cur_y] == 0)
			return false;

		return true;

	}

	//moves to next pixel in neighborhood
	void move2next(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter move2next" << std::endl;
#endif
		if (window_pixel_id_x >= 2){
			cur_x -= window_pixel_id_x;
			window_pixel_id_x = 0;

			
			if (window_pixel_id_y >= 2){
				cur_y -= window_pixel_id_y;
				window_pixel_id_y = 0;
			}else{
				cur_y++;
				window_pixel_id_y++;
			}


		}else{
			window_pixel_id_x++;
			cur_x++;
		}
	}

		
	//adds current pixel of neighborhood to FIFO and moves to next pixel
	void propagate(){
#ifdef VERBOSE_WSDF_RECONS_FIFO
		std::cout << "Enter propagate" << std::endl;
#endif
		assert(cur_x >= 0);
		assert(cur_y >= 0);

		assert((unsigned)cur_x < size_x);
		assert((unsigned)cur_y < size_y);

		out[cur_x][cur_y] = ~((T)0);
		coord_out[0] = cur_x;
		coord_out[1] = cur_y;

		move2next();

	}

	//dummy function
	void terminate(){
		std::cout << "Enter terminate" << std::endl;
	}

	


public:
	m_wsdf_recons_fifo( sc_module_name name ,
											unsigned size_x,
											unsigned size_y)
		: smoc_actor(name,read_seed),
			mask_in(ns_smoc_vector_init::ul_vector_init[1][1], //firing_blocks
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //c
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //delta_c
							ns_smoc_vector_init::sl_vector_init[0][0], //bs
							ns_smoc_vector_init::sl_vector_init[0][0] //bt
							),
			seed_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
							ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
							ns_smoc_vector_init::ul_vector_init[3][3], //c
							ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
							ns_smoc_vector_init::sl_vector_init[1][1], //bs
							ns_smoc_vector_init::sl_vector_init[1][1], //bt
							typename smoc_md_port_in<T,2>::border_init(~(T(0)))
							),
			out(ns_smoc_vector_init::ul_vector_init[size_x][size_y] <<
					ns_smoc_vector_init::ul_vector_init[size_x][size_y]
					),
			size_x(size_x), size_y(size_y),
			cur_x(0), 
			window_pixel_id_x(0), 
			cur_y(0),
			window_pixel_id_y(0)
	{
		read_seed = 
			//add pixel to FIFO
			(seed_in(1) && mask_in(0,1) && out(0,1) && coord_out(2))
			>> (
					(
					 (seed_in.getIteration(0,0) != (size_t)(size_x-1)) || 
					 (seed_in.getIteration(0,1) != (size_t)(size_y-1))
					 ) && 
					GUARD(m_wsdf_recons_fifo::init_guard)
					)
			>> CALL(m_wsdf_recons_fifo::input_add2fifo) 
			>> read_seed

			//failed to add pixel to FIFO
			|(seed_in(1) && mask_in(0,1) && out(0,1))
			>> (
					(
					 (seed_in.getIteration(0,0) != (size_t)(size_x-1)) || 
					 (seed_in.getIteration(0,1) != (size_t)(size_y-1))
					 ) && 
					GUARD(m_wsdf_recons_fifo::init_guard)
					)
			>> CALL(m_wsdf_recons_fifo::failed) 
			>> stuck

			//do not add pixel to FIFO
			| (seed_in(1) && mask_in(0,1) && out(0,1))
			>> (
					(
					 (seed_in.getIteration(0,0) != (size_t)(size_x-1)) || 
					 (seed_in.getIteration(0,1) != (size_t)(size_y-1))
					 ) && 
					(!GUARD(m_wsdf_recons_fifo::init_guard))
					)
			>> CALL(m_wsdf_recons_fifo::input_ignore) 
			>> read_seed

			//add last pixel to FIFO
			| (seed_in(1) && mask_in(0,1) && out(0,1) && coord_out(2))
			>> (
					(
					 (seed_in.getIteration(0,0) == (size_t)(size_x-1)) &&
					 (seed_in.getIteration(0,1) == (size_t)(size_y-1))
					 ) && 
					GUARD(m_wsdf_recons_fifo::init_guard)
					)
			>> CALL(m_wsdf_recons_fifo::input_add2fifo) 
			>> do_recons

			//failed to add last pixel to FIFO
			| (seed_in(1) && mask_in(0,1) && out(0,1))
			>> (
					(
					 (seed_in.getIteration(0,0) == (size_t)(size_x-1)) &&
					 (seed_in.getIteration(0,1) == (size_t)(size_y-1))
					 ) && 
					GUARD(m_wsdf_recons_fifo::init_guard)
					)
			>> CALL(m_wsdf_recons_fifo::failed) 
			>> stuck

			//do not add last pixel to FIFO
			| (seed_in(1) && mask_in(0,1) && out(0,1))
			>> (
					(
					 (seed_in.getIteration(0,0) == (size_t)(size_x-1)) && 
					 (seed_in.getIteration(0,1) == (size_t)(size_y-1))
					 ) && 
					(!GUARD(m_wsdf_recons_fifo::init_guard))
					)
			>> CALL(m_wsdf_recons_fifo::input_ignore) 
			>> do_recons;



		do_recons = 
			//process new pixel and add first neighbor to FIFO
			(coord_in(2) && out(0,1) && mask_in(0,1) && coord_out(2))
			>> (
					(VAR(window_pixel_id_x) == (unsigned int)0) &&
					(VAR(window_pixel_id_y) == (unsigned int)0) &&
					GUARD(m_wsdf_recons_fifo::propagate_first_guard)
					)
			>> CALL(m_wsdf_recons_fifo::propagate_first)
			>> do_recons

			//failed to process new pixel and to add first neighbor to FIFO
			//ATTENTION to evaluation order of state-machine!!
			| (coord_in(2) && out(0,1) && mask_in(0,1))
			>> (
					(var(window_pixel_id_x) == (unsigned int)0) &&
					(var(window_pixel_id_y) == (unsigned int)0) &&
					GUARD(m_wsdf_recons_fifo::propagate_first_guard)
					)
			>> CALL(m_wsdf_recons_fifo::failed)
			>> stuck
			
			//process new pixel and don't add first neighbor to FIFO
			| (coord_in(2) && out(0,1) && mask_in(0,1))
			>> (
					(var(window_pixel_id_x) == (unsigned int)0) &&
					(var(window_pixel_id_y) == (unsigned int)0) &&
					(!GUARD(m_wsdf_recons_fifo::propagate_first_guard))
					)
			>> CALL(m_wsdf_recons_fifo::move2second)
			>> do_recons

			//process next neighbor and add to FIFO
			| (out(0,1) && mask_in(0,1) && coord_out(2))
			>> (
					(
					 (var(window_pixel_id_x) != (unsigned int)0) ||
					 (var(window_pixel_id_y) != (unsigned int)0)
					 ) &&
					GUARD(m_wsdf_recons_fifo::prop_guard)
					)
			>> CALL(m_wsdf_recons_fifo::propagate)
			>> do_recons

			//failed to process next neighbor and to add to FIFO
			//ATTENTION to evaluation of statemachine
			| (out(0,1) && mask_in(0,1))
			>> (
					(
					 (var(window_pixel_id_x) != (unsigned int)0) ||
					 (var(window_pixel_id_y) != (unsigned int)0)
					 ) && 
					GUARD(m_wsdf_recons_fifo::prop_guard)
					)
			>> CALL(m_wsdf_recons_fifo::failed)
			>> stuck
			
			//process next neighbor and don't add to FIFO
			| (out(0,1) && mask_in(0,1))
			>> (
					(
					 (var(window_pixel_id_x) != (unsigned int)0) ||
					 (var(window_pixel_id_y) != (unsigned int)0)
					 ) &&
					(!GUARD(m_wsdf_recons_fifo::prop_guard))
					)
			>> CALL(m_wsdf_recons_fifo::move2next)
			>> do_recons

			//terminate operation
			//ATTENTION: If pixels take some time to arrive, catastrophy occurs
			//           (HW-implementation!!!)
			| (out(1) && mask_in(1))
			>> (
					(var(window_pixel_id_x) == (unsigned int)0) &&
					(var(window_pixel_id_y) == (unsigned int)0)
					)
			>> CALL(m_wsdf_recons_fifo::terminate)
			>> read_seed;	


	}
	
};


#endif //INCLUDE_WSDF_DIL_OUT_SWITCH_HPP
