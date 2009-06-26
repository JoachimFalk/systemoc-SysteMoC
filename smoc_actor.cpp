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

#ifdef SYSTEMOC_ENABLE_SGX
  
void smoc_actor::finalise() {
  assembleXML();
  smoc_root_node::finalise();
  // FIXME: FSM is attribute of Actor, not of Process
  ac->firingFSM() = getFiringFSM()->getNGXObj();
}

void smoc_actor::assembleXML() {
  using namespace SystemCoDesigner::SGX;
  using namespace SysteMoC::ActivationPattern;

  assert(!ac);
  
  Actor _actor(name());
  ac = &_actor;
  proc = ac;
  
  // set some attributes
  ac->cxxClass() = typeid(*this).name();
 
  for(ParamInfoList::const_iterator pIter = constrArgs.pil.begin();
      pIter != constrArgs.pil.end(); ++pIter)
  {
    Parameter p(pIter->type, pIter->value);
    p.name().set(pIter->name);
    ac->constructorParameters().push_back(p);
  }

  smoc_graph_base* parent =
    dynamic_cast<smoc_graph_base*>(get_parent_object());
  
  if(parent)
    parent->addProcess(_actor);
  else
    assert(!"Actor has no parent!");
}
#endif
