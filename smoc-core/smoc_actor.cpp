#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph_type.hpp>

smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s)
  : smoc_root_node(name, s)
{}

smoc_actor::smoc_actor(smoc_firing_state &s)
  : smoc_root_node(sc_gen_unique_name("smoc_actor"), s)
{}

#ifdef SYSTEMOC_DEBUG
smoc_actor::~smoc_actor() {
  std::cerr << "~smoc_actor() name = \"" << name() << "\"" << std::endl;
}
#endif
