#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph_type.hpp>

using namespace SystemCoDesigner::SGX;

smoc_actor::smoc_actor(sc_module_name name, smoc_hierarchical_state &s)
  : smoc_root_node(name, s)
#ifndef __SCFE__
    , ac(this->name())
#endif
{
#ifndef __SCFE__
  init();
#endif
}
  
smoc_actor::smoc_actor(smoc_firing_state &s)
  : smoc_root_node(sc_gen_unique_name("smoc_actor"), s)
#ifndef __SCFE__
    , ac(this->name())
#endif
{
#ifndef __SCFE__
  init();
#endif
}

#ifdef SYSTEMOC_DEBUG
smoc_actor::~smoc_actor() {
  std::cerr << "~smoc_actor() name = \"" << name() << "\"" << std::endl;
}
#endif

#ifndef __SCFE__
  
void smoc_actor::assemble( smoc_modes::PGWriter &pgw ) const {
  return smoc_root_node::assemble(pgw);
}
  
void smoc_actor::finalise() {

  smoc_root_node::finalise();

  // set some attributes
  ac.cxxClass() = typeid(*this).name();
}

void smoc_actor::init() {

  proc = &ac;

  smoc_graph_base* parent =
    dynamic_cast<smoc_graph_base*>(get_parent_object());
  
  if(parent)
    parent->addProcess(ac);
  else
    assert(!"Actor has no parent!");
}
#endif
