#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "in_id106005.h"

void in_id106005::in_id106005_process()
{
while(true){
bool var_in2c_in_diff_id106512 = 0;
bool var_in2c_in_rf_id106553 = 0;
bool var_in2c_in_bm_id106511 = 0;
from_monitor->read();
AbstractComponent& r=Director::getInstance().getResource(IN_ID106005);
//smoc_event *ev=new smoc_event(); 
 r.compute(IN_ID106005,""/*,ev*/);
//smoc_wait(*(ev));
//smoc_reset(*(ev));
cout << "IN triggered!" << endl <<  endl;
port_in2c_in_diff_id106512->write(var_in2c_in_diff_id106512);
port_in2c_in_rf_id106553->write(var_in2c_in_rf_id106553);
port_in2c_in_bm_id106511->write(var_in2c_in_bm_id106511);
}
}
