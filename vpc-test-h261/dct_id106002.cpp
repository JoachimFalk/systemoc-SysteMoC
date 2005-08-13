#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "dct_id106002.h"

void dct_id106002::dct_id106002_process()
{
while(true){
bool var_c_diff_dct2dct_id106504 = port_c_diff_dct2dct_id106504->read();
bool var_dct2c_dct_q_id106505 = 0;
AbstractComponent& r=Director::getInstance().getResource(DCT_ID106002);
smoc_event *ev=new smoc_event(); 
r.compute(DCT_ID106002,ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "DCT triggered!" << endl << endl;
port_dct2c_dct_q_id106505->write(var_dct2c_dct_q_id106505);
}
}
