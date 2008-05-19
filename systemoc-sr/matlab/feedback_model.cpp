
      
  /*This will be the main file for the feedback_model Model*/
  #include <cstdlib>
  #include <iostream>
  
  #include <systemoc/smoc_moc.hpp>
  #include <systemoc/smoc_multicast_sr_signal.hpp>
  
  
  #include <feedback_model_types.h>
  
  
  #include <DiscreteTimeIntegratorOpaque.h>
  #include <Scope.h>
  #include <SineWaveOpaque.h>
  #include <AddOpaque.h>
  #include <DiscreteTransferFcn.h>
  #include <UnitDelayOpaque.h>
  using namespace std;
  
  class feedback_model: public smoc_graph_sr {
  protected:
  
  DiscreteTimeIntegratorOpaque DiscreteTimeIntegratorOpaqueActor;
  Scope ScopeActor;
  SineWaveOpaque SineWaveOpaqueActor;
  AddOpaque AddOpaqueActor;
  DiscreteTransferFcn DiscreteTransferFcnActor;
  UnitDelayOpaque UnitDelayOpaqueActor;
    
  public:
    feedback_model( sc_module_name name )
      : smoc_graph_sr(name),
          DiscreteTimeIntegratorOpaqueActor("DiscreteTimeIntegratorOpaque"),
          ScopeActor("Scope"),
          SineWaveOpaqueActor("SineWaveOpaque"),
          AddOpaqueActor("AddOpaque"),
          DiscreteTransferFcnActor("DiscreteTransferFcn"),
          UnitDelayOpaqueActor("UnitDelayOpaque")
    {
    
              
      smoc_multicast_sr_signal<real_T> DiscreteTimeIntegratorOpaqueSig0;
      connector(DiscreteTimeIntegratorOpaqueSig0)
        << DiscreteTimeIntegratorOpaqueActor.o0
        << ScopeActor.i0
        << SineWaveOpaqueActor.i0;
    

              
      smoc_multicast_sr_signal<real_T> SineWaveOpaqueSig0;
      connector(SineWaveOpaqueSig0)
        << SineWaveOpaqueActor.o0
        << AddOpaqueActor.i0;
    

              
      smoc_multicast_sr_signal<real_T> SineWaveOpaqueSig1;
      connector(SineWaveOpaqueSig1)
        << SineWaveOpaqueActor.o1
        << AddOpaqueActor.i1;
    

              
      smoc_multicast_sr_signal<real_T> AddOpaqueSig0;
      connector(AddOpaqueSig0)
        << AddOpaqueActor.o0
        << UnitDelayOpaqueActor.i0;
    

              
      smoc_multicast_sr_signal<real_T> DiscreteTransferFcnSig0;
      connector(DiscreteTransferFcnSig0)
        << DiscreteTransferFcnActor.o0
        << DiscreteTimeIntegratorOpaqueActor.i0;
    

              
      smoc_multicast_sr_signal<real_T> UnitDelayOpaqueSig0;
      connector(UnitDelayOpaqueSig0)
        << UnitDelayOpaqueActor.o0
        << DiscreteTransferFcnActor.i0;
    

      
    }
  };
  
  int sc_main (int argc, char **argv) {
    smoc_top_moc<feedback_model> feedback_model("feedback_model");
    sc_start(-1);
    return 0;
  }
  

  
