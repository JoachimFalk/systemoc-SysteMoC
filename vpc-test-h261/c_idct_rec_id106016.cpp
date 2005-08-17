#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_idct_rec_id106016.h"

void c_idct_rec_id106016::c_idct_rec_id106016_process()
{
while(true){
bool var_idct2c_idct_rec_id106533 = port_idct2c_idct_rec_id106533->read();
bool var_c_idct_rec2rec_id106534 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_IDCT_REC_ID106016);
smoc_event *ev=new smoc_event(); 
r.compute(C_IDCT_REC_ID106016,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "C_IDCT_REC triggered!" << endl << endl;
port_c_idct_rec2rec_id106534->write(var_c_idct_rec2rec_id106534);
}
}
