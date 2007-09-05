
#include "snk2addr_table_win_iter.hpp"


snk2addr_table_win_iter::snk2addr_table_win_iter(const smoc_src_md_static_loop_iterator& src_iter,
                                                 const smoc_snk_md_static_loop_iterator& snk_iter,
                                                 const smoc_md_array<struct src_addr_info_struct>& ref_point_addr_offset_table)
  : snk2addr_table(src_iter, snk_iter),
    win_iter_addr_offset_table(snk_iter.iterator_depth(),
                               snk_iter.iteration_size()),
    ref_point_addr_offset_table(ref_point_addr_offset_table)
  
{
  build_win_iter_addr_offset_table();

}



void 
snk2addr_table_win_iter::build_win_iter_addr_offset_table(){

  CoSupport::dout << "Enter snk2addr_table_win_iter::build_win_iter_addr_offset_table" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;

  do {
    smoc_snk_md_static_loop_iterator::iter_domain_vector_type
      window_iterator(snk_iter.token_dimensions(),
                      (smoc_snk_md_static_loop_iterator::iter_item_type)0);
    
    // Iteration over all window elements
    bool end_of_window = false;
    while(!end_of_window){

      // Set window iterator, such that we can use the
      // iteration vector as array index
      snk_iter.set_window_iterator(window_iterator);

      CoSupport::dout << "Iteration: " 
                      << snk_iter.iteration_vector() 
                      << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;


      bool addr_valid;
      long lin_addr = 
        calc_lin_addr(window_iterator,
                      addr_valid);

      //Note, that here the structure names are not very nice.
      //Perhaps it would be better to introduce a new structure.
      //Store absolute address
      win_iter_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr =
        lin_addr;
      //Calculate relative address to reference pixel
      win_iter_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr =
        lin_addr - 
        ref_point_addr_offset_table[snk_iter.iteration_vector()].curr_abs_addr;
      
      win_iter_addr_offset_table[snk_iter.iteration_vector()].curr_addr_valid =
        win_iter_addr_offset_table[snk_iter.iteration_vector()].next_addr_valid =
        addr_valid;

      CoSupport::dout << "Absolute address: " << lin_addr << std::endl;
      CoSupport::dout << "Relative address: " 
                      << win_iter_addr_offset_table[snk_iter.iteration_vector()].rel_next_addr 
                      << std::endl;
      CoSupport::dout << (addr_valid ? "address valid" : "address not valid")
                      << std::endl;

      CoSupport::dout << "Move to next window ..." << std::endl;

      end_of_window = true;
      //Move to next window position
      for(int i = snk_iter.token_dimensions()-1;
          i >= 0;
          i--){
        window_iterator[i]++;
        if (window_iterator[i] > snk_iter.max_window_iteration()[i]){
          window_iterator[i] = 0;
        }else{
          end_of_window = false;
          break;
        }
      }

      CoSupport::dout << CoSupport::Indent::Down;

    } //iteration of window

    
  }while(!snk_iter.inc());

  CoSupport::dout << "Leave snk2addr_table_win_iter::build_win_iter_addr_offset_table" 
                  << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;

  
}

