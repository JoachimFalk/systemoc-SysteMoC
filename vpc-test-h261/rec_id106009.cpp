#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "rec_id106009.h"

void rec_id106009::rec_id106009_process()
{
while(true){
bool var_c_idct_rec2rec_id106521 = port_c_idct_rec2rec_id106521->read();
bool var_c_lf_rec2rec_id106522 = port_c_lf_rec2rec_id106522->read();
bool var_rec2c_rec_sf_id106523 = 0;
AbstractComponent& r=Director::getInstance().getResource(REC_ID106009);
smoc_event *ev=new smoc_event(); 
r.compute(REC_ID106009,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "REC triggered!" << endl << endl;
port_rec2c_rec_sf_id106523->write(var_rec2c_rec_sf_id106523);
}
}
