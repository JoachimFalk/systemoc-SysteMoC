#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "c_rf_bm_id106025.h"

void c_rf_bm_id106025::c_rf_bm_id106025_process()
{
while(true){
bool var_rf2c_rf_bm_id106551 = port_rf2c_rf_bm_id106551->read();
bool var_c_rf_bm2bm_id106552 = 0;
AbstractComponent& r=Director::getInstance().getResource(Director::C_RF_BM_ID106025);
r.compute(Director::C_RF_BM_ID106025);
cout << "C_RF_BM triggered!" << endl << endl;
port_c_rf_bm2bm_id106552->write(var_c_rf_bm2bm_id106552);
}
}
