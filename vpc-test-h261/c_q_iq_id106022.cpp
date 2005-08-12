#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_q_iq_id106022.h"

void c_q_iq_id106022::c_q_iq_id106022_process()
{
while(true){
bool var_q2c_q_iq_id106545 = port_q2c_q_iq_id106545->read();
bool var_c_q_iq2iq_id106546 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_Q_IQ_ID106022);
sc_event *ev=new sc_event(); 
r.compute(C_Q_IQ_ID106022,ev);
wait(*(ev));
cout << "C_Q_IQ triggered!" << endl << endl;
port_c_q_iq2iq_id106546->write(var_c_q_iq2iq_id106546);
}
}
