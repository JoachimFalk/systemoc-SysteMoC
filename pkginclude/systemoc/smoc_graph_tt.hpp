#ifndef __INCLUDED__SMOC_GRAPH__TT__HPP__
#define __INCLUDED__SMOC_GRAPH__TT__HPP__


#include <systemoc/smoc_graph_type.hpp>
#include <boost/smart_ptr.hpp>

class NodeQueue : public sc_module, public smoc_event{
  SC_HAS_PROCESS(NodeQueue);	

/* struct used to store an event with a certain release-time */
struct TimeNodePair{
  TimeNodePair(sc_time time,  smoc_root_node *node)
    : time(time), node(node) {}
  sc_time time;
  smoc_root_node *node;
};

/* struct used for comparison
 * needed by the priority_queue */
struct nodeCompare{
  bool operator()(const TimeNodePair& tnp1,
                  const TimeNodePair& tnp2) const
  {
    sc_time p1=tnp1.time;
    sc_time p2=tnp2.time;
    if (p1 > p2)
      return true;
    else
      return false;
  }
};

public:
  NodeQueue(sc_module_name name): sc_module(name), smoc_event() {
    SC_THREAD(waiter);
  };

  // register an event with its next releasetime in the EventQueue
  void registerNode(smoc_root_node* node, sc_time time){
    if(time < sc_time_stamp()){
      std::cerr << "Warning: re-activation of a time-triggered Node with a release-time in the past!" << std::endl
		<< "         Maybe the real execution-time was larger then the period or exceeds the deadline?" << std::endl
		<< "         time-triggered activation will be moved to the next periodic point of time in the future" << std::endl;
      smoc_periodic_actor *p_actor = dynamic_cast<smoc_periodic_actor *>( node );
      if(!p_actor){
	std::cerr << "only a smoc_periodic_actor can determine it's next execution-time itself" << std::endl;
	assert(0);
      }
    }
    TimeNodePair tnp(time, node);
    pqueue.push(tnp);	
    //is the new node earlier to release then the current node? or is there currently no node aktiv? -> reactivate the waiter
    if((current!=NULL && time < current->time) || current == NULL ){
      node_added.notify();
    }
  }

  smoc_root_node* getNextNode(){
    TimeNodePair pair = pqueue.top();
    smoc_root_node* top_node = pair.node;
    pqueue.pop();

    if( pqueue.empty() || pqueue.top().time > sc_time_stamp()){
      smoc_reset(*this);
      nodes_processed.notify();
    }

    return top_node;
  }

private:
  //SystemC-process, it tops the queue and waits the specific amount of time
  void waiter(){
   while(true){
    if(!pqueue.empty()){
      current=boost::shared_ptr<TimeNodePair>(new TimeNodePair(pqueue.top()));
      sc_time toWait=current->time-sc_time_stamp();
      // if not, something very strange happend
      assert(toWait >= sc_time(0,SC_NS));
      wait(toWait, node_added);
      //node_added.cancel();
      if(current->time == sc_time_stamp()){
	//NodeQueue is an Event, so let's notify itself. After that, the graph-scheduler knows that some periodic tasks could be executed
	this->notify();
	//wait until all nodes of this step of time are activated
	wait(nodes_processed);
	//nodes_processed.cancel();
      }
    }else{
      //no node registered in the queue, so wait for a new one
      current.reset();
      wait(node_added);
      //node_added.cancel();
    }
   }
  }

  boost::shared_ptr<TimeNodePair> current;
  sc_event node_added;
  sc_event nodes_processed;
  typedef std::priority_queue <TimeNodePair,
                               std::vector<TimeNodePair>,
                               nodeCompare>  TimedQueue;
  TimedQueue pqueue;
};



/**
 * TimeTriggered graph with FSM which schedules children by selecting
 * scheduling is done timetriggered-parameters (offset, period)
 */
class smoc_graph_tt : public smoc_graph_base {
public:
  // construct graph with name
  explicit smoc_graph_tt(const sc_module_name& name);

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
  /// @brief See smoc_graph_base
  void finalise();

private:
  
  // common constructor code
  void constructor();

  void initTT();

  // schedule children of this graph
  void scheduleTT();

  // a list containing the transitions of the graph's children
  // that may be executed
  typedef CoSupport::SystemC::EventOrList<smoc_event>
          smoc_node_ready_list;
  typedef CoSupport::SystemC::EventOrList<smoc_node_ready_list>
          smoc_nodes_ready_list;

  // OrList for the dataflow-driven nodes
  smoc_node_ready_list ddf_nodes_activations;
  // OrList for the activation of the graph
  smoc_event_or_list graph_activation; // nodes scheduleable?

  // handling of the time-triggered nodes
  NodeQueue ttNodeQueue;

  // graph scheduler FSM state
  smoc_firing_state run;

  std::map<std::string, smoc_root_node*> nameToNode;
  std::map<smoc_root_node*, bool> nodeDisabled;
};

#endif //__INCLUDED__SMOC_GRAPH__TT__HPP__
