#include <systemoc/smoc_tt.hpp>


smoc_graph_tt::smoc_graph_tt(const sc_module_name& name) :
  smoc_graph_base(name, run),
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  ttNodeQueue("__smoc_ttEventQueue"),
  run("run")
{
  constructor();
}

smoc_graph_tt::smoc_graph_tt() :
  smoc_graph_base(sc_gen_unique_name("smoc_graph_tt"), run),
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  ttNodeQueue("__smoc_ttNodeQueue"),
  run("run")
{
  constructor();
}
  
void smoc_graph_tt::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph::finalise name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  
  smoc_graph_base::finalise();
  initTT();

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_graph_tt::constructor() {
  // if there is at least one active transition: execute it
  graph_activation |= ddf_nodes_activations;
  graph_activation |= ttNodeQueue;
  run = smoc::Expr::till(graph_activation) >> CALL(smoc_graph_tt::scheduleTT) >> run;
}

void smoc_graph_tt::initTT() {
  for(smoc_node_list::const_iterator iter = getNodes().begin();
      iter != getNodes().end(); ++iter)
  {
    nameToNode[(*iter)->name()] = *iter;
    smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( (*iter) );
    if(entry){
      ttNodeQueue.registerNode(entry, entry->getOffset());
    }else{
      //nodes of other types then smoc_periodic_actor will be added to ddf_nodes_activations
      //could be another graph or other nodes
      ddf_nodes_activations |= **iter;
    }
  }
}

void smoc_graph_tt::scheduleTT() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph_tt::scheduleTT name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  while(ddf_nodes_activations){
    //schedule the "normal" Tasks (DDF)
    smoc_root_node& n = dynamic_cast<smoc_root_node&>( ddf_nodes_activations.getEventTrigger());
#ifdef SYSTEMOC_DEBUG
    outDbg << "<node name=\"" << n.name() << "\">" << std::endl
           << Indent::Up;
#endif // SYSTEMOC_DEBUG
    n.schedule();
    smoc_periodic_actor *p_node = dynamic_cast<smoc_periodic_actor *>( &n);
    if(p_node){ // it is a TT-Node
      //remove it from ddf_nodes_activations and re-register it as a tt-node
      ddf_nodes_activations.remove(n);
      ttNodeQueue.registerNode(p_node, p_node->getNextReleaseTime());
    }
#ifdef SYSTEMOC_DEBUG
    outDbg << Indent::Down << "</node>" << std::endl;
#endif // SYSTEMOC_DEBUG
  }
  while(ttNodeQueue){ // TT-Scheduled
    smoc_root_node* next = ttNodeQueue.getNextNode();
    smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( next);
    assert(entry);
#ifdef SYSTEMOC_DEBUG
    outDbg << "<node name=\"" << next->name() << "\">" << std::endl
           << Indent::Up;
#endif // SYSTEMOC_DEBUG
    if(nodeDisabled[entry] == false){
      entry->schedule();
      if(entry->inCommState()){ // Node needs some time to process (VPC is used), switch node to DDF
        ddf_nodes_activations |= *next;
      }else{ // Node completely processed -> re-register it in the ttNodeQueue
        ttNodeQueue.registerNode(entry, entry->getNextReleaseTime());
      }
    }
#ifdef SYSTEMOC_DEBUG
    outDbg << Indent::Down << "</node>" << std::endl;
#endif // SYSTEMOC_DEBUG
  }
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph_tt::scheduleTT>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_graph_tt::disableActor(std::string actor_name){
  std::cout<<"smoc_graph_tt::disable_actor " << actor_name << " has id" << nameToNode[actor_name] << std::endl;
  assert(nameToNode[actor_name] != 0);
  smoc_root_node* nodeToDisable = nameToNode[actor_name];
  if(ddf_nodes_activations.contains(*nodeToDisable)){
    ddf_nodes_activations.remove(*nodeToDisable);
  }else{
    //so it must be a tt-actor
    //ttNodeQueue.disableNode(nodeToDisable);
    nodeDisabled[nodeToDisable] = true;
  }

}


void smoc_graph_tt::reEnableActor(std::string actor_name){
  assert(nameToNode[actor_name] != 0);
  smoc_root_node* nodeToEnable = nameToNode[actor_name];
  smoc_periodic_actor *entry = dynamic_cast<smoc_periodic_actor *>( nodeToEnable );
      if(entry){
        nodeDisabled[entry] = false;
        ttNodeQueue.registerNode(entry, entry->getNextReleaseTime());
      }else{
        //nodes of other types then smoc_periodic_actor will be added to ddf_nodes_activations
        //could be another graph or other nodes
        ddf_nodes_activations |= *nodeToEnable;
      }
}

