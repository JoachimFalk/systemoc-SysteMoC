#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "c_in_rf_id106026.h"

void c_in_rf_id106026::c_in_rf_id106026_process()
{
while(true){
bool var_in2c_in_rf_id106554 = port_in2c_in_rf_id106554->read();
bool var_c_in_rf2rf_id106555 = 0;
AbstractComponent& r=Director::getInstance().getResource(Director::C_IN_RF_ID106026);
r.compute(Director::C_IN_RF_ID106026);
cout << "C_IN_RF triggered!" << endl << endl;
port_c_in_rf2rf_id106555->write(var_c_in_rf2rf_id106555);
}
}
