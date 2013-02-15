#ifndef __INCLUDED__ACTORDISABLE_hpp__
#define __INCLUDED__ACTORDISABLE_hpp__

#include <systemc.h>
#include <map>
#include <vector>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>
#include <CoSupport/SystemC/ChannelModificationListener.hpp>
#include <systemoc/smoc_tt.hpp>

using namespace CoSupport::SystemC;

enum component_kind {COMMUNICATION
          ,EXECUTION, COMPONENT, ENABLE, DISABLE};

struct list_element{

  list_element(component_kind kind_of,
               std::string resname,
               sc_time when,
               unsigned int newstat)
    : kind_of(kind_of),
      resname(resname),
      when(when),
      newstat(newstat) { }

  component_kind kind_of; //which kind of failure has to be simulated
  std::string resname;    //Name of the component
  //ChannelId id;     //ID of the channel which has to change its status
  sc_time when;     //time the change has to occur
  unsigned int newstat; // 0 : kein Fehler , >0 : Fehler!
};

struct channel_element{
  unsigned int status;    // 0: kein Fehler
  std::string releasedBy; //
};

class Tester : public sc_module{

public:
  void tester(){}

  SC_HAS_PROCESS(Tester);

  Tester(sc_module_name name): sc_module(name) {
    SC_THREAD(tester);
  };
};

class ActorDisabler: public sc_module {
    std::map <std::string , ChannelId> IDmap;  // Zuordnung Channel-Name - ID
    std::vector<channel_element> ChannelStatus;
    int temp;
    std::vector<list_element> liste;
    bool scriptfile;
    smoc_graph_tt* graph;

public:
  //Constructor
    ActorDisabler(smoc_graph_tt* smoc_graph) : sc_module("ActorDisabler"), liste() {
    Tester tester("tester");
    graph = smoc_graph;
    char* script = getenv("FAILURE_INJECT");
    if(script!=NULL){
    scriptfile=true;
    parseScriptFile(script);
    }else{
    cout<<"no Config-File set!"<<endl<<"Please set FAILURE_INJECT"<<endl;
    scriptfile=false;
    }
    SC_THREAD(changer);
  }

    SC_HAS_PROCESS(ActorDisabler);

  //Abfragefunktion realisierung --- wird aus der Anwendung aufgerufen, wenn die ID bekannt ist!
  bool isDefect(ChannelId id){
  //  cout<<"isDefect("<<id<< ")= "<< ChannelStatus[id].status << endl;
    return ChannelStatus[id].status!=0;
  }

  //Registriert den Kanal id = Kanal-ID ; releasedBy = schreibender Task
  void registerChannel(ChannelId id, std::string releasedBy){
    channel_element neu={0,releasedBy};
    IDmap[releasedBy]=id;
    if(id >= ChannelStatus.size()){
      channel_element dummy={0,""};
      ChannelStatus.resize(id+100, dummy);
    }
    ChannelStatus[id] = neu;
    //cout<<"Channel "<<id<< " == "<<releasedBy<<" registered"<<endl;
  }

  //Hauptthread, durchaeuft die Schedule-liste und fuehrt entsprechende Aenderungen durch
  void changer();
private:
  //get all Tasks bound to the specific Component
  std::vector<SystemC_VPC::ProcessId> * getTaskAnnotation(std::string compName){
    //ask the Director for the List
    return SystemC_VPC::Director::getInstance().getTaskAnnotation(compName);
  }

  //disable the Task
  void deactivate_task(SystemC_VPC::ProcessId id, unsigned int change ){
    //need to find out, which Links are affected
    std::cout<<"deactivate_task(ProcessId id, unsigned int change )" << id <<" " << change << std::endl;
    for(ChannelId i=0; i<ChannelStatus.size(); i++){
      struct channel_element temp = ChannelStatus[i];
      if(id == SystemC_VPC::Director::getInstance().getProcessId(temp.releasedBy)){
        // invalidate_link(i, change);
      }
    }
  }

  //disable the Task
  void deactivate_task(std::string process){
    std::cout<<"deactivate_task(std::string id)" << process << std::endl;
    //need to find out, which Links are affected
    graph->disableActor(process);

  }

  //enable the Task
  void activate_task(std::string process){
    std::cout<<"activate_task(std::string id)" << process << std::endl;
    //need to find out, which Links are affected
    graph->reEnableActor(process);

  }

  //load the Schedule-Script
  void parseScriptFile(std::string filename);

   //traegt Befehle in den Ablaufplan ein
  void addListElement(component_kind kindOf, std::string resName, sc_time when, unsigned int change){
    liste.push_back( list_element(kindOf,resName,when,change) );
  }

};

#endif // __INCLUDED__ACTORDISABLE_hpp__
