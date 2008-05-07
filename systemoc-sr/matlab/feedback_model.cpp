
      
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
      DiscreteTimeIntegratorOpaqueSig0.connect(DiscreteTimeIntegratorOpaqueActor.o0)
          .connect(ScopeActor.i0)
          .connect(SineWaveOpaqueActor.i0);
    

              
      smoc_multicast_sr_signal<real_T> SineWaveOpaqueSig0;
      SineWaveOpaqueSig0.connect(SineWaveOpaqueActor.o0)
          .connect(AddOpaqueActor.i0);
    

              
      smoc_multicast_sr_signal<real_T> SineWaveOpaqueSig1;
      SineWaveOpaqueSig1.connect(SineWaveOpaqueActor.o1)
          .connect(AddOpaqueActor.i1);
    

              
      smoc_multicast_sr_signal<real_T> AddOpaqueSig0;
      AddOpaqueSig0.connect(AddOpaqueActor.o0)
          .connect(UnitDelayOpaqueActor.i0);
    

              
      smoc_multicast_sr_signal<real_T> DiscreteTransferFcnSig0;
      DiscreteTransferFcnSig0.connect(DiscreteTransferFcnActor.o0)
          .connect(DiscreteTimeIntegratorOpaqueActor.i0);
    

              
      smoc_multicast_sr_signal<real_T> UnitDelayOpaqueSig0;
      UnitDelayOpaqueSig0.connect(UnitDelayOpaqueActor.o0)
          .connect(DiscreteTransferFcnActor.i0);
    

      
    }
  };
  
  int sc_main (int argc, char **argv) {
    smoc_top_moc<feedback_model> feedback_model("feedback_model");
    sc_start(-1);
    return 0;
  }
  

  
