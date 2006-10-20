#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "idct_id106004.h"

void idct_id106004::idct_id106004_process()
{
while(true){
bool var_c_iq_idct2idct_id106509 = port_c_iq_idct2idct_id106509->read();
bool var_idct2c_idct_rec_id106510 = 0;
AbstractComponent& r=Director::getInstance().getResource(IDCT_ID106004);
r.compute(IDCT_ID106004);
cout << "IDCT triggered!" << endl << endl;
port_idct2c_idct_rec_id106510->write(var_idct2c_idct_rec_id106510);
}
}
