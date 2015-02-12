//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2012 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * --- This software and any associated documentation is provided "as is"
 *
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef TRANSCEIVERM_ASPECTMODELS_HPP_
#define TRANSCEIVERM_ASPECTMODELS_HPP_


/*
 * Transceiver Model
 * v0.1
 *
 * started: June-2012
 * Rafael Rosales
 * Hardware-Software-Co-Design
 * University of Erlangen-Nuremberg
 *
 * change: rosales      -       22-June-2012 -       Project Created
 * change: rosales      -       26-June-2012 -       First empty AMS model
 * change: rosales      -       27-June-2012 -       PowerModel, PowerMonitor ControlTimingModel, ControlScheduler & ControlIC created
 *
 *
 * */

#include <CoSupport/compatibility-glue/nullptr.h>

#include <cstdlib>
#include <stdlib.h>
#include <helpers.hpp>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>


/**
 *
 * This interface specifies the behavior of the Control IC
 *
 * It coordinates the execution of the control logic and the simulated timing for each control action
 *
 */
class ControlICBehavior
: public Booter_Scheduler_FirmwareIC  {

private:

  smoc_firing_state waitingTiming;


public:

  /**
   * Request timing for a given action
   * param: action name
   */
  smoc_port_out<Request<Transition*> > outGetTiming;
  /**
   * Response to timing request
   * returns: timing value
   */
  smoc_port_in<Response<sc_time*> > inTiming;

  ControlICBehavior( sc_module_name name)
  : Booter_Scheduler_FirmwareIC(name)

  {
    start=  (GUARD(Booter_Scheduler_FirmwareIC::canExecuteA) && outGetTiming(1))>>
        CALL(ControlICBehavior::requestTimingValue) >>
        waitingTiming
        |
        !GUARD(Booter_Scheduler_FirmwareIC::canExecuteA) >>
        CALL(Booter_Scheduler_FirmwareIC::waitForTransition) >>
        start;

    waitingTiming = inTiming(1) >>
        CALL(ControlICBehavior::simulateTime) >>
        CALL(Booter_Scheduler_FirmwareIC::executeA) >>
        start;

  }


  void requestTimingValue()
  {
    log->showAndLogMessage("Requesting timing value");
    Transition* tr = new Transition(*actor);
    this->actor->getCurrentTransition(*tr);
    Request<Transition*> rqst(tr);
    outGetTiming[0]= rqst;

  }

  void simulateTime()
  {
#ifdef LOG
    log->showAndLogMessage("Executing actor transition...");
#endif

    Response<sc_time*> rsp = inTiming[0];
    sc_time* t = rsp.object;
    wait(*t);

  }



};




class ControlLogic_TimingModel
: public ControlLogicTimingModelI {
private:

public:

  ControlLogic_TimingModel( sc_module_name name)
  : ControlLogicTimingModelI(name)

  {
    parser();
  }

  void addActionTiming(string aName, sc_time* timing)
  {
    timings[aName]=timing;
  }

  virtual void calculateTransitionTiming()
  {
    Request<Transition*> rqst = inGetTiming[0];
    Transition* transition = rqst.object;
    //string name = transition->actionNames->front();
    //log->showAndLogMessage(name);

    sc_time* transitionTiming= new sc_time(0,SC_NS);
    for(list<string>::iterator it= transition->guardNames->begin();it != transition->guardNames->end();it++)
      {
        string guardName = (*it);

        //verify transition timing contains this guard, comment to speed up
        if(timings.find(guardName) == timings.end())
          {
            //#ifdef LOG
            //log->showAndLogMessage("Configuration Error. No timing specified for guard: " + guardName);
            //#endif
            continue;
            cout << "Configuration Error. No timing specified for guard: " << guardName<< endl;
            exit(-1);
          }

        *transitionTiming += *timings[guardName];
      }
    for(list<string>::iterator it= transition->actionNames->begin();it != transition->actionNames->end();it++)
      {
        string actionName = (*it);

        //verify transition timing contains this guard, comment to speed up
        if(timings.find(actionName) == timings.end())
          {
            //#ifdef LOG
            //log->showAndLogMessage("Configuration Error. No timing specified for action: " + actionName);
            //#endif
            continue;
            cout << "Configuration Error. No timing specified for action: " << actionName<< endl;
            exit(-1);
          }

        *transitionTiming += *timings[actionName];
      }

    this->calculatedTiming = transitionTiming;

  }

  void parser()
  {
    //Traverse all XML's in search of the Timing annotations
    const path & dir_path= boost::filesystem::current_path();
    directory_iterator end_itr; // default construction yields past-the-end


    for ( directory_iterator itr( dir_path ); itr != end_itr; ++itr )
      {

        if ((*itr).path().extension() == ".xml")
          {
#if BOOST_FILESYSTEM_VERSION > 2
            string xmlFileName = (*itr).path().filename().native();
#else
            string xmlFileName = (*itr).path().filename();
#endif


            try {
                XMLPlatformUtils::Initialize();
            }
            catch(const XMLException& e){
                this->log->showAndLogMessage("Error initializing Xerces:");
            }

            this->log->showAndLogMessage("Parsing: "+xmlFileName);
            bool FALLBACKMODE=false;

            // open file and check existence
            FILE* fconffile;
            const char* cfile = (*itr).path().string().c_str();

            if(cfile){
                fconffile=fopen(cfile,"r");
                if( nullptr == fconffile ){       // test if file exists
                    //std::cout << "=== null : " << cfile <<std::endl;
                    std::cerr << "Warning: could not open '" << cfile << "'" << std::endl;
                    FALLBACKMODE=true;
                }else{
                    fclose(fconffile);
                    //std::cout << "=== else: " << std::endl;
                }
            }else{
                FALLBACKMODE=true;
            }

            // init vars for parsing
            if(FALLBACKMODE){
                //this->director->FALLBACKMODE = true;
                //DBG_OUT("running fallbackmode" << std::endl);
            }else{
                // process xml
                XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* vpcConfigDoc;
                XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilder* configParser;
                static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
                DOMImplementation* configImpl = DOMImplementationRegistry::getDOMImplementation(gLS);
                // create an error handler and install it
                //VpcDomErrorHandler* configErrorh=new VpcDomErrorHandler();//rrr
                configParser =
                    ((DOMImplementationLS*)configImpl)->
                    createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
                // turn on validation
                configParser->setFeature(XMLUni::fgDOMValidation, true);
                //configParser->setErrorHandler(configErrorh);//rrr

                configParser->setFeature(XMLUni::fgDOMWRTWhitespaceInElementContent, false);
                configParser->setFeature(XMLUni::fgDOMWhitespaceInElementContent, false);
                //http://apache.org/xml/features/dom/include-ignorable-whitespace
                try {
                    // reset document pool - clear all previous allocated data
                    configParser->resetDocumentPool();
                    vpcConfigDoc = configParser->parseURI(cfile);
                }
                catch (const XMLException& toCatch) {
                    this->log->showAndLogMessage("Error while parsing xml file: " +  xmlFileName + ": " + NStr( toCatch.getMessage()) + "\n");
                    return;
                }
                catch (const DOMException& toCatch) {
                    this->log->showAndLogMessage("Error while parsing xml file:" + xmlFileName + "'\n"+ "DOMException code is:  " + NStr( toCatch.msg));
                    const unsigned int maxChars = 2047;
                    XMLCh errText[maxChars + 1];

                    if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                        errText,
                        maxChars))
                      std::cerr << "Message is: " << NStr( errText) << std::endl;
                    return;
                }
                catch (...) {
                    std::cout << "Unexpected exception while parsing" << " xml file: '" << cfile << "'\n";
                    return;
                }

                //rrr
                //check if parsing failed
                //if(configErrorh->parseFailed()){
                //DBG_OUT("VPCBuilder: Parsing of configuration failed,"                " aborting initialization!" << std::endl);
                //  return;
                //}

                // set treewalker to documentroot
                DOMTreeWalker* vpcConfigTreeWalker=
                    vpcConfigDoc->createTreeWalker(
                        (DOMNode*)vpcConfigDoc->getDocumentElement(),
                        DOMNodeFilter::SHOW_ELEMENT, 0,
                        true);

                vpcConfigTreeWalker->setCurrentNode(
                    (DOMNode*)vpcConfigDoc->getDocumentElement());

                // moves the Treewalker to the first Child
                DOMNode* node = vpcConfigTreeWalker->getCurrentNode()->getFirstChild();
                // name of xmlTag
                XStr topNodeName;

                CoSupport::XML::Xerces::XStr toptag = "Maestro";
                CoSupport::XML::Xerces::XStr annotationstag = "Annotations";
                CoSupport::XML::Xerces::XStr tannotationstag = "TimingAnnotations";
                CoSupport::XML::Xerces::XStr timingmodeltag = "model";
                CoSupport::XML::Xerces::XStr timingmodelVersion = "ATTL v1.0";
                CoSupport::XML::Xerces::XStr resourcetag = "Resource";
                CoSupport::XML::Xerces::XStr resourceidtag = "id";
                CoSupport::XML::Xerces::XStr timingtag = "Timing";
                CoSupport::XML::Xerces::XStr actionidtag = "action";
                CoSupport::XML::Xerces::XStr valueidtag = "value";
                CoSupport::XML::Xerces::XStr unitsidtag = "units";

                //Outermost loop
                while(node!=0){
                    topNodeName = node->getNodeName();

                    //Check for Annotations XML
                    if( topNodeName == annotationstag )
                      {
                        DOMNode* annotationTypenode = node->getFirstChild();

                        if(annotationTypenode != nullptr){
                            //loop for all annotationtypes
                            for(; annotationTypenode != nullptr; annotationTypenode = annotationTypenode->getNextSibling()){
                                const XStr annotationTypeNodeName = annotationTypenode->getNodeName();

                                //If Timing Annotations Found
                                if(annotationTypeNodeName == tannotationstag)
                                  {
                                    DOMNode* timingModelIdAtt = annotationTypenode->getAttributes()->getNamedItem(timingmodeltag);
                                    const XStr timingModelIdName = timingModelIdAtt ->getNodeValue();

                                    if(timingModelIdName != timingmodelVersion)
                                      {
                                        this->log->showAndLogMessage(xmlFileName+"is not compatible with: "+to_string(timingmodelVersion));
                                        exit(-1);
                                      }
                                    else
                                      {
                                        this->log->showAndLogMessage(xmlFileName+"compatible with: "+to_string(timingmodelVersion));
                                      }


                                    DOMNode* resourcenode = annotationTypenode->getFirstChild();
                                    if(resourcenode != nullptr){
                                        //loop for all resources
                                        for(; resourcenode != nullptr; resourcenode = resourcenode->getNextSibling()){
                                            const XStr resourceNodeName = resourcenode->getNodeName();

                                            //Check for Resource
                                            if(resourceNodeName == resourcetag)
                                              {
                                                //Check this resource is target
                                                DOMNode* resourceAtt = resourcenode->getAttributes()->getNamedItem(resourceidtag);
                                                if(resourceAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                const XStr resourceName = resourceAtt->getNodeValue();

                                                //Get name of Component containing this TimingModel
                                                string timingModelName = this->getName();
                                                string componentName;
                                                int lastPoint = timingModelName.find_last_of('.');
                                                componentName= timingModelName.substr(0,lastPoint);
                                                if(resourceName != componentName)
                                                  {
                                                    continue;
                                                  }
                                                else
                                                  {
                                                    //cout << "Match" << endl;
                                                  }

                                                DOMNode* timingnode = resourcenode->getFirstChild();

                                                if(timingnode != nullptr){
                                                    //loop for all timings
                                                    for(; timingnode != nullptr; timingnode = timingnode->getNextSibling()){
                                                        const XStr timingNodeName = timingnode->getNodeName();

                                                        //Check for Timing
                                                        if(timingNodeName == timingtag)
                                                          {

                                                            DOMNode* actionAtt = timingnode->getAttributes()->getNamedItem(actionidtag);
                                                            if(actionAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                            const XStr actionName = actionAtt->getNodeValue();

                                                            DOMNode* valueAtt = timingnode->getAttributes()->getNamedItem(valueidtag);
                                                            if(valueAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                            const XStr valueName = valueAtt->getNodeValue();
                                                            double val;
                                                            val = XMLString::parseInt(valueName);

                                                            DOMNode* unitsAtt = timingnode->getAttributes()->getNamedItem(unitsidtag);
                                                            if(unitsAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                            const XStr unitsName = unitsAtt->getNodeValue();
                                                            sc_time* scUnit;
                                                            double scale=1;
                                                            if(unitsName == "ps")
                                                              {
                                                                scale=1;
                                                              }
                                                            else if(unitsName == "ns")
                                                              {
                                                                scale=1000;
                                                              }
                                                            else if(unitsName == "us")
                                                              {
                                                                scale=1000000;
                                                              }
                                                            else if(unitsName == "ms")
                                                              {
                                                                scale=1000000000;
                                                              }
                                                            else if(unitsName == "s")
                                                              {
                                                                scale=1000000000000;
                                                              }
                                                            else
                                                              {
                                                                this->log->showAndLogMessage("Warning. Units not recognized in: " + to_string(actionName.c_str()) + ", assuming picoseconds");
                                                              }

                                                            scUnit = new sc_time(val*scale,false);
                                                            this->timings[actionName]= scUnit;

                                                          }

                                                    }
                                                }
                                                else//no action found, keep parsing other XMLs
                                                  {
                                                  }
                                              }
                                        }
                                    }


                                  }
                            }


                        }

                      }

                    node=node->getNextSibling();
                    //int a; cin>> a;
                }
            }
          }
        else
          {
            continue;//try next XML file
          }
      }


  }
};

/**
 * Model of the digital IC controlling the analog component
 *
 * Aspects:
 *
 * Scheduler (ControlIC)
 * TimingModel
 */
class DigitalControl
: public smoc_graph{


private:

  ControlICBehavior    controlIC;


public:

  ControlLogic_TimingModel      controlLogicTiming;

  DigitalControl( sc_module_name name)
  : smoc_graph(name),
    controlIC("ControlIC"),
    controlLogicTiming("ControlTimingModel")

  {

    //Connect ControlICBehavior to ControlTiming
    connectNodePorts(controlIC.inTiming,controlLogicTiming.outTiming);
    connectNodePorts(controlIC.outGetTiming,controlLogicTiming.inGetTiming);


  }


};

/**
 * Implements the power model interface, defining the methods to calculate the power consumption of the component
 * as well as providing an API to specify the back-annotated power consumption values
 */
class AMS_PowerModel
: public AMS_PowerModelI {
private:

  /**
   * Power consumption on a given state (and no transition taking place)
   *
   * Key: power state
   * value: power consumption
   */
  map<string,double> statePowerConsumption;

  /**
   * Power consumption at a transition
   *
   * Key: source state
   * value: <destination state, power consumption>
   */
  map<string,map<string,double> > transitionPowerConsumption;


public:

  AMS_PowerModel( sc_module_name name)
  : AMS_PowerModelI(name)

  {
    currentState="on";

    parser();

  }

  virtual double calculateStatePowerConsumption(string currentState)
  {
    return statePowerConsumption[currentState];
  }


  virtual double calculateTransitionPowerConsumption(string sourceState,string destinationState)
  {
    return transitionPowerConsumption[sourceState][destinationState];
  }


  /**
   * Method to feed the back-annotation values
   */
  void setStatePowerConsumption(string powerState, double powerConsumption)
  {
    this->statePowerConsumption[powerState] = powerConsumption;
  }

  /**
   * Method to feed the back-annotation values
   */
  void setTransitionPowerConsumption(string sourcePowerState, string destinationPowerState,double powerConsumption)
  {
    this->transitionPowerConsumption[sourcePowerState][destinationPowerState]= powerConsumption;
  }

  void parser()
  {
    {
      //Traverse all XML's in search of the Power annotations
      const path & dir_path= boost::filesystem::current_path();
      directory_iterator end_itr; // default construction yields past-the-end


      for ( directory_iterator itr( dir_path ); itr != end_itr; ++itr )
        {

          if ((*itr).path().extension() == ".xml")
            {


#if BOOST_FILESYSTEM_VERSION > 2
              string xmlFileName = (*itr).path().filename().native();
#else
              string xmlFileName = (*itr).path().filename();
#endif

              try {
                  XMLPlatformUtils::Initialize();
              }
              catch(const XMLException& e){
                  this->log->showAndLogMessage("Error initializing Xerces:" );
              }

              this->log->showAndLogMessage("Parsing: " + xmlFileName);
              bool FALLBACKMODE=false;

              // open file and check existence
              FILE* fconffile;
              const char* cfile = (*itr).path().string().c_str();

              if(cfile){
                  fconffile=fopen(cfile,"r");
                  if( nullptr == fconffile ){       // test if file exists
                      //std::cout << "=== null : " << cfile <<std::endl;
                      std::cerr << "Warning: could not open '" << cfile << "'" << std::endl;
                      FALLBACKMODE=true;
                  }else{
                      fclose(fconffile);
                      //std::cout << "=== else: " << std::endl;
                  }
              }else{
                  FALLBACKMODE=true;
              }

              // init vars for parsing
              if(FALLBACKMODE){
                  //this->director->FALLBACKMODE = true;
                  //DBG_OUT("running fallbackmode" << std::endl);
              }else{
                  // process xml
                  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* vpcConfigDoc;
                  XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilder* configParser;
                  static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
                  DOMImplementation* configImpl = DOMImplementationRegistry::getDOMImplementation(gLS);
                  // create an error handler and install it
                  //VpcDomErrorHandler* configErrorh=new VpcDomErrorHandler();//rrr
                  configParser =
                      ((DOMImplementationLS*)configImpl)->
                      createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
                  // turn on validation
                  configParser->setFeature(XMLUni::fgDOMValidation, true);
                  //configParser->setErrorHandler(configErrorh);//rrr

                  configParser->setFeature(XMLUni::fgDOMWRTWhitespaceInElementContent, false);
                  configParser->setFeature(XMLUni::fgDOMWhitespaceInElementContent, false);
                  //http://apache.org/xml/features/dom/include-ignorable-whitespace
                  try {
                      // reset document pool - clear all previous allocated data
                      configParser->resetDocumentPool();
                      vpcConfigDoc = configParser->parseURI(cfile);
                  }
                  catch (const XMLException& toCatch) {
                      this->log->showAndLogMessage(" Error while parsing xml file: " +  xmlFileName + ": " + NStr( toCatch.getMessage()) + "\n" );
                      return;
                  }
                  catch (const DOMException& toCatch) {
                      this->log->showAndLogMessage("Error while parsing xml file:" + xmlFileName + "'\n"+ "DOMException code is:  " + NStr( toCatch.msg) );
                      const unsigned int maxChars = 2047;
                      XMLCh errText[maxChars + 1];

                      if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                          errText,
                          maxChars))
                        std::cerr << "Message is: " << NStr( errText) << std::endl;
                      return;
                  }
                  catch (...) {
                      this->log->showAndLogMessage("Unexpected exception while parsing xml file: " + xmlFileName);
                      return;
                  }

                  //rrr
                  //check if parsing failed
                  //if(configErrorh->parseFailed()){
                  //DBG_OUT("VPCBuilder: Parsing of configuration failed,"                " aborting initialization!" << std::endl);
                  //  return;
                  //}

                  // set treewalker to documentroot
                  DOMTreeWalker* vpcConfigTreeWalker=
                      vpcConfigDoc->createTreeWalker(
                          (DOMNode*)vpcConfigDoc->getDocumentElement(),
                          DOMNodeFilter::SHOW_ELEMENT, 0,
                          true);

                  vpcConfigTreeWalker->setCurrentNode(
                      (DOMNode*)vpcConfigDoc->getDocumentElement());

                  // moves the Treewalker to the first Child
                  DOMNode* node = vpcConfigTreeWalker->getCurrentNode()->getFirstChild();
                  // name of xmlTag
                  XStr topNodeName;

                  CoSupport::XML::Xerces::XStr toptag = "Maestro";
                  CoSupport::XML::Xerces::XStr annotationstag = "Annotations";
                  CoSupport::XML::Xerces::XStr pannotationstag = "PowerAnnotations";
                  CoSupport::XML::Xerces::XStr powermodeltag = "model";
                  CoSupport::XML::Xerces::XStr resourcetag = "Resource";
                  CoSupport::XML::Xerces::XStr resourceidtag = "id";
                  CoSupport::XML::Xerces::XStr powermodelVersion = "Analog PSMs v1.0";
                  CoSupport::XML::Xerces::XStr powerstatetag = "PowerState";
                  CoSupport::XML::Xerces::XStr powerstateidtag = "id";
                  CoSupport::XML::Xerces::XStr initpowerstateidtag = "initialState";
                  CoSupport::XML::Xerces::XStr spowerconsumptiontag = "powerConsumption";
                  CoSupport::XML::Xerces::XStr spowerunitsidtag = "power_units";
                  CoSupport::XML::Xerces::XStr initialstateidtag = "initialState";

                  CoSupport::XML::Xerces::XStr transitiontag = "Transition";
                  CoSupport::XML::Xerces::XStr transitionsourcetag = "source";
                  CoSupport::XML::Xerces::XStr transitiondestinationtag = "destination";
                  CoSupport::XML::Xerces::XStr tpowerconsumptiontag = "powerConsumption";
                  CoSupport::XML::Xerces::XStr tpowerunitsidtag = "power_units";

                  //Outermost loop
                  while(node!=0){
                      topNodeName = node->getNodeName();

                      //Check for Annotations XML
                      if( topNodeName == annotationstag )
                        {
                          DOMNode* annotationTypenode = node->getFirstChild();

                          if(annotationTypenode != nullptr){
                              //loop for all annotationtypes
                              for(; annotationTypenode != nullptr; annotationTypenode = annotationTypenode->getNextSibling()){
                                  const XStr annotationTypeNodeName = annotationTypenode->getNodeName();

                                  //If Power Annotations Found
                                  if(annotationTypeNodeName == pannotationstag)
                                    {
                                      DOMNode* timingModelIdAtt = annotationTypenode->getAttributes()->getNamedItem(powermodeltag);
                                      const XStr timingModelIdName = timingModelIdAtt ->getNodeValue();

                                      if(timingModelIdName != powermodelVersion)
                                        {
                                          this->log->showAndLogMessage(xmlFileName+  "is not compatible with: "+to_string(powermodelVersion));
                                          exit(-1);
                                        }
                                      else
                                        {
                                          this->log->showAndLogMessage(xmlFileName+  " compatible with: "+to_string(powermodelVersion));
                                        }


                                      DOMNode* resourcenode = annotationTypenode->getFirstChild();
                                      if(resourcenode != nullptr){
                                          //loop for all resources
                                          for(; resourcenode != nullptr; resourcenode = resourcenode->getNextSibling()){
                                              const XStr resourceNodeName = resourcenode->getNodeName();

                                              //Check for Resource
                                              if(resourceNodeName == resourcetag)
                                                {
                                                  //Check this resource is target
                                                  DOMNode* resourceAtt = resourcenode->getAttributes()->getNamedItem(resourceidtag);
                                                  if(resourceAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                  const XStr resourceName = resourceAtt->getNodeValue();

                                                  //Get name of Component containing this TimingModel
                                                  string powerModelName = this->getName();
                                                  string componentName;
                                                  int lastPoint = powerModelName.find_last_of('.');
                                                  componentName= powerModelName.substr(0,lastPoint);
                                                  if(resourceName != componentName)
                                                    {
                                                      continue;
                                                    }
                                                  else
                                                    {
                                                      //cout << "Match" << endl;
                                                    }

                                                  DOMNode* powerstate_transnode = resourcenode->getFirstChild();

                                                  if(powerstate_transnode != nullptr){
                                                      //loop for all powerstates and transitions
                                                      for(; powerstate_transnode != nullptr; powerstate_transnode = powerstate_transnode->getNextSibling()){
                                                          const XStr powerStateNodeName = powerstate_transnode->getNodeName();

                                                          //Check for PowerState
                                                          if(powerStateNodeName == powerstatetag)
                                                            {



                                                              DOMNode* powerStateAtt = powerstate_transnode->getAttributes()->getNamedItem(powerstateidtag);
                                                              if(powerStateAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                              const XStr powerStateName = powerStateAtt->getNodeValue();

                                                              DOMNode* initpowerStateAtt = powerstate_transnode->getAttributes()->getNamedItem(initpowerstateidtag);
                                                              if(initpowerStateAtt != nullptr){//Argument is optional
                                                              const XStr initpowerState = initpowerStateAtt->getNodeValue();
                                                              if(initpowerState == "true")
                                                              {
                                                            	  this->currentState = powerStateName;
                                                              }
                                                              }

                                                              DOMNode* statePowerConsumptionAtt = powerstate_transnode->getAttributes()->getNamedItem(spowerconsumptiontag);
                                                              if(statePowerConsumptionAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                              const XStr valueName = statePowerConsumptionAtt->getNodeValue();
                                                              double val;
                                                              val = XMLString::parseInt(valueName);

                                                              DOMNode* statePowerUnitsAtt = powerstate_transnode->getAttributes()->getNamedItem(spowerunitsidtag);
                                                              if(statePowerUnitsAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                              const XStr unitsName = statePowerUnitsAtt->getNodeValue();

                                                              double scale=1;
                                                              if(unitsName == "pW")
                                                                {
                                                                  scale=.000000001;
                                                                }
                                                              else if(unitsName == "nW")
                                                                {
                                                                  scale=.000001;
                                                                }
                                                              else if(unitsName == "uW")
                                                                {
                                                                  scale=.001;
                                                                }
                                                              else if(unitsName == "mW")
                                                                {
                                                                  scale=1;
                                                                }
                                                              else if(unitsName == "W")
                                                                {
                                                                  scale=1000;
                                                                }
                                                              else if(unitsName == "kW")
                                                                {
                                                                  scale=1000000;
                                                                }
                                                              else if(unitsName == "MW")
                                                                {
                                                                  scale=1000000000;
                                                                }
                                                              else
                                                                {
                                                                  this->log->showAndLogMessage("Warning. Units not recognized in: " + to_string(powerStateName.c_str()) + ", assuming milliWatts");
                                                                }

                                                              //this->log->showAndLogMessage("State " + to_string(powerStateName) + ", pC: " + to_string(val*scale));
                                                              this->statePowerConsumption[powerStateName]= val*scale;

                                                            }
                                                          else //Check for Transitions
                                                            if(powerStateNodeName == transitiontag)
                                                              {

                                                                DOMNode* sourceAtt = powerstate_transnode->getAttributes()->getNamedItem(transitionsourcetag);
                                                                if(sourceAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                                const XStr sourceName = sourceAtt->getNodeValue();

                                                                DOMNode* destAtt = powerstate_transnode->getAttributes()->getNamedItem(transitiondestinationtag);
                                                                if(destAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                                const XStr destName = destAtt->getNodeValue();

                                                                DOMNode* transPowerConsumptionAtt = powerstate_transnode->getAttributes()->getNamedItem(spowerconsumptiontag);
                                                                if(transPowerConsumptionAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                                const XStr valueName = transPowerConsumptionAtt->getNodeValue();
                                                                double val;
                                                                val = XMLString::parseInt(valueName);

                                                                DOMNode* transPowerUnitsAtt = powerstate_transnode->getAttributes()->getNamedItem(spowerunitsidtag);
                                                                if(transPowerUnitsAtt == nullptr){throw new std::exception;}//Validate argument was found
                                                                const XStr unitsName = transPowerUnitsAtt->getNodeValue();
                                                                sc_time* scUnit;
                                                                double scale=1;
                                                                if(unitsName == "pW")
                                                                  {
                                                                    scale=.000000001;
                                                                  }
                                                                else if(unitsName == "nW")
                                                                  {
                                                                    scale=.000001;
                                                                  }
                                                                else if(unitsName == "uW")
                                                                  {
                                                                    scale=.001;
                                                                  }
                                                                else if(unitsName == "mW")
                                                                  {
                                                                    scale=1;
                                                                  }
                                                                else if(unitsName == "W")
                                                                  {
                                                                    scale=1000;
                                                                  }
                                                                else if(unitsName == "kW")
                                                                  {
                                                                    scale=1000000;
                                                                  }
                                                                else if(unitsName == "MW")
                                                                  {
                                                                    scale=1000000000;
                                                                  }
                                                                else
                                                                  {
                                                                    this->log->showAndLogMessage("Warning. Units not recognized in: " + to_string(sourceName.c_str()) + ", assuming milliWatts");
                                                                  }

                                                                //this->log->showAndLogMessage("Transition " + to_string(sourceName) + " " + to_string(destName) + ", pC: " + to_string(val*scale));
                                                                this->transitionPowerConsumption[sourceName][destName]= val*scale;

                                                              }

                                                      }
                                                  }
                                                  else//no action found, keep parsing other XMLs
                                                    {
                                                    }
                                                }
                                          }
                                      }


                                    }
                              }


                          }

                        }

                      node=node->getNextSibling();
                      //int a; cin>> a;
                  }
              }
            }
          else
            {
              continue;//try next XML file
            }
        }


    }
  }
};



/**
 * Generates power traces
 */
class PowerMonitor: public IPowerMonitor {

private:
  /*
   * Key   = Component Name
   * Value = Power Consumption
   */
  map<string,double> currentPowerConsumption;

  /*
   * Key   = Component Name
   * Value = Last Power Consumption
   */
  map<string,double> lastPowerConsumption;

  /*
   * Key   = Component Name
   * Value = Last time (not real simulation time) power consumption changed
   */
  map<string, sc_time> lastTimeChanged;

  sc_time tlastTimePrinted;

  /*
   * Whenever one of the contained components updates its power consumption,
   * if time has moved forward (since last update from the same component)
   * then prints the total current consumption aggregating all components
   */
  virtual void updatePowerConsumption() {

    sc_time currentTime = sc_time_stamp();

    //FAST_cout << this->nname << ": Component::" << componentName <<" current power consumption value:...." << colorize(powerConsumption,true,0) << " at: "<< sc_time_stamp()  <<std::endl;


    //Update current power consumption
    currentPowerConsumption[componentName]=powerConsumption;

#ifdef LOG
    log->logMessage(componentName + ": last power: " + colorize(lastPowerConsumption[componentName],true,2));
    log->logMessage(componentName + ": lastTimeCh: " + to_string(lastTimeChanged[componentName]));
    log->showAndLogMessage(/*componentName + ": */"New power: " + to_string(powerConsumption) + " mW" /*+ " s:" + to_string(pCstable)*/);
#endif

    //If time has moved fwd, print last power consumption

    //find if some component has an old lasttimechanged
    for(map<string, sc_time>::iterator i=lastTimeChanged.begin();i != lastTimeChanged.end();i++)
      {
        string cN = (*i).first;
        sc_time lastTime =(*i).second;

        if(currentTime > lastTime && lastTime >= tlastTimePrinted)
          {
            printTotalPowerConsumption(lastTime);
            //printTotalPowerConsumption(tlastTimeChanged);
            tlastTimePrinted = currentTime;
          }

      }
    /*
        if(currentTime > lastTimeChanged[componentName] && lastTimeChanged[componentName] >= tlastTimePrinted)
        //if(currentTime > tlastTimeChanged)
          {
            printTotalPowerConsumption(lastTimeChanged[componentName]);
            //printTotalPowerConsumption(tlastTimeChanged);
            tlastTimePrinted = currentTime;
          }
     */
    //update last time changed ("real simulated" time)
    lastTimeChanged[componentName] = currentTime;

    //Update last power consumption (idea is to have a single delayed (not "real" simulated time, but partial order time) copy of the power consumption)
    lastPowerConsumption[componentName]=currentPowerConsumption[componentName];

  }

  void printPowerConsumption(double p,string& componentName){
    //FAST_cout << this->nname << ": Component::" << componentName <<  " true Power consumption:........" << colorize(p,true,2) << " at: "<< lastTimeChanged[componentName]  <<std::endl;
  }


  void printTotalPowerConsumption(sc_time lastTimePowerChanged){
    double tPower=0;
    //FAST_cout << this->nname << ": Member components: " ;
    for(map<string,double>::iterator it = lastPowerConsumption.begin();it != lastPowerConsumption.end();it++)
      {
        string componentName = (*it).first;
        double power = (*it).second;
        //Print Component names

        //FAST_cout << componentName << ",";

        tPower += power;
      }
    //Print total consumption


    //cout << lastTimePowerChanged.to_double()/100  << "\t" << tPower << "\t" << sc_time_stamp().to_double()/100 << "\t" << this->getName() << endl;
    outputFile << lastTimePowerChanged.to_double()/1000  << "\t" << tPower << "\t" << sc_time_stamp().to_double()/1000 << "\t" << this->getName() << endl;

#ifdef LOG
    log->showAndLogMessage("Power consumption:" + colorize(tPower,true,2) + " mW at: "+ to_string(lastTimePowerChanged));
#endif
  }

  smoc_firing_state start;
  smoc_firing_state end;

  ofstream outputFile;

public:
  PowerMonitor(sc_module_name name)
  : IPowerMonitor(name) {
    tlastTimePrinted=sc_time_stamp();
    //Set enough precision to outputfile
    outputFile.precision(15);


    //const char *n = name;
    const char *t= (this->getName()+".dat").c_str();

    outputFile.open(t);

  }

  ~PowerMonitor()
  {
    //print last power consumption
    updatePowerConsumption();

    //Update current power consumption
    currentPowerConsumption[componentName]=powerConsumption;
    lastPowerConsumption[componentName]=currentPowerConsumption[componentName];
    printTotalPowerConsumption(tlastTimePrinted);

    outputFile.close();
  }
};



#endif/* TRANSCEIVERM_ASPECTMODELS_HPP_*/
