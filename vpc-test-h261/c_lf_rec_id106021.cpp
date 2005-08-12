#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_lf_rec_id106021.h"

void c_lf_rec_id106021::c_lf_rec_id106021_process()
{
while(true){
bool var_lf2c_lf_rec_id106543 = port_lf2c_lf_rec_id106543->read();
bool var_c_lf_rec2rec_id106544 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_LF_REC_ID106021);
sc_event *ev=new sc_event(); 
r.compute(C_LF_REC_ID106021,ev);
wait(*(ev));
cout << "C_LF_REC triggered!" << endl << endl;
port_c_lf_rec2rec_id106544->write(var_c_lf_rec2rec_id106544);
}
}
