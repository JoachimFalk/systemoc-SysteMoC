#ifndef __INCLUDED__SMOC_GRAPH__TT__HPP__
#define __INCLUDED__SMOC_GRAPH__TT__HPP__


#include <CoSupport/compatibility-glue/nullptr.h>

#include <boost/smart_ptr.hpp>

#include "../smoc/detail/GraphBase.hpp"
#include "../smoc/detail/NodeQueue.hpp"

/**
 * TimeTriggered graph with FSM which schedules children by selecting
 * scheduling is done timetriggered-parameters (offset, period)
 */
class smoc_graph_tt : public smoc::Detail::GraphBase {
public:
  // construct graph with name
  explicit smoc_graph_tt(const sc_core::sc_module_name& name);

  // construct graph with generated name
  smoc_graph_tt();
  
  /**
   * disables the executability of an actor
   */
  void disableActor(std::string actor_name);

  /**
   * reenables the executability of an actor
   */
  void reEnableActor(std::string actor_name);

protected:
  /// @brief See GraphBase
  virtual void before_end_of_elaboration();

private:
  
  // common constructor code
  void constructor();

  void initTT();

  // schedule children of this graph
  void scheduleTT();

  // a list containing the transitions of the graph's children
  // that may be executed
  typedef CoSupport::SystemC::EventOrList<smoc::smoc_event>
          smoc_node_ready_list;
  typedef CoSupport::SystemC::EventOrList<smoc_node_ready_list>
          smoc_nodes_ready_list;

  // OrList for the dataflow-driven nodes
  smoc_node_ready_list ddf_nodes_activations;
  // OrList for the activation of the graph
  smoc::smoc_event_or_list graph_activation; // nodes scheduleable?

  // handling of the time-triggered nodes
  smoc::Detail::NodeQueue ttNodeQueue;

  // graph scheduler FSM state
  smoc_firing_state run;

  std::map<std::string, smoc_root_node*> nameToNode;
  std::map<smoc_root_node*, bool> nodeDisabled;
};

#endif //__INCLUDED__SMOC_GRAPH__TT__HPP__
