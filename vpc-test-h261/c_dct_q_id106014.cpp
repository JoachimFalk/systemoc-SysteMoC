#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_dct_q_id106014.h"

void c_dct_q_id106014::c_dct_q_id106014_process()
{
while(true){
bool var_dct2c_dct_q_id106529 = port_dct2c_dct_q_id106529->read();
bool var_c_dct_q2q_id106530 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_DCT_Q_ID106014);
smoc_event *ev=new smoc_event(); 
r.compute(C_DCT_Q_ID106014,ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "C_DCT_Q triggered!" << endl << endl;
port_c_dct_q2q_id106530->write(var_c_dct_q2q_id106530);
}
}
