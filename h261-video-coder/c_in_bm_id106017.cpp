#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_in_bm_id106017.h"

void c_in_bm_id106017::c_in_bm_id106017_process()
{
while(true){
bool var_in2c_in_bm_id106535 = port_in2c_in_bm_id106535->read();
bool var_c_in_bm2bm_id106536 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_IN_BM_ID106017);
r.compute(C_IN_BM_ID106017);
cout << "C_IN_BM triggered!" << endl << endl;
port_c_in_bm2bm_id106536->write(var_c_in_bm2bm_id106536);
}
}
