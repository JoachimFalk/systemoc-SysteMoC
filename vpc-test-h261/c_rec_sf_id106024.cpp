#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_rec_sf_id106024.h"

void c_rec_sf_id106024::c_rec_sf_id106024_process()
{
while(true){
bool var_rec2c_rec_sf_id106549 = port_rec2c_rec_sf_id106549->read();
bool var_c_rec_sf2sf_id106550 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_REC_SF_ID106024);
sc_event *ev=new sc_event(); 
r.compute(C_REC_SF_ID106024,ev);
wait(*(ev));
cout << "C_REC_SF triggered!" << endl << endl;
port_c_rec_sf2sf_id106550->write(var_c_rec_sf2sf_id106550);
}
}