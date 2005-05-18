#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "bm_id106001.h"

void bm_id106001::bm_id106001_process()
{
while(true){
bool var_c_in_bm2bm_id106501 = port_c_in_bm2bm_id106501->read();
bool var_c_rf_bm2bm_id106502 = port_c_rf_bm2bm_id106502->read();
bool var_bm2c_bm_lf_id106503 = 0;
AbstractComponent& r=Director::getInstance().getResource(Director::BM_ID106001);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
wait(10,SC_NS);
r.compute(Director::BM_ID106001);
cout << "BM triggered!" << endl << endl;
port_bm2c_bm_lf_id106503->write(var_bm2c_bm_lf_id106503);
}
}
