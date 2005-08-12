#include <hscd_vpc_Director.h>
#include "systemc.h"
#include "names.h"
#include "rlc_id106011.h"
using namespace SystemC_VPC;

void rlc_id106011::rlc_id106011_process()
{
while(true){
bool var_c_q_rlc2rlc_id106525 = port_c_q_rlc2rlc_id106525->read();
bool var_c_sf_rlc2rlc_id106560 = port_c_sf_rlc2rlc_id106560->read();
AbstractComponent& r=Director::getInstance().getResource(RLC_ID106011);
sc_event *ev=new sc_event(); 
r.compute(RLC_ID106011,ev);
wait(*(ev));
cout << "RLC triggered!" << endl << endl;
to_monitor->write(true);
}
}
