#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_lf_diff_id106020.h"

void c_lf_diff_id106020::c_lf_diff_id106020_process()
{
while(true){
bool var_lf2c_lf_diff_id106541 = port_lf2c_lf_diff_id106541->read();
bool var_c_lf_diff2diff_id106542 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_LF_DIFF_ID106020);
r.compute(C_LF_DIFF_ID106020);
cout << "C_LF_DIFF triggered!" << endl << endl;
port_c_lf_diff2diff_id106542->write(var_c_lf_diff2diff_id106542);
}
}
