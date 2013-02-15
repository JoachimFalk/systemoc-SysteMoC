#include <ActorDisabler.hpp>


  void ActorDisabler::parseScriptFile(std::string filename){
    std::string line;
    ifstream schedulefile;
    sc_time relTime = SC_ZERO_TIME;

    schedulefile.open(filename.c_str());
    assert(schedulefile.fail()==false);
    /*
    Config-Format:
    @100ns
    *(COMMUNICATION/EXECUTION/COMPONENT)* *ResName* *Change@Time in SC_NS* (failure/repaired)
    z.B. COMMUNICATION ChannelDefect.Link-a1 100 failure
    z.B. EXECUTION ChannelDefect.a1 150 repaired
    */
    while (! schedulefile.eof() && schedulefile.is_open() ){
      getline(schedulefile, line);
      if(line.size()>0){
        std::string part1, part2;
        int pos1=0, pos2=0;
        if(line[0]=='@'){
          //switch to another time
          relTime=SystemC_VPC::Director::createSC_Time(line.substr(1).c_str());
          //relTime=sc_time(atoi((line.substr(1,line.find_first_of(" "))).c_str()),SC_NS);
        }else{//keine Aenderung des Zeitpunktes.. also kanns nur ne Schedule-Zeile sein
          component_kind res;
          std::string resname;
          pos1=line.find_first_of(" ");
          part1=line.substr(0, pos1);
          if(part1=="COMMUNICATION")res=COMMUNICATION;
          else if(part1=="EXECUTION")res=EXECUTION;
          else if(part1=="COMPONENT")res=COMPONENT;
          else if(part1=="ENABLE")res=ENABLE;
          else if(part1=="DISABLE")res=DISABLE;
          part2=line.substr(pos1+1, line.size()-pos1);
          pos2=part2.find_first_of(" ");
          resname=part2.substr(0,pos2);
          addListElement(res, resname, relTime, 1);
          //cerr << "add Element: " << res << ", " << resname << ", "
          //     << relTime  << ", " << change << endl;
        }
      }
    }
  }


  void ActorDisabler::changer(){
    //Liste der zu aendernden Aktoren durchlaufen
      for(size_t i=0;i<liste.size();i++){
        if(i==0){
          wait(liste[0].when);
        }else{
          //Quick-Fix, da wait(0) ein Schedule bewirken wuerde -> Simulation verfaelscht
          sc_time waitFor = liste[i].when - liste[i-1].when;
          if( waitFor != SC_ZERO_TIME ) wait(waitFor);
        }

        switch(liste[i].kind_of){
        case COMMUNICATION:
          {
           /*
           //cerr << "COMMUNICATION " << liste[i].resname<<" changed "<< endl;
           ChannelId test= SystemC_VPC::Director::getInstance().getProcessId(liste[i].resname);
           if(IDmap.find(liste[i].resname) == IDmap.end())
              IDmap[liste[i].resname]=test;
           assert( IDmap.find(liste[i].resname) != IDmap.end() );
           */
           //IDmap enthaelt Zuordnung Name - ID
          //invalidate_link(IDmap[liste[i].resname], liste[i].newstat);
          break;
          }
        case COMPONENT:
          {
            //cerr << "COMPONENT" << endl;
            //has to look after all running tasks on one single resource
            //and has to invalidate all outgoing links of these tasks
            std::vector<SystemC_VPC::ProcessId> * tasks = getTaskAnnotation(liste[i].resname);
            for(size_t a=0;a<tasks->size();a++){
              //deactivate_task((*tasks)[a]);
              //invalidate_link((*tasks)[a], liste[i].newstat );
            }
          }
          break;
        case ENABLE:
          //has to activate the task
          activate_task( liste[i].resname );
          break;
        case DISABLE:
          //has to deactivate the task
          deactivate_task( liste[i].resname);
          break;

        default:
          assert(0);
        }
      }
      //QUICK HACK: segmentation fault caused by process termination
      wait(100, SC_SEC);
      //cerr << "ChannelDefect::changer" << endl;
  }
