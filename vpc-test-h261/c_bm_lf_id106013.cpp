#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_bm_lf_id106013.h"

void c_bm_lf_id106013::c_bm_lf_id106013_process()
{
while(true){
bool var_bm2c_bm_lf_id106527 = port_bm2c_bm_lf_id106527->read();
bool var_c_bm_lf2lf_id106528 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_BM_LF_ID106013);
smoc_event *ev=new smoc_event(); 
r.compute(C_BM_LF_ID106013,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "C_BM_LF triggered!" << endl << endl;
port_c_bm_lf2lf_id106528->write(var_c_bm_lf2lf_id106528);
}
}
