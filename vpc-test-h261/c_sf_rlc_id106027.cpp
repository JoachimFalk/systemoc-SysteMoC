#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_sf_rlc_id106027.h"

void c_sf_rlc_id106027::c_sf_rlc_id106027_process()
{
while(true){
bool var_sf2c_sf_rlc_id106558 = port_sf2c_sf_rlc_id106558->read();
bool var_c_sf_rlc2rlc_id106559 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_SF_RLC_ID106027);
sc_event *ev=new sc_event(); 
r.compute(C_SF_RLC_ID106027,ev);
wait(*(ev));
cout << "C_SF_RLC triggered!" << endl << endl;
port_c_sf_rlc2rlc_id106559->write(var_c_sf_rlc2rlc_id106559);
}
}
