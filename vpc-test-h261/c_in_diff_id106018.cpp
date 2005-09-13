#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_in_diff_id106018.h"

void c_in_diff_id106018::c_in_diff_id106018_process()
{
while(true){
bool var_in2c_in_diff_id106537 = port_in2c_in_diff_id106537->read();
bool var_c_in_diff2diff_id106538 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_IN_DIFF_ID106018);
smoc_event *ev=new smoc_event(); 
r.compute(C_IN_DIFF_ID106018,"",ev);
smoc_wait(*(ev));
smoc_reset(*(ev));
cout << "C_IN_DIFF triggered!" << endl << endl;
port_c_in_diff2diff_id106538->write(var_c_in_diff2diff_id106538);
}
}
