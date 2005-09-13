#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "sf_id106012.h"

void sf_id106012::sf_id106012_process()
{
while(true){
bool var_c_rec_sf2sf_id106526 = port_c_rec_sf2sf_id106526->read();
bool var_sf2c_sf_rlc_id106557 = 0;
AbstractComponent& r=Director::getInstance().getResource(SF_ID106012);
smoc_event *ev=new smoc_event(); 
r.compute(SF_ID106012,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "SF triggered!" << endl << endl;
port_sf2c_sf_rlc_id106557->write(var_sf2c_sf_rlc_id106557);
}
}
