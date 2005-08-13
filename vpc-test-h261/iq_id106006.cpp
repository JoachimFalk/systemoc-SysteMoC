#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "iq_id106006.h"

void iq_id106006::iq_id106006_process()
{
while(true){
bool var_c_q_iq2iq_id106513 = port_c_q_iq2iq_id106513->read();
bool var_iq2c_iq_idct_id106514 = 0;
AbstractComponent& r=Director::getInstance().getResource(IQ_ID106006);
smoc_event *ev=new smoc_event(); 
r.compute(IQ_ID106006,ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "IQ triggered!" << endl << endl;
port_iq2c_iq_idct_id106514->write(var_iq2c_iq_idct_id106514);
}
}
