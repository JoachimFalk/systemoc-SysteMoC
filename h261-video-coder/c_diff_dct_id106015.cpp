#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "c_diff_dct_id106015.h"

void c_diff_dct_id106015::c_diff_dct_id106015_process()
{
while(true){
bool var_diff2c_diff_dct_id106531 = port_diff2c_diff_dct_id106531->read();
bool var_c_diff_dct2dct_id106532 = 0;
AbstractComponent& r=Director::getInstance().getResource(C_DIFF_DCT_ID106015);
r.compute(C_DIFF_DCT_ID106015);
cout << "C_DIFF_DCT triggered!" << endl << endl;
port_c_diff_dct2dct_id106532->write(var_c_diff_dct2dct_id106532);
}
}
