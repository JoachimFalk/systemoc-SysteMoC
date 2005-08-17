#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "diff_id106003.h"

void diff_id106003::diff_id106003_process()
{
while(true){
bool var_c_in_diff2diff_id106507 = port_c_in_diff2diff_id106507->read();
bool var_c_lf_diff2diff_id106506 = port_c_lf_diff2diff_id106506->read();
bool var_diff2c_diff_dct_id106508 = 0;
AbstractComponent& r=Director::getInstance().getResource(DIFF_ID106003);
smoc_event *ev=new smoc_event(); 
r.compute(DIFF_ID106003,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "DIFF triggered!" << endl << endl;
port_diff2c_diff_dct_id106508->write(var_diff2c_diff_dct_id106508);
}
}
