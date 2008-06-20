//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_SCHEDULE_HPP
#define _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_SCHEDULE_HPP

#include <fstream>

#include "smoc_md_ba_linearized_buffer.hpp"
#include <wsdf/smoc_md_array.hpp>

namespace smoc_md_ba {

  /// This class can be used to calculate a self timed schedule
  class smoc_mb_ba_lin_buffer_schedule 
  : public smoc_mb_ba_lin_buffer {
  public:
    typedef smoc_mb_ba_lin_buffer parent_type;
  public:
    smoc_mb_ba_lin_buffer_schedule(
      const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
      const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator,
      unsigned int buffer_height = 1);

    virtual ~smoc_mb_ba_lin_buffer_schedule();

  public:
    // Get invocation table (result)
    const smoc_md_array<unsigned long>& get_snk2src_invocation_table() const {
      return snk2src_invocation_table;
    }
    const smoc_md_array<unsigned long>& get_src2snk_invocation_table() const {
      return src2snk_invocation_table;
    }


    void dump_results(std::ostream& os) const{};

  protected:
    /// Implementation of interface, inhereted from smoc_md_buffer_analysis
    void consumption_update(
      const iter_domain_vector_type& current_iteration,
      bool new_schedule_period,
      const iter_domain_vector_type& consumed_window_start,
      const iter_domain_vector_type& consumed_window_end);

    void consumption_update(
      const iter_domain_vector_type& current_iteration,
      bool new_schedule_period);

    void production_update(
      const iter_domain_vector_type& current_iteration,
      const iter_domain_vector_type& max_window_iteration,
      bool new_schedule_period);
  private:
    unsigned long *src_order_vector;

    void init_src_order_vector();

    unsigned long 
    calc_num_src_invocations(
      const iter_domain_vector_type& previous_src_iter,
      const iter_domain_vector_type& current_src_iter
      ) const;
  private:
    /// Specifies for each snk iteration, how many source iterations
    /// can be additionally scheduled after terminating sink execution
    smoc_md_array<unsigned long> &snk2src_invocation_table;

    /// Specifies for each source invocation, how many sink iterations
    /// can be additionally scheduled after terminating the source
    /// execution
    smoc_md_array<unsigned long> &src2snk_invocation_table;

    iter_domain_vector_type current_src_iteration;
  };

  /// User interface
  class smoc_md_ba_ui_schedule
  : public smoc_md_ba_user_interface {
  public:
    smoc_md_ba_ui_schedule() {}

    virtual ~smoc_md_ba_ui_schedule() {}
  public:
    /// This function sets up the corresponding buffer analysis class
    /// and returns a corresponding pointer
    virtual smoc_md_buffer_analysis*
    create_buffer_analysis_object(
      const smoc_src_md_loop_iterator_kind& src_md_loop_iterator,
      const smoc_snk_md_loop_iterator_kind& snk_md_loop_iterator);
  public:
    const smoc_md_array<unsigned long>& get_snk2src_invocation_table() const {
      assert(ba_obj != NULL);
      return ba_obj->get_snk2src_invocation_table();
    }

    const smoc_md_array<unsigned long>& get_src2snk_invocation_table() const {
      assert(ba_obj != NULL);
      return ba_obj->get_src2snk_invocation_table();
    }
  private:
    /// associated buffer analysis object
    const smoc_mb_ba_lin_buffer_schedule* ba_obj;
  };

};

#endif // _INCLUDED_SMOC_MD_BA_LINEARIZED_BUFFER_SCHEDULE_HPP
