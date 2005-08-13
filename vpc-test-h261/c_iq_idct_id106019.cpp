#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_iq_idct_id106019.h"

void c_iq_idct_id106019::c_iq_idct_id106019_process()
{
while(true){
bool var_iq2c_iq_idct_id106539 = port_iq2c_iq_idct_id106539->read();
bool var_c_iq_idct2idct_id106540 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_IQ_IDCT_ID106019);
smoc_event *ev=new smoc_event(); 
r.compute(C_IQ_IDCT_ID106019,ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "C_IQ_IDCT triggered!" << endl << endl;
port_c_iq_idct2idct_id106540->write(var_c_iq_idct2idct_id106540);
}
}
