#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "lf_id106007.h"

void lf_id106007::lf_id106007_process()
{
while(true){
bool var_c_bm_lf2lf_id106515 = port_c_bm_lf2lf_id106515->read();
bool var_lf2c_lf_rec_id106516 = 0;
bool var_lf2c_lf_diff_id106517 = 0;
AbstractComponent& r=Director::getInstance().getResource(Director::LF_ID106007);
r.compute(Director::LF_ID106007);
cout << "LF triggered!" << endl << endl;
port_lf2c_lf_rec_id106516->write(var_lf2c_lf_rec_id106516);
port_lf2c_lf_diff_id106517->write(var_lf2c_lf_diff_id106517);
}
}
