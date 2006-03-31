/*
  Blowfish algorithm.  Written 1997 by by Paul Kocher
  (pck@netcom.com).  The Blowfish algorithm is public domain.

  Translation from C to SystemC datatypes
  2004 Andreas Schallenberg
  Carl von Ossietzky Universität Oldenburg
  Andreas.Schallenberg@Uni-Oldenburg.de
*/

#include "blowfish.hpp"

#define BLOWFISH_N 16

Blowfish::Blowfish(sc_module_name name) : CryptoAlgorithm(name), key_parts_already_processed(0)
{
      ORIG.slice[0].S[  0] = 0xD1310BA6L; ORIG.slice[0].S[  1] = 0x98DFB5ACL;
      ORIG.slice[0].S[  2] = 0x2FFD72DBL; ORIG.slice[0].S[  3] = 0xD01ADFB7L;
      ORIG.slice[0].S[  4] = 0xB8E1AFEDL; ORIG.slice[0].S[  5] = 0x6A267E96L;
      ORIG.slice[0].S[  6] = 0xBA7C9045L; ORIG.slice[0].S[  7] = 0xF12C7F99L;
      ORIG.slice[0].S[  8] = 0x24A19947L; ORIG.slice[0].S[  9] = 0xB3916CF7L; 
      ORIG.slice[0].S[ 10] = 0x0801F2E2L; ORIG.slice[0].S[ 11] = 0x858EFC16L;
      ORIG.slice[0].S[ 12] = 0x636920D8L; ORIG.slice[0].S[ 13] = 0x71574E69L;
      ORIG.slice[0].S[ 14] = 0xA458FEA3L; ORIG.slice[0].S[ 15] = 0xF4933D7EL;
      ORIG.slice[0].S[ 16] = 0x0D95748FL; ORIG.slice[0].S[ 17] = 0x728EB658L;
      ORIG.slice[0].S[ 18] = 0x718BCD58L; ORIG.slice[0].S[ 19] = 0x82154AEEL;
      ORIG.slice[0].S[ 20] = 0x7B54A41DL; ORIG.slice[0].S[ 21] = 0xC25A59B5L;
      ORIG.slice[0].S[ 22] = 0x9C30D539L; ORIG.slice[0].S[ 23] = 0x2AF26013L;
      ORIG.slice[0].S[ 24] = 0xC5D1B023L; ORIG.slice[0].S[ 25] = 0x286085F0L;
      ORIG.slice[0].S[ 26] = 0xCA417918L; ORIG.slice[0].S[ 27] = 0xB8DB38EFL;
      ORIG.slice[0].S[ 28] = 0x8E79DCB0L; ORIG.slice[0].S[ 29] = 0x603A180EL;
      ORIG.slice[0].S[ 30] = 0x6C9E0E8BL; ORIG.slice[0].S[ 31] = 0xB01E8A3EL;
      ORIG.slice[0].S[ 32] = 0xD71577C1L; ORIG.slice[0].S[ 33] = 0xBD314B27L;
      ORIG.slice[0].S[ 34] = 0x78AF2FDAL; ORIG.slice[0].S[ 35] = 0x55605C60L;
      ORIG.slice[0].S[ 36] = 0xE65525F3L; ORIG.slice[0].S[ 37] = 0xAA55AB94L;
      ORIG.slice[0].S[ 38] = 0x57489862L; ORIG.slice[0].S[ 39] = 0x63E81440L;
      ORIG.slice[0].S[ 40] = 0x55CA396AL; ORIG.slice[0].S[ 41] = 0x2AAB10B6L;
      ORIG.slice[0].S[ 42] = 0xB4CC5C34L; ORIG.slice[0].S[ 43] = 0x1141E8CEL;
      ORIG.slice[0].S[ 44] = 0xA15486AFL; ORIG.slice[0].S[ 45] = 0x7C72E993L;
      ORIG.slice[0].S[ 46] = 0xB3EE1411L; ORIG.slice[0].S[ 47] = 0x636FBC2AL;
      ORIG.slice[0].S[ 48] = 0x2BA9C55DL; ORIG.slice[0].S[ 49] = 0x741831F6L;
      ORIG.slice[0].S[ 50] = 0xCE5C3E16L; ORIG.slice[0].S[ 51] = 0x9B87931EL;
      ORIG.slice[0].S[ 52] = 0xAFD6BA33L; ORIG.slice[0].S[ 53] = 0x6C24CF5CL;
      ORIG.slice[0].S[ 54] = 0x7A325381L; ORIG.slice[0].S[ 55] = 0x28958677L;
      ORIG.slice[0].S[ 56] = 0x3B8F4898L; ORIG.slice[0].S[ 57] = 0x6B4BB9AFL;
      ORIG.slice[0].S[ 58] = 0xC4BFE81BL; ORIG.slice[0].S[ 59] = 0x66282193L;
      ORIG.slice[0].S[ 60] = 0x61D809CCL; ORIG.slice[0].S[ 61] = 0xFB21A991L;
      ORIG.slice[0].S[ 62] = 0x487CAC60L; ORIG.slice[0].S[ 63] = 0x5DEC8032L;
      ORIG.slice[0].S[ 64] = 0xEF845D5DL; ORIG.slice[0].S[ 65] = 0xE98575B1L;
      ORIG.slice[0].S[ 66] = 0xDC262302L; ORIG.slice[0].S[ 67] = 0xEB651B88L;
      ORIG.slice[0].S[ 68] = 0x23893E81L; ORIG.slice[0].S[ 69] = 0xD396ACC5L;
      ORIG.slice[0].S[ 70] = 0x0F6D6FF3L; ORIG.slice[0].S[ 71] = 0x83F44239L;
      ORIG.slice[0].S[ 72] = 0x2E0B4482L; ORIG.slice[0].S[ 73] = 0xA4842004L;
      ORIG.slice[0].S[ 74] = 0x69C8F04AL; ORIG.slice[0].S[ 75] = 0x9E1F9B5EL;
      ORIG.slice[0].S[ 76] = 0x21C66842L; ORIG.slice[0].S[ 77] = 0xF6E96C9AL;
      ORIG.slice[0].S[ 78] = 0x670C9C61L; ORIG.slice[0].S[ 79] = 0xABD388F0L;
      ORIG.slice[0].S[ 80] = 0x6A51A0D2L; ORIG.slice[0].S[ 81] = 0xD8542F68L;
      ORIG.slice[0].S[ 82] = 0x960FA728L; ORIG.slice[0].S[ 83] = 0xAB5133A3L;
      ORIG.slice[0].S[ 84] = 0x6EEF0B6CL; ORIG.slice[0].S[ 85] = 0x137A3BE4L;
      ORIG.slice[0].S[ 86] = 0xBA3BF050L; ORIG.slice[0].S[ 87] = 0x7EFB2A98L;
      ORIG.slice[0].S[ 88] = 0xA1F1651DL; ORIG.slice[0].S[ 89] = 0x39AF0176L;
      ORIG.slice[0].S[ 90] = 0x66CA593EL; ORIG.slice[0].S[ 91] = 0x82430E88L;
      ORIG.slice[0].S[ 92] = 0x8CEE8619L; ORIG.slice[0].S[ 93] = 0x456F9FB4L;
      ORIG.slice[0].S[ 94] = 0x7D84A5C3L; ORIG.slice[0].S[ 95] = 0x3B8B5EBEL;
      ORIG.slice[0].S[ 96] = 0xE06F75D8L; ORIG.slice[0].S[ 97] = 0x85C12073L;
      ORIG.slice[0].S[ 98] = 0x401A449FL; ORIG.slice[0].S[ 99] = 0x56C16AA6L;
      ORIG.slice[0].S[100] = 0x4ED3AA62L; ORIG.slice[0].S[101] = 0x363F7706L;
      ORIG.slice[0].S[102] = 0x1BFEDF72L; ORIG.slice[0].S[103] = 0x429B023DL;
      ORIG.slice[0].S[104] = 0x37D0D724L; ORIG.slice[0].S[105] = 0xD00A1248L;
      ORIG.slice[0].S[106] = 0xDB0FEAD3L; ORIG.slice[0].S[107] = 0x49F1C09BL;
      ORIG.slice[0].S[108] = 0x075372C9L; ORIG.slice[0].S[109] = 0x80991B7BL;
      ORIG.slice[0].S[110] = 0x25D479D8L; ORIG.slice[0].S[111] = 0xF6E8DEF7L;
      ORIG.slice[0].S[112] = 0xE3FE501AL; ORIG.slice[0].S[113] = 0xB6794C3BL;
      ORIG.slice[0].S[114] = 0x976CE0BDL; ORIG.slice[0].S[115] = 0x04C006BAL;
      ORIG.slice[0].S[116] = 0xC1A94FB6L; ORIG.slice[0].S[117] = 0x409F60C4L;
      ORIG.slice[0].S[118] = 0x5E5C9EC2L; ORIG.slice[0].S[119] = 0x196A2463L;
      ORIG.slice[0].S[120] = 0x68FB6FAFL; ORIG.slice[0].S[121] = 0x3E6C53B5L;
      ORIG.slice[0].S[122] = 0x1339B2EBL; ORIG.slice[0].S[123] = 0x3B52EC6FL;
      ORIG.slice[0].S[124] = 0x6DFC511FL; ORIG.slice[0].S[125] = 0x9B30952CL;
      ORIG.slice[0].S[126] = 0xCC814544L; ORIG.slice[0].S[127] = 0xAF5EBD09L;
      ORIG.slice[0].S[128] = 0xBEE3D004L; ORIG.slice[0].S[129] = 0xDE334AFDL;
      ORIG.slice[0].S[130] = 0x660F2807L; ORIG.slice[0].S[131] = 0x192E4BB3L;
      ORIG.slice[0].S[132] = 0xC0CBA857L; ORIG.slice[0].S[133] = 0x45C8740FL;
      ORIG.slice[0].S[134] = 0xD20B5F39L; ORIG.slice[0].S[135] = 0xB9D3FBDBL;
      ORIG.slice[0].S[136] = 0x5579C0BDL; ORIG.slice[0].S[137] = 0x1A60320AL;
      ORIG.slice[0].S[138] = 0xD6A100C6L; ORIG.slice[0].S[139] = 0x402C7279L;
      ORIG.slice[0].S[140] = 0x679F25FEL; ORIG.slice[0].S[141] = 0xFB1FA3CCL;
      ORIG.slice[0].S[142] = 0x8EA5E9F8L; ORIG.slice[0].S[143] = 0xDB3222F8L;
      ORIG.slice[0].S[144] = 0x3C7516DFL; ORIG.slice[0].S[145] = 0xFD616B15L;
      ORIG.slice[0].S[146] = 0x2F501EC8L; ORIG.slice[0].S[147] = 0xAD0552ABL;
      ORIG.slice[0].S[148] = 0x323DB5FAL; ORIG.slice[0].S[149] = 0xFD238760L;
      ORIG.slice[0].S[150] = 0x53317B48L; ORIG.slice[0].S[151] = 0x3E00DF82L;
      ORIG.slice[0].S[152] = 0x9E5C57BBL; ORIG.slice[0].S[153] = 0xCA6F8CA0L;
      ORIG.slice[0].S[154] = 0x1A87562EL; ORIG.slice[0].S[155] = 0xDF1769DBL;
      ORIG.slice[0].S[156] = 0xD542A8F6L; ORIG.slice[0].S[157] = 0x287EFFC3L;
      ORIG.slice[0].S[158] = 0xAC6732C6L; ORIG.slice[0].S[159] = 0x8C4F5573L;
      ORIG.slice[0].S[160] = 0x695B27B0L; ORIG.slice[0].S[161] = 0xBBCA58C8L;
      ORIG.slice[0].S[162] = 0xE1FFA35DL; ORIG.slice[0].S[163] = 0xB8F011A0L;
      ORIG.slice[0].S[164] = 0x10FA3D98L; ORIG.slice[0].S[165] = 0xFD2183B8L;
      ORIG.slice[0].S[166] = 0x4AFCB56CL; ORIG.slice[0].S[167] = 0x2DD1D35BL;
      ORIG.slice[0].S[168] = 0x9A53E479L; ORIG.slice[0].S[169] = 0xB6F84565L;
      ORIG.slice[0].S[170] = 0xD28E49BCL; ORIG.slice[0].S[171] = 0x4BFB9790L;
      ORIG.slice[0].S[172] = 0xE1DDF2DAL; ORIG.slice[0].S[173] = 0xA4CB7E33L;
      ORIG.slice[0].S[174] = 0x62FB1341L; ORIG.slice[0].S[175] = 0xCEE4C6E8L;
      ORIG.slice[0].S[176] = 0xEF20CADAL; ORIG.slice[0].S[177] = 0x36774C01L;
      ORIG.slice[0].S[178] = 0xD07E9EFEL; ORIG.slice[0].S[179] = 0x2BF11FB4L;
      ORIG.slice[0].S[180] = 0x95DBDA4DL; ORIG.slice[0].S[181] = 0xAE909198L;
      ORIG.slice[0].S[182] = 0xEAAD8E71L; ORIG.slice[0].S[183] = 0x6B93D5A0L;
      ORIG.slice[0].S[184] = 0xD08ED1D0L; ORIG.slice[0].S[185] = 0xAFC725E0L;
      ORIG.slice[0].S[186] = 0x8E3C5B2FL; ORIG.slice[0].S[187] = 0x8E7594B7L;
      ORIG.slice[0].S[188] = 0x8FF6E2FBL; ORIG.slice[0].S[189] = 0xF2122B64L;
      ORIG.slice[0].S[190] = 0x8888B812L; ORIG.slice[0].S[191] = 0x900DF01CL;
      ORIG.slice[0].S[192] = 0x4FAD5EA0L; ORIG.slice[0].S[193] = 0x688FC31CL;
      ORIG.slice[0].S[194] = 0xD1CFF191L; ORIG.slice[0].S[195] = 0xB3A8C1ADL;
      ORIG.slice[0].S[196] = 0x2F2F2218L; ORIG.slice[0].S[197] = 0xBE0E1777L;
      ORIG.slice[0].S[198] = 0xEA752DFEL; ORIG.slice[0].S[199] = 0x8B021FA1L;
      ORIG.slice[0].S[200] = 0xE5A0CC0FL; ORIG.slice[0].S[201] = 0xB56F74E8L;
      ORIG.slice[0].S[202] = 0x18ACF3D6L; ORIG.slice[0].S[203] = 0xCE89E299L;
      ORIG.slice[0].S[204] = 0xB4A84FE0L; ORIG.slice[0].S[205] = 0xFD13E0B7L;
      ORIG.slice[0].S[206] = 0x7CC43B81L; ORIG.slice[0].S[207] = 0xD2ADA8D9L;
      ORIG.slice[0].S[208] = 0x165FA266L; ORIG.slice[0].S[209] = 0x80957705L;
      ORIG.slice[0].S[210] = 0x93CC7314L; ORIG.slice[0].S[211] = 0x211A1477L;
      ORIG.slice[0].S[212] = 0xE6AD2065L; ORIG.slice[0].S[213] = 0x77B5FA86L;
      ORIG.slice[0].S[214] = 0xC75442F5L; ORIG.slice[0].S[215] = 0xFB9D35CFL;
      ORIG.slice[0].S[216] = 0xEBCDAF0CL; ORIG.slice[0].S[217] = 0x7B3E89A0L;
      ORIG.slice[0].S[218] = 0xD6411BD3L; ORIG.slice[0].S[219] = 0xAE1E7E49L;
      ORIG.slice[0].S[220] = 0x00250E2DL; ORIG.slice[0].S[221] = 0x2071B35EL;
      ORIG.slice[0].S[222] = 0x226800BBL; ORIG.slice[0].S[223] = 0x57B8E0AFL;
      ORIG.slice[0].S[224] = 0x2464369BL; ORIG.slice[0].S[225] = 0xF009B91EL;
      ORIG.slice[0].S[226] = 0x5563911DL; ORIG.slice[0].S[227] = 0x59DFA6AAL;
      ORIG.slice[0].S[228] = 0x78C14389L; ORIG.slice[0].S[229] = 0xD95A537FL;
      ORIG.slice[0].S[230] = 0x207D5BA2L; ORIG.slice[0].S[231] = 0x02E5B9C5L;
      ORIG.slice[0].S[232] = 0x83260376L; ORIG.slice[0].S[233] = 0x6295CFA9L;
      ORIG.slice[0].S[234] = 0x11C81968L; ORIG.slice[0].S[235] = 0x4E734A41L;
      ORIG.slice[0].S[236] = 0xB3472DCAL; ORIG.slice[0].S[237] = 0x7B14A94AL;
      ORIG.slice[0].S[238] = 0x1B510052L; ORIG.slice[0].S[239] = 0x9A532915L;
      ORIG.slice[0].S[240] = 0xD60F573FL; ORIG.slice[0].S[241] = 0xBC9BC6E4L;
      ORIG.slice[0].S[242] = 0x2B60A476L; ORIG.slice[0].S[243] = 0x81E67400L;
      ORIG.slice[0].S[244] = 0x08BA6FB5L; ORIG.slice[0].S[245] = 0x571BE91FL;
      ORIG.slice[0].S[246] = 0xF296EC6BL; ORIG.slice[0].S[247] = 0x2A0DD915L;
      ORIG.slice[0].S[248] = 0xB6636521L; ORIG.slice[0].S[249] = 0xE7B9F9B6L;
      ORIG.slice[0].S[250] = 0xFF34052EL; ORIG.slice[0].S[251] = 0xC5855664L;
      ORIG.slice[0].S[252] = 0x53B02D5DL; ORIG.slice[0].S[253] = 0xA99F8FA1L;
      ORIG.slice[0].S[254] = 0x08BA4799L; ORIG.slice[0].S[255] = 0x6E85076AL;
  
      ORIG.slice[1].S[  0] = 0x4B7A70E9L; ORIG.slice[1].S[  1] = 0xB5B32944L;
      ORIG.slice[1].S[  2] = 0xDB75092EL; ORIG.slice[1].S[  3] = 0xC4192623L;
      ORIG.slice[1].S[  4] = 0xAD6EA6B0L; ORIG.slice[1].S[  5] = 0x49A7DF7DL;
      ORIG.slice[1].S[  6] = 0x9CEE60B8L; ORIG.slice[1].S[  7] = 0x8FEDB266L;
      ORIG.slice[1].S[  8] = 0xECAA8C71L; ORIG.slice[1].S[  9] = 0x699A17FFL;
      ORIG.slice[1].S[ 10] = 0x5664526CL; ORIG.slice[1].S[ 11] = 0xC2B19EE1L;
      ORIG.slice[1].S[ 12] = 0x193602A5L; ORIG.slice[1].S[ 13] = 0x75094C29L;
      ORIG.slice[1].S[ 14] = 0xA0591340L; ORIG.slice[1].S[ 15] = 0xE4183A3EL;
      ORIG.slice[1].S[ 16] = 0x3F54989AL; ORIG.slice[1].S[ 17] = 0x5B429D65L;
      ORIG.slice[1].S[ 18] = 0x6B8FE4D6L; ORIG.slice[1].S[ 19] = 0x99F73FD6L;
      ORIG.slice[1].S[ 20] = 0xA1D29C07L; ORIG.slice[1].S[ 21] = 0xEFE830F5L;
      ORIG.slice[1].S[ 22] = 0x4D2D38E6L; ORIG.slice[1].S[ 23] = 0xF0255DC1L;
      ORIG.slice[1].S[ 24] = 0x4CDD2086L; ORIG.slice[1].S[ 25] = 0x8470EB26L;
      ORIG.slice[1].S[ 26] = 0x6382E9C6L; ORIG.slice[1].S[ 27] = 0x021ECC5EL;
      ORIG.slice[1].S[ 28] = 0x09686B3FL; ORIG.slice[1].S[ 29] = 0x3EBAEFC9L;
      ORIG.slice[1].S[ 30] = 0x3C971814L; ORIG.slice[1].S[ 31] = 0x6B6A70A1L;
      ORIG.slice[1].S[ 32] = 0x687F3584L; ORIG.slice[1].S[ 33] = 0x52A0E286L;
      ORIG.slice[1].S[ 34] = 0xB79C5305L; ORIG.slice[1].S[ 35] = 0xAA500737L;
      ORIG.slice[1].S[ 36] = 0x3E07841CL; ORIG.slice[1].S[ 37] = 0x7FDEAE5CL;
      ORIG.slice[1].S[ 38] = 0x8E7D44ECL; ORIG.slice[1].S[ 39] = 0x5716F2B8L;
      ORIG.slice[1].S[ 40] = 0xB03ADA37L; ORIG.slice[1].S[ 41] = 0xF0500C0DL;
      ORIG.slice[1].S[ 42] = 0xF01C1F04L; ORIG.slice[1].S[ 43] = 0x0200B3FFL;
      ORIG.slice[1].S[ 44] = 0xAE0CF51AL; ORIG.slice[1].S[ 45] = 0x3CB574B2L;
      ORIG.slice[1].S[ 46] = 0x25837A58L; ORIG.slice[1].S[ 47] = 0xDC0921BDL;
      ORIG.slice[1].S[ 48] = 0xD19113F9L; ORIG.slice[1].S[ 49] = 0x7CA92FF6L;
      ORIG.slice[1].S[ 50] = 0x94324773L; ORIG.slice[1].S[ 51] = 0x22F54701L;
      ORIG.slice[1].S[ 52] = 0x3AE5E581L; ORIG.slice[1].S[ 53] = 0x37C2DADCL;
      ORIG.slice[1].S[ 54] = 0xC8B57634L; ORIG.slice[1].S[ 55] = 0x9AF3DDA7L;
      ORIG.slice[1].S[ 56] = 0xA9446146L; ORIG.slice[1].S[ 57] = 0x0FD0030EL;
      ORIG.slice[1].S[ 58] = 0xECC8C73EL; ORIG.slice[1].S[ 59] = 0xA4751E41L;
      ORIG.slice[1].S[ 60] = 0xE238CD99L; ORIG.slice[1].S[ 61] = 0x3BEA0E2FL;
      ORIG.slice[1].S[ 62] = 0x3280BBA1L; ORIG.slice[1].S[ 63] = 0x183EB331L;
      ORIG.slice[1].S[ 64] = 0x4E548B38L; ORIG.slice[1].S[ 65] = 0x4F6DB908L;
      ORIG.slice[1].S[ 66] = 0x6F420D03L; ORIG.slice[1].S[ 67] = 0xF60A04BFL;
      ORIG.slice[1].S[ 68] = 0x2CB81290L; ORIG.slice[1].S[ 69] = 0x24977C79L;
      ORIG.slice[1].S[ 70] = 0x5679B072L; ORIG.slice[1].S[ 71] = 0xBCAF89AFL;
      ORIG.slice[1].S[ 72] = 0xDE9A771FL; ORIG.slice[1].S[ 73] = 0xD9930810L;
      ORIG.slice[1].S[ 74] = 0xB38BAE12L; ORIG.slice[1].S[ 75] = 0xDCCF3F2EL;
      ORIG.slice[1].S[ 76] = 0x5512721FL; ORIG.slice[1].S[ 77] = 0x2E6B7124L;
      ORIG.slice[1].S[ 78] = 0x501ADDE6L; ORIG.slice[1].S[ 79] = 0x9F84CD87L;
      ORIG.slice[1].S[ 80] = 0x7A584718L; ORIG.slice[1].S[ 81] = 0x7408DA17L;
      ORIG.slice[1].S[ 82] = 0xBC9F9ABCL; ORIG.slice[1].S[ 83] = 0xE94B7D8CL;
      ORIG.slice[1].S[ 84] = 0xEC7AEC3AL; ORIG.slice[1].S[ 85] = 0xDB851DFAL;
      ORIG.slice[1].S[ 86] = 0x63094366L; ORIG.slice[1].S[ 87] = 0xC464C3D2L;
      ORIG.slice[1].S[ 88] = 0xEF1C1847L; ORIG.slice[1].S[ 89] = 0x3215D908L;
      ORIG.slice[1].S[ 90] = 0xDD433B37L; ORIG.slice[1].S[ 91] = 0x24C2BA16L;
      ORIG.slice[1].S[ 92] = 0x12A14D43L; ORIG.slice[1].S[ 93] = 0x2A65C451L;
      ORIG.slice[1].S[ 94] = 0x50940002L; ORIG.slice[1].S[ 95] = 0x133AE4DDL;
      ORIG.slice[1].S[ 96] = 0x71DFF89EL; ORIG.slice[1].S[ 97] = 0x10314E55L;
      ORIG.slice[1].S[ 98] = 0x81AC77D6L; ORIG.slice[1].S[ 99] = 0x5F11199BL;
      ORIG.slice[1].S[100] = 0x043556F1L; ORIG.slice[1].S[101] = 0xD7A3C76BL;
      ORIG.slice[1].S[102] = 0x3C11183BL; ORIG.slice[1].S[103] = 0x5924A509L;
      ORIG.slice[1].S[104] = 0xF28FE6EDL; ORIG.slice[1].S[105] = 0x97F1FBFAL;
      ORIG.slice[1].S[106] = 0x9EBABF2CL; ORIG.slice[1].S[107] = 0x1E153C6EL;
      ORIG.slice[1].S[108] = 0x86E34570L; ORIG.slice[1].S[109] = 0xEAE96FB1L;
      ORIG.slice[1].S[110] = 0x860E5E0AL; ORIG.slice[1].S[111] = 0x5A3E2AB3L;
      ORIG.slice[1].S[112] = 0x771FE71CL; ORIG.slice[1].S[113] = 0x4E3D06FAL;
      ORIG.slice[1].S[114] = 0x2965DCB9L; ORIG.slice[1].S[115] = 0x99E71D0FL;
      ORIG.slice[1].S[116] = 0x803E89D6L; ORIG.slice[1].S[117] = 0x5266C825L;
      ORIG.slice[1].S[118] = 0x2E4CC978L; ORIG.slice[1].S[119] = 0x9C10B36AL;
      ORIG.slice[1].S[120] = 0xC6150EBAL; ORIG.slice[1].S[121] = 0x94E2EA78L;
      ORIG.slice[1].S[122] = 0xA5FC3C53L; ORIG.slice[1].S[123] = 0x1E0A2DF4L;
      ORIG.slice[1].S[124] = 0xF2F74EA7L; ORIG.slice[1].S[125] = 0x361D2B3DL;
      ORIG.slice[1].S[126] = 0x1939260FL; ORIG.slice[1].S[127] = 0x19C27960L;
      ORIG.slice[1].S[128] = 0x5223A708L; ORIG.slice[1].S[129] = 0xF71312B6L;
      ORIG.slice[1].S[130] = 0xEBADFE6EL; ORIG.slice[1].S[131] = 0xEAC31F66L;
      ORIG.slice[1].S[132] = 0xE3BC4595L; ORIG.slice[1].S[133] = 0xA67BC883L;
      ORIG.slice[1].S[134] = 0xB17F37D1L; ORIG.slice[1].S[135] = 0x018CFF28L;
      ORIG.slice[1].S[136] = 0xC332DDEFL; ORIG.slice[1].S[137] = 0xBE6C5AA5L;
      ORIG.slice[1].S[138] = 0x65582185L; ORIG.slice[1].S[139] = 0x68AB9802L;
      ORIG.slice[1].S[140] = 0xEECEA50FL; ORIG.slice[1].S[141] = 0xDB2F953BL;
      ORIG.slice[1].S[142] = 0x2AEF7DADL; ORIG.slice[1].S[143] = 0x5B6E2F84L;
      ORIG.slice[1].S[144] = 0x1521B628L; ORIG.slice[1].S[145] = 0x29076170L;
      ORIG.slice[1].S[146] = 0xECDD4775L; ORIG.slice[1].S[147] = 0x619F1510L;
      ORIG.slice[1].S[148] = 0x13CCA830L; ORIG.slice[1].S[149] = 0xEB61BD96L;
      ORIG.slice[1].S[150] = 0x0334FE1EL; ORIG.slice[1].S[151] = 0xAA0363CFL;
      ORIG.slice[1].S[152] = 0xB5735C90L; ORIG.slice[1].S[153] = 0x4C70A239L;
      ORIG.slice[1].S[154] = 0xD59E9E0BL; ORIG.slice[1].S[155] = 0xCBAADE14L;
      ORIG.slice[1].S[156] = 0xEECC86BCL; ORIG.slice[1].S[157] = 0x60622CA7L;
      ORIG.slice[1].S[158] = 0x9CAB5CABL; ORIG.slice[1].S[159] = 0xB2F3846EL;
      ORIG.slice[1].S[160] = 0x648B1EAFL; ORIG.slice[1].S[161] = 0x19BDF0CAL;
      ORIG.slice[1].S[162] = 0xA02369B9L; ORIG.slice[1].S[163] = 0x655ABB50L;
      ORIG.slice[1].S[164] = 0x40685A32L; ORIG.slice[1].S[165] = 0x3C2AB4B3L;
      ORIG.slice[1].S[166] = 0x319EE9D5L; ORIG.slice[1].S[167] = 0xC021B8F7L;
      ORIG.slice[1].S[168] = 0x9B540B19L; ORIG.slice[1].S[169] = 0x875FA099L;
      ORIG.slice[1].S[170] = 0x95F7997EL; ORIG.slice[1].S[171] = 0x623D7DA8L;
      ORIG.slice[1].S[172] = 0xF837889AL; ORIG.slice[1].S[173] = 0x97E32D77L;
      ORIG.slice[1].S[174] = 0x11ED935FL; ORIG.slice[1].S[175] = 0x16681281L;
      ORIG.slice[1].S[176] = 0x0E358829L; ORIG.slice[1].S[177] = 0xC7E61FD6L;
      ORIG.slice[1].S[178] = 0x96DEDFA1L; ORIG.slice[1].S[179] = 0x7858BA99L;
      ORIG.slice[1].S[180] = 0x57F584A5L; ORIG.slice[1].S[181] = 0x1B227263L;
      ORIG.slice[1].S[182] = 0x9B83C3FFL; ORIG.slice[1].S[183] = 0x1AC24696L;
      ORIG.slice[1].S[184] = 0xCDB30AEBL; ORIG.slice[1].S[185] = 0x532E3054L;
      ORIG.slice[1].S[186] = 0x8FD948E4L; ORIG.slice[1].S[187] = 0x6DBC3128L;
      ORIG.slice[1].S[188] = 0x58EBF2EFL; ORIG.slice[1].S[189] = 0x34C6FFEAL;
      ORIG.slice[1].S[190] = 0xFE28ED61L; ORIG.slice[1].S[191] = 0xEE7C3C73L;
      ORIG.slice[1].S[192] = 0x5D4A14D9L; ORIG.slice[1].S[193] = 0xE864B7E3L;
      ORIG.slice[1].S[194] = 0x42105D14L; ORIG.slice[1].S[195] = 0x203E13E0L;
      ORIG.slice[1].S[196] = 0x45EEE2B6L; ORIG.slice[1].S[197] = 0xA3AAABEAL;
      ORIG.slice[1].S[198] = 0xDB6C4F15L; ORIG.slice[1].S[199] = 0xFACB4FD0L;
      ORIG.slice[1].S[200] = 0xC742F442L; ORIG.slice[1].S[201] = 0xEF6ABBB5L;
      ORIG.slice[1].S[202] = 0x654F3B1DL; ORIG.slice[1].S[203] = 0x41CD2105L;
      ORIG.slice[1].S[204] = 0xD81E799EL; ORIG.slice[1].S[205] = 0x86854DC7L;
      ORIG.slice[1].S[206] = 0xE44B476AL; ORIG.slice[1].S[207] = 0x3D816250L;
      ORIG.slice[1].S[208] = 0xCF62A1F2L; ORIG.slice[1].S[209] = 0x5B8D2646L;
      ORIG.slice[1].S[210] = 0xFC8883A0L; ORIG.slice[1].S[211] = 0xC1C7B6A3L;
      ORIG.slice[1].S[212] = 0x7F1524C3L; ORIG.slice[1].S[213] = 0x69CB7492L;
      ORIG.slice[1].S[214] = 0x47848A0BL; ORIG.slice[1].S[215] = 0x5692B285L;
      ORIG.slice[1].S[216] = 0x095BBF00L; ORIG.slice[1].S[217] = 0xAD19489DL;
      ORIG.slice[1].S[218] = 0x1462B174L; ORIG.slice[1].S[219] = 0x23820E00L;
      ORIG.slice[1].S[220] = 0x58428D2AL; ORIG.slice[1].S[221] = 0x0C55F5EAL;
      ORIG.slice[1].S[222] = 0x1DADF43EL; ORIG.slice[1].S[223] = 0x233F7061L;
      ORIG.slice[1].S[224] = 0x3372F092L; ORIG.slice[1].S[225] = 0x8D937E41L;
      ORIG.slice[1].S[226] = 0xD65FECF1L; ORIG.slice[1].S[227] = 0x6C223BDBL;
      ORIG.slice[1].S[228] = 0x7CDE3759L; ORIG.slice[1].S[229] = 0xCBEE7460L;
      ORIG.slice[1].S[230] = 0x4085F2A7L; ORIG.slice[1].S[231] = 0xCE77326EL;
      ORIG.slice[1].S[232] = 0xA6078084L; ORIG.slice[1].S[233] = 0x19F8509EL;
      ORIG.slice[1].S[234] = 0xE8EFD855L; ORIG.slice[1].S[235] = 0x61D99735L;
      ORIG.slice[1].S[236] = 0xA969A7AAL; ORIG.slice[1].S[237] = 0xC50C06C2L;
      ORIG.slice[1].S[238] = 0x5A04ABFCL; ORIG.slice[1].S[239] = 0x800BCADCL;
      ORIG.slice[1].S[240] = 0x9E447A2EL; ORIG.slice[1].S[241] = 0xC3453484L;
      ORIG.slice[1].S[242] = 0xFDD56705L; ORIG.slice[1].S[243] = 0x0E1E9EC9L;
      ORIG.slice[1].S[244] = 0xDB73DBD3L; ORIG.slice[1].S[245] = 0x105588CDL;
      ORIG.slice[1].S[246] = 0x675FDA79L; ORIG.slice[1].S[247] = 0xE3674340L;
      ORIG.slice[1].S[248] = 0xC5C43465L; ORIG.slice[1].S[249] = 0x713E38D8L;
      ORIG.slice[1].S[250] = 0x3D28F89EL; ORIG.slice[1].S[251] = 0xF16DFF20L;
      ORIG.slice[1].S[252] = 0x153E21E7L; ORIG.slice[1].S[253] = 0x8FB03D4AL;
      ORIG.slice[1].S[254] = 0xE6E39F2BL; ORIG.slice[1].S[255] = 0xDB83ADF7L;
  
      ORIG.slice[2].S[  0] = 0xE93D5A68L; ORIG.slice[2].S[  1] = 0x948140F7L;
      ORIG.slice[2].S[  2] = 0xF64C261CL; ORIG.slice[2].S[  3] = 0x94692934L;
      ORIG.slice[2].S[  4] = 0x411520F7L; ORIG.slice[2].S[  5] = 0x7602D4F7L;
      ORIG.slice[2].S[  6] = 0xBCF46B2EL; ORIG.slice[2].S[  7] = 0xD4A20068L;
      ORIG.slice[2].S[  8] = 0xD4082471L; ORIG.slice[2].S[  9] = 0x3320F46AL;
      ORIG.slice[2].S[ 10] = 0x43B7D4B7L; ORIG.slice[2].S[ 11] = 0x500061AFL;
      ORIG.slice[2].S[ 12] = 0x1E39F62EL; ORIG.slice[2].S[ 13] = 0x97244546L;
      ORIG.slice[2].S[ 14] = 0x14214F74L; ORIG.slice[2].S[ 15] = 0xBF8B8840L;
      ORIG.slice[2].S[ 16] = 0x4D95FC1DL; ORIG.slice[2].S[ 17] = 0x96B591AFL;
      ORIG.slice[2].S[ 18] = 0x70F4DDD3L; ORIG.slice[2].S[ 19] = 0x66A02F45L;
      ORIG.slice[2].S[ 20] = 0xBFBC09ECL; ORIG.slice[2].S[ 21] = 0x03BD9785L;
      ORIG.slice[2].S[ 22] = 0x7FAC6DD0L; ORIG.slice[2].S[ 23] = 0x31CB8504L;
      ORIG.slice[2].S[ 24] = 0x96EB27B3L; ORIG.slice[2].S[ 25] = 0x55FD3941L;
      ORIG.slice[2].S[ 26] = 0xDA2547E6L; ORIG.slice[2].S[ 27] = 0xABCA0A9AL;
      ORIG.slice[2].S[ 28] = 0x28507825L; ORIG.slice[2].S[ 29] = 0x530429F4L;
      ORIG.slice[2].S[ 30] = 0x0A2C86DAL; ORIG.slice[2].S[ 31] = 0xE9B66DFBL;
      ORIG.slice[2].S[ 32] = 0x68DC1462L; ORIG.slice[2].S[ 33] = 0xD7486900L;
      ORIG.slice[2].S[ 34] = 0x680EC0A4L; ORIG.slice[2].S[ 35] = 0x27A18DEEL;
      ORIG.slice[2].S[ 36] = 0x4F3FFEA2L; ORIG.slice[2].S[ 37] = 0xE887AD8CL;
      ORIG.slice[2].S[ 38] = 0xB58CE006L; ORIG.slice[2].S[ 39] = 0x7AF4D6B6L;
      ORIG.slice[2].S[ 40] = 0xAACE1E7CL; ORIG.slice[2].S[ 41] = 0xD3375FECL;
      ORIG.slice[2].S[ 42] = 0xCE78A399L; ORIG.slice[2].S[ 43] = 0x406B2A42L;
      ORIG.slice[2].S[ 44] = 0x20FE9E35L; ORIG.slice[2].S[ 45] = 0xD9F385B9L;
      ORIG.slice[2].S[ 46] = 0xEE39D7ABL; ORIG.slice[2].S[ 47] = 0x3B124E8BL;
      ORIG.slice[2].S[ 48] = 0x1DC9FAF7L; ORIG.slice[2].S[ 49] = 0x4B6D1856L;
      ORIG.slice[2].S[ 50] = 0x26A36631L; ORIG.slice[2].S[ 51] = 0xEAE397B2L;
      ORIG.slice[2].S[ 52] = 0x3A6EFA74L; ORIG.slice[2].S[ 53] = 0xDD5B4332L;
      ORIG.slice[2].S[ 54] = 0x6841E7F7L; ORIG.slice[2].S[ 55] = 0xCA7820FBL;
      ORIG.slice[2].S[ 56] = 0xFB0AF54EL; ORIG.slice[2].S[ 57] = 0xD8FEB397L;
      ORIG.slice[2].S[ 58] = 0x454056ACL; ORIG.slice[2].S[ 59] = 0xBA489527L;
      ORIG.slice[2].S[ 60] = 0x55533A3AL; ORIG.slice[2].S[ 61] = 0x20838D87L;
      ORIG.slice[2].S[ 62] = 0xFE6BA9B7L; ORIG.slice[2].S[ 63] = 0xD096954BL;
      ORIG.slice[2].S[ 64] = 0x55A867BCL; ORIG.slice[2].S[ 65] = 0xA1159A58L;
      ORIG.slice[2].S[ 66] = 0xCCA92963L; ORIG.slice[2].S[ 67] = 0x99E1DB33L;
      ORIG.slice[2].S[ 68] = 0xA62A4A56L; ORIG.slice[2].S[ 69] = 0x3F3125F9L;
      ORIG.slice[2].S[ 70] = 0x5EF47E1CL; ORIG.slice[2].S[ 71] = 0x9029317CL;
      ORIG.slice[2].S[ 72] = 0xFDF8E802L; ORIG.slice[2].S[ 73] = 0x04272F70L;
      ORIG.slice[2].S[ 74] = 0x80BB155CL; ORIG.slice[2].S[ 75] = 0x05282CE3L;
      ORIG.slice[2].S[ 76] = 0x95C11548L; ORIG.slice[2].S[ 77] = 0xE4C66D22L;
      ORIG.slice[2].S[ 78] = 0x48C1133FL; ORIG.slice[2].S[ 79] = 0xC70F86DCL;
      ORIG.slice[2].S[ 80] = 0x07F9C9EEL; ORIG.slice[2].S[ 81] = 0x41041F0FL;
      ORIG.slice[2].S[ 82] = 0x404779A4L; ORIG.slice[2].S[ 83] = 0x5D886E17L;
      ORIG.slice[2].S[ 84] = 0x325F51EBL; ORIG.slice[2].S[ 85] = 0xD59BC0D1L;
      ORIG.slice[2].S[ 86] = 0xF2BCC18FL; ORIG.slice[2].S[ 87] = 0x41113564L;
      ORIG.slice[2].S[ 88] = 0x257B7834L; ORIG.slice[2].S[ 89] = 0x602A9C60L;
      ORIG.slice[2].S[ 90] = 0xDFF8E8A3L; ORIG.slice[2].S[ 91] = 0x1F636C1BL;
      ORIG.slice[2].S[ 92] = 0x0E12B4C2L; ORIG.slice[2].S[ 93] = 0x02E1329EL;
      ORIG.slice[2].S[ 94] = 0xAF664FD1L; ORIG.slice[2].S[ 95] = 0xCAD18115L;
      ORIG.slice[2].S[ 96] = 0x6B2395E0L; ORIG.slice[2].S[ 97] = 0x333E92E1L;
      ORIG.slice[2].S[ 98] = 0x3B240B62L; ORIG.slice[2].S[ 99] = 0xEEBEB922L;
      ORIG.slice[2].S[100] = 0x85B2A20EL; ORIG.slice[2].S[101] = 0xE6BA0D99L;
      ORIG.slice[2].S[102] = 0xDE720C8CL; ORIG.slice[2].S[103] = 0x2DA2F728L;
      ORIG.slice[2].S[104] = 0xD0127845L; ORIG.slice[2].S[105] = 0x95B794FDL;
      ORIG.slice[2].S[106] = 0x647D0862L; ORIG.slice[2].S[107] = 0xE7CCF5F0L;
      ORIG.slice[2].S[108] = 0x5449A36FL; ORIG.slice[2].S[109] = 0x877D48FAL;
      ORIG.slice[2].S[110] = 0xC39DFD27L; ORIG.slice[2].S[111] = 0xF33E8D1EL;
      ORIG.slice[2].S[112] = 0x0A476341L; ORIG.slice[2].S[113] = 0x992EFF74L;
      ORIG.slice[2].S[114] = 0x3A6F6EABL; ORIG.slice[2].S[115] = 0xF4F8FD37L;
      ORIG.slice[2].S[116] = 0xA812DC60L; ORIG.slice[2].S[117] = 0xA1EBDDF8L;
      ORIG.slice[2].S[118] = 0x991BE14CL; ORIG.slice[2].S[119] = 0xDB6E6B0DL;
      ORIG.slice[2].S[120] = 0xC67B5510L; ORIG.slice[2].S[121] = 0x6D672C37L;
      ORIG.slice[2].S[122] = 0x2765D43BL; ORIG.slice[2].S[123] = 0xDCD0E804L;
      ORIG.slice[2].S[124] = 0xF1290DC7L; ORIG.slice[2].S[125] = 0xCC00FFA3L;
      ORIG.slice[2].S[126] = 0xB5390F92L; ORIG.slice[2].S[127] = 0x690FED0BL;
      ORIG.slice[2].S[128] = 0x667B9FFBL; ORIG.slice[2].S[129] = 0xCEDB7D9CL;
      ORIG.slice[2].S[130] = 0xA091CF0BL; ORIG.slice[2].S[131] = 0xD9155EA3L;
      ORIG.slice[2].S[132] = 0xBB132F88L; ORIG.slice[2].S[133] = 0x515BAD24L;
      ORIG.slice[2].S[134] = 0x7B9479BFL; ORIG.slice[2].S[135] = 0x763BD6EBL;
      ORIG.slice[2].S[136] = 0x37392EB3L; ORIG.slice[2].S[137] = 0xCC115979L;
      ORIG.slice[2].S[138] = 0x8026E297L; ORIG.slice[2].S[139] = 0xF42E312DL;
      ORIG.slice[2].S[140] = 0x6842ADA7L; ORIG.slice[2].S[141] = 0xC66A2B3BL;
      ORIG.slice[2].S[142] = 0x12754CCCL; ORIG.slice[2].S[143] = 0x782EF11CL;
      ORIG.slice[2].S[144] = 0x6A124237L; ORIG.slice[2].S[145] = 0xB79251E7L;
      ORIG.slice[2].S[146] = 0x06A1BBE6L; ORIG.slice[2].S[147] = 0x4BFB6350L;
      ORIG.slice[2].S[148] = 0x1A6B1018L; ORIG.slice[2].S[149] = 0x11CAEDFAL;
      ORIG.slice[2].S[150] = 0x3D25BDD8L; ORIG.slice[2].S[151] = 0xE2E1C3C9L;
      ORIG.slice[2].S[152] = 0x44421659L; ORIG.slice[2].S[153] = 0x0A121386L;
      ORIG.slice[2].S[154] = 0xD90CEC6EL; ORIG.slice[2].S[155] = 0xD5ABEA2AL;
      ORIG.slice[2].S[156] = 0x64AF674EL; ORIG.slice[2].S[157] = 0xDA86A85FL;
      ORIG.slice[2].S[158] = 0xBEBFE988L; ORIG.slice[2].S[159] = 0x64E4C3FEL;
      ORIG.slice[2].S[160] = 0x9DBC8057L; ORIG.slice[2].S[161] = 0xF0F7C086L;
      ORIG.slice[2].S[162] = 0x60787BF8L; ORIG.slice[2].S[163] = 0x6003604DL;
      ORIG.slice[2].S[164] = 0xD1FD8346L; ORIG.slice[2].S[165] = 0xF6381FB0L;
      ORIG.slice[2].S[166] = 0x7745AE04L; ORIG.slice[2].S[167] = 0xD736FCCCL;
      ORIG.slice[2].S[168] = 0x83426B33L; ORIG.slice[2].S[169] = 0xF01EAB71L;
      ORIG.slice[2].S[170] = 0xB0804187L; ORIG.slice[2].S[171] = 0x3C005E5FL;
      ORIG.slice[2].S[172] = 0x77A057BEL; ORIG.slice[2].S[173] = 0xBDE8AE24L;
      ORIG.slice[2].S[174] = 0x55464299L; ORIG.slice[2].S[175] = 0xBF582E61L;
      ORIG.slice[2].S[176] = 0x4E58F48FL; ORIG.slice[2].S[177] = 0xF2DDFDA2L;
      ORIG.slice[2].S[178] = 0xF474EF38L; ORIG.slice[2].S[179] = 0x8789BDC2L;
      ORIG.slice[2].S[180] = 0x5366F9C3L; ORIG.slice[2].S[181] = 0xC8B38E74L;
      ORIG.slice[2].S[182] = 0xB475F255L; ORIG.slice[2].S[183] = 0x46FCD9B9L;
      ORIG.slice[2].S[184] = 0x7AEB2661L; ORIG.slice[2].S[185] = 0x8B1DDF84L;
      ORIG.slice[2].S[186] = 0x846A0E79L; ORIG.slice[2].S[187] = 0x915F95E2L;
      ORIG.slice[2].S[188] = 0x466E598EL; ORIG.slice[2].S[189] = 0x20B45770L;
      ORIG.slice[2].S[190] = 0x8CD55591L; ORIG.slice[2].S[191] = 0xC902DE4CL;
      ORIG.slice[2].S[192] = 0xB90BACE1L; ORIG.slice[2].S[193] = 0xBB8205D0L;
      ORIG.slice[2].S[194] = 0x11A86248L; ORIG.slice[2].S[195] = 0x7574A99EL;
      ORIG.slice[2].S[196] = 0xB77F19B6L; ORIG.slice[2].S[197] = 0xE0A9DC09L;
      ORIG.slice[2].S[198] = 0x662D09A1L; ORIG.slice[2].S[199] = 0xC4324633L;
      ORIG.slice[2].S[200] = 0xE85A1F02L; ORIG.slice[2].S[201] = 0x09F0BE8CL;
      ORIG.slice[2].S[202] = 0x4A99A025L; ORIG.slice[2].S[203] = 0x1D6EFE10L;
      ORIG.slice[2].S[204] = 0x1AB93D1DL; ORIG.slice[2].S[205] = 0x0BA5A4DFL;
      ORIG.slice[2].S[206] = 0xA186F20FL; ORIG.slice[2].S[207] = 0x2868F169L;
      ORIG.slice[2].S[208] = 0xDCB7DA83L; ORIG.slice[2].S[209] = 0x573906FEL;
      ORIG.slice[2].S[210] = 0xA1E2CE9BL; ORIG.slice[2].S[211] = 0x4FCD7F52L;
      ORIG.slice[2].S[212] = 0x50115E01L; ORIG.slice[2].S[213] = 0xA70683FAL;
      ORIG.slice[2].S[214] = 0xA002B5C4L; ORIG.slice[2].S[215] = 0x0DE6D027L;
      ORIG.slice[2].S[216] = 0x9AF88C27L; ORIG.slice[2].S[217] = 0x773F8641L;
      ORIG.slice[2].S[218] = 0xC3604C06L; ORIG.slice[2].S[219] = 0x61A806B5L;
      ORIG.slice[2].S[220] = 0xF0177A28L; ORIG.slice[2].S[221] = 0xC0F586E0L;
      ORIG.slice[2].S[222] = 0x006058AAL; ORIG.slice[2].S[223] = 0x30DC7D62L;
      ORIG.slice[2].S[224] = 0x11E69ED7L; ORIG.slice[2].S[225] = 0x2338EA63L;
      ORIG.slice[2].S[226] = 0x53C2DD94L; ORIG.slice[2].S[227] = 0xC2C21634L;
      ORIG.slice[2].S[228] = 0xBBCBEE56L; ORIG.slice[2].S[229] = 0x90BCB6DEL;
      ORIG.slice[2].S[230] = 0xEBFC7DA1L; ORIG.slice[2].S[231] = 0xCE591D76L;
      ORIG.slice[2].S[232] = 0x6F05E409L; ORIG.slice[2].S[233] = 0x4B7C0188L;
      ORIG.slice[2].S[234] = 0x39720A3DL; ORIG.slice[2].S[235] = 0x7C927C24L;
      ORIG.slice[2].S[236] = 0x86E3725FL; ORIG.slice[2].S[237] = 0x724D9DB9L;
      ORIG.slice[2].S[238] = 0x1AC15BB4L; ORIG.slice[2].S[239] = 0xD39EB8FCL;
      ORIG.slice[2].S[240] = 0xED545578L; ORIG.slice[2].S[241] = 0x08FCA5B5L;
      ORIG.slice[2].S[242] = 0xD83D7CD3L; ORIG.slice[2].S[243] = 0x4DAD0FC4L;
      ORIG.slice[2].S[244] = 0x1E50EF5EL; ORIG.slice[2].S[245] = 0xB161E6F8L;
      ORIG.slice[2].S[246] = 0xA28514D9L; ORIG.slice[2].S[247] = 0x6C51133CL;
      ORIG.slice[2].S[248] = 0x6FD5C7E7L; ORIG.slice[2].S[249] = 0x56E14EC4L;
      ORIG.slice[2].S[250] = 0x362ABFCEL; ORIG.slice[2].S[251] = 0xDDC6C837L;
      ORIG.slice[2].S[252] = 0xD79A3234L; ORIG.slice[2].S[253] = 0x92638212L;
      ORIG.slice[2].S[254] = 0x670EFA8EL; ORIG.slice[2].S[255] = 0x406000E0L;
  
      ORIG.slice[3].S[  0] = 0x3A39CE37L; ORIG.slice[3].S[  1] = 0xD3FAF5CFL;
      ORIG.slice[3].S[  2] = 0xABC27737L; ORIG.slice[3].S[  3] = 0x5AC52D1BL;
      ORIG.slice[3].S[  4] = 0x5CB0679EL; ORIG.slice[3].S[  5] = 0x4FA33742L;
      ORIG.slice[3].S[  6] = 0xD3822740L; ORIG.slice[3].S[  7] = 0x99BC9BBEL;
      ORIG.slice[3].S[  8] = 0xD5118E9DL; ORIG.slice[3].S[  9] = 0xBF0F7315L;
      ORIG.slice[3].S[ 10] = 0xD62D1C7EL; ORIG.slice[3].S[ 11] = 0xC700C47BL;
      ORIG.slice[3].S[ 12] = 0xB78C1B6BL; ORIG.slice[3].S[ 13] = 0x21A19045L;
      ORIG.slice[3].S[ 14] = 0xB26EB1BEL; ORIG.slice[3].S[ 15] = 0x6A366EB4L;
      ORIG.slice[3].S[ 16] = 0x5748AB2FL; ORIG.slice[3].S[ 17] = 0xBC946E79L;
      ORIG.slice[3].S[ 18] = 0xC6A376D2L; ORIG.slice[3].S[ 19] = 0x6549C2C8L;
      ORIG.slice[3].S[ 20] = 0x530FF8EEL; ORIG.slice[3].S[ 21] = 0x468DDE7DL;
      ORIG.slice[3].S[ 22] = 0xD5730A1DL; ORIG.slice[3].S[ 23] = 0x4CD04DC6L;
      ORIG.slice[3].S[ 24] = 0x2939BBDBL; ORIG.slice[3].S[ 25] = 0xA9BA4650L;
      ORIG.slice[3].S[ 26] = 0xAC9526E8L; ORIG.slice[3].S[ 27] = 0xBE5EE304L;
      ORIG.slice[3].S[ 28] = 0xA1FAD5F0L; ORIG.slice[3].S[ 29] = 0x6A2D519AL;
      ORIG.slice[3].S[ 30] = 0x63EF8CE2L; ORIG.slice[3].S[ 31] = 0x9A86EE22L;
      ORIG.slice[3].S[ 32] = 0xC089C2B8L; ORIG.slice[3].S[ 33] = 0x43242EF6L;
      ORIG.slice[3].S[ 34] = 0xA51E03AAL; ORIG.slice[3].S[ 35] = 0x9CF2D0A4L;
      ORIG.slice[3].S[ 36] = 0x83C061BAL; ORIG.slice[3].S[ 37] = 0x9BE96A4DL;
      ORIG.slice[3].S[ 38] = 0x8FE51550L; ORIG.slice[3].S[ 39] = 0xBA645BD6L;
      ORIG.slice[3].S[ 40] = 0x2826A2F9L; ORIG.slice[3].S[ 41] = 0xA73A3AE1L;
      ORIG.slice[3].S[ 42] = 0x4BA99586L; ORIG.slice[3].S[ 43] = 0xEF5562E9L;
      ORIG.slice[3].S[ 44] = 0xC72FEFD3L; ORIG.slice[3].S[ 45] = 0xF752F7DAL;
      ORIG.slice[3].S[ 46] = 0x3F046F69L; ORIG.slice[3].S[ 47] = 0x77FA0A59L;
      ORIG.slice[3].S[ 48] = 0x80E4A915L; ORIG.slice[3].S[ 49] = 0x87B08601L;
      ORIG.slice[3].S[ 50] = 0x9B09E6ADL; ORIG.slice[3].S[ 51] = 0x3B3EE593L;
      ORIG.slice[3].S[ 52] = 0xE990FD5AL; ORIG.slice[3].S[ 53] = 0x9E34D797L;
      ORIG.slice[3].S[ 54] = 0x2CF0B7D9L; ORIG.slice[3].S[ 55] = 0x022B8B51L;
      ORIG.slice[3].S[ 56] = 0x96D5AC3AL; ORIG.slice[3].S[ 57] = 0x017DA67DL;
      ORIG.slice[3].S[ 58] = 0xD1CF3ED6L; ORIG.slice[3].S[ 59] = 0x7C7D2D28L;
      ORIG.slice[3].S[ 60] = 0x1F9F25CFL; ORIG.slice[3].S[ 61] = 0xADF2B89BL;
      ORIG.slice[3].S[ 62] = 0x5AD6B472L; ORIG.slice[3].S[ 63] = 0x5A88F54CL;
      ORIG.slice[3].S[ 64] = 0xE029AC71L; ORIG.slice[3].S[ 65] = 0xE019A5E6L;
      ORIG.slice[3].S[ 66] = 0x47B0ACFDL; ORIG.slice[3].S[ 67] = 0xED93FA9BL;
      ORIG.slice[3].S[ 68] = 0xE8D3C48DL; ORIG.slice[3].S[ 69] = 0x283B57CCL;
      ORIG.slice[3].S[ 70] = 0xF8D56629L; ORIG.slice[3].S[ 71] = 0x79132E28L;
      ORIG.slice[3].S[ 72] = 0x785F0191L; ORIG.slice[3].S[ 73] = 0xED756055L;
      ORIG.slice[3].S[ 74] = 0xF7960E44L; ORIG.slice[3].S[ 75] = 0xE3D35E8CL;
      ORIG.slice[3].S[ 76] = 0x15056DD4L; ORIG.slice[3].S[ 77] = 0x88F46DBAL;
      ORIG.slice[3].S[ 78] = 0x03A16125L; ORIG.slice[3].S[ 79] = 0x0564F0BDL;
      ORIG.slice[3].S[ 80] = 0xC3EB9E15L; ORIG.slice[3].S[ 81] = 0x3C9057A2L;
      ORIG.slice[3].S[ 82] = 0x97271AECL; ORIG.slice[3].S[ 83] = 0xA93A072AL;
      ORIG.slice[3].S[ 84] = 0x1B3F6D9BL; ORIG.slice[3].S[ 85] = 0x1E6321F5L;
      ORIG.slice[3].S[ 86] = 0xF59C66FBL; ORIG.slice[3].S[ 87] = 0x26DCF319L;
      ORIG.slice[3].S[ 88] = 0x7533D928L; ORIG.slice[3].S[ 89] = 0xB155FDF5L;
      ORIG.slice[3].S[ 90] = 0x03563482L; ORIG.slice[3].S[ 91] = 0x8ABA3CBBL;
      ORIG.slice[3].S[ 92] = 0x28517711L; ORIG.slice[3].S[ 93] = 0xC20AD9F8L;
      ORIG.slice[3].S[ 94] = 0xABCC5167L; ORIG.slice[3].S[ 95] = 0xCCAD925FL;
      ORIG.slice[3].S[ 96] = 0x4DE81751L; ORIG.slice[3].S[ 97] = 0x3830DC8EL;
      ORIG.slice[3].S[ 98] = 0x379D5862L; ORIG.slice[3].S[ 99] = 0x9320F991L;
      ORIG.slice[3].S[100] = 0xEA7A90C2L; ORIG.slice[3].S[101] = 0xFB3E7BCEL;
      ORIG.slice[3].S[102] = 0x5121CE64L; ORIG.slice[3].S[103] = 0x774FBE32L;
      ORIG.slice[3].S[104] = 0xA8B6E37EL; ORIG.slice[3].S[105] = 0xC3293D46L;
      ORIG.slice[3].S[106] = 0x48DE5369L; ORIG.slice[3].S[107] = 0x6413E680L;
      ORIG.slice[3].S[108] = 0xA2AE0810L; ORIG.slice[3].S[109] = 0xDD6DB224L;
      ORIG.slice[3].S[110] = 0x69852DFDL; ORIG.slice[3].S[111] = 0x09072166L;
      ORIG.slice[3].S[112] = 0xB39A460AL; ORIG.slice[3].S[113] = 0x6445C0DDL;
      ORIG.slice[3].S[114] = 0x586CDECFL; ORIG.slice[3].S[115] = 0x1C20C8AEL;
      ORIG.slice[3].S[116] = 0x5BBEF7DDL; ORIG.slice[3].S[117] = 0x1B588D40L;
      ORIG.slice[3].S[118] = 0xCCD2017FL; ORIG.slice[3].S[119] = 0x6BB4E3BBL;
      ORIG.slice[3].S[120] = 0xDDA26A7EL; ORIG.slice[3].S[121] = 0x3A59FF45L;
      ORIG.slice[3].S[122] = 0x3E350A44L; ORIG.slice[3].S[123] = 0xBCB4CDD5L;
      ORIG.slice[3].S[124] = 0x72EACEA8L; ORIG.slice[3].S[125] = 0xFA6484BBL;
      ORIG.slice[3].S[126] = 0x8D6612AEL; ORIG.slice[3].S[127] = 0xBF3C6F47L;
      ORIG.slice[3].S[128] = 0xD29BE463L; ORIG.slice[3].S[129] = 0x542F5D9EL;
      ORIG.slice[3].S[130] = 0xAEC2771BL; ORIG.slice[3].S[131] = 0xF64E6370L;
      ORIG.slice[3].S[132] = 0x740E0D8DL; ORIG.slice[3].S[133] = 0xE75B1357L;
      ORIG.slice[3].S[134] = 0xF8721671L; ORIG.slice[3].S[135] = 0xAF537D5DL;
      ORIG.slice[3].S[136] = 0x4040CB08L; ORIG.slice[3].S[137] = 0x4EB4E2CCL;
      ORIG.slice[3].S[138] = 0x34D2466AL; ORIG.slice[3].S[139] = 0x0115AF84L;
      ORIG.slice[3].S[140] = 0xE1B00428L; ORIG.slice[3].S[141] = 0x95983A1DL;
      ORIG.slice[3].S[142] = 0x06B89FB4L; ORIG.slice[3].S[143] = 0xCE6EA048L;
      ORIG.slice[3].S[144] = 0x6F3F3B82L; ORIG.slice[3].S[145] = 0x3520AB82L;
      ORIG.slice[3].S[146] = 0x011A1D4BL; ORIG.slice[3].S[147] = 0x277227F8L;
      ORIG.slice[3].S[148] = 0x611560B1L; ORIG.slice[3].S[149] = 0xE7933FDCL;
      ORIG.slice[3].S[150] = 0xBB3A792BL; ORIG.slice[3].S[151] = 0x344525BDL;
      ORIG.slice[3].S[152] = 0xA08839E1L; ORIG.slice[3].S[153] = 0x51CE794BL;
      ORIG.slice[3].S[154] = 0x2F32C9B7L; ORIG.slice[3].S[155] = 0xA01FBAC9L;
      ORIG.slice[3].S[156] = 0xE01CC87EL; ORIG.slice[3].S[157] = 0xBCC7D1F6L;
      ORIG.slice[3].S[158] = 0xCF0111C3L; ORIG.slice[3].S[159] = 0xA1E8AAC7L;
      ORIG.slice[3].S[160] = 0x1A908749L; ORIG.slice[3].S[161] = 0xD44FBD9AL;
      ORIG.slice[3].S[162] = 0xD0DADECBL; ORIG.slice[3].S[163] = 0xD50ADA38L;
      ORIG.slice[3].S[164] = 0x0339C32AL; ORIG.slice[3].S[165] = 0xC6913667L;
      ORIG.slice[3].S[166] = 0x8DF9317CL; ORIG.slice[3].S[167] = 0xE0B12B4FL;
      ORIG.slice[3].S[168] = 0xF79E59B7L; ORIG.slice[3].S[169] = 0x43F5BB3AL;
      ORIG.slice[3].S[170] = 0xF2D519FFL; ORIG.slice[3].S[171] = 0x27D9459CL;
      ORIG.slice[3].S[172] = 0xBF97222CL; ORIG.slice[3].S[173] = 0x15E6FC2AL;
      ORIG.slice[3].S[174] = 0x0F91FC71L; ORIG.slice[3].S[175] = 0x9B941525L;
      ORIG.slice[3].S[176] = 0xFAE59361L; ORIG.slice[3].S[177] = 0xCEB69CEBL;
      ORIG.slice[3].S[178] = 0xC2A86459L; ORIG.slice[3].S[179] = 0x12BAA8D1L;
      ORIG.slice[3].S[180] = 0xB6C1075EL; ORIG.slice[3].S[181] = 0xE3056A0CL;
      ORIG.slice[3].S[182] = 0x10D25065L; ORIG.slice[3].S[183] = 0xCB03A442L;
      ORIG.slice[3].S[184] = 0xE0EC6E0EL; ORIG.slice[3].S[185] = 0x1698DB3BL;
      ORIG.slice[3].S[186] = 0x4C98A0BEL; ORIG.slice[3].S[187] = 0x3278E964L;
      ORIG.slice[3].S[188] = 0x9F1F9532L; ORIG.slice[3].S[189] = 0xE0D392DFL;
      ORIG.slice[3].S[190] = 0xD3A0342BL; ORIG.slice[3].S[191] = 0x8971F21EL;
      ORIG.slice[3].S[192] = 0x1B0A7441L; ORIG.slice[3].S[193] = 0x4BA3348CL;
      ORIG.slice[3].S[194] = 0xC5BE7120L; ORIG.slice[3].S[195] = 0xC37632D8L;
      ORIG.slice[3].S[196] = 0xDF359F8DL; ORIG.slice[3].S[197] = 0x9B992F2EL;
      ORIG.slice[3].S[198] = 0xE60B6F47L; ORIG.slice[3].S[199] = 0x0FE3F11DL;
      ORIG.slice[3].S[200] = 0xE54CDA54L; ORIG.slice[3].S[201] = 0x1EDAD891L;
      ORIG.slice[3].S[202] = 0xCE6279CFL; ORIG.slice[3].S[203] = 0xCD3E7E6FL;
      ORIG.slice[3].S[204] = 0x1618B166L; ORIG.slice[3].S[205] = 0xFD2C1D05L;
      ORIG.slice[3].S[206] = 0x848FD2C5L; ORIG.slice[3].S[207] = 0xF6FB2299L;
      ORIG.slice[3].S[208] = 0xF523F357L; ORIG.slice[3].S[209] = 0xA6327623L;
      ORIG.slice[3].S[210] = 0x93A83531L; ORIG.slice[3].S[211] = 0x56CCCD02L;
      ORIG.slice[3].S[212] = 0xACF08162L; ORIG.slice[3].S[213] = 0x5A75EBB5L;
      ORIG.slice[3].S[214] = 0x6E163697L; ORIG.slice[3].S[215] = 0x88D273CCL;
      ORIG.slice[3].S[216] = 0xDE966292L; ORIG.slice[3].S[217] = 0x81B949D0L;
      ORIG.slice[3].S[218] = 0x4C50901BL; ORIG.slice[3].S[219] = 0x71C65614L;
      ORIG.slice[3].S[220] = 0xE6C6C7BDL; ORIG.slice[3].S[221] = 0x327A140AL;
      ORIG.slice[3].S[222] = 0x45E1D006L; ORIG.slice[3].S[223] = 0xC3F27B9AL;
      ORIG.slice[3].S[224] = 0xC9AA53FDL; ORIG.slice[3].S[225] = 0x62A80F00L;
      ORIG.slice[3].S[226] = 0xBB25BFE2L; ORIG.slice[3].S[227] = 0x35BDD2F6L;
      ORIG.slice[3].S[228] = 0x71126905L; ORIG.slice[3].S[229] = 0xB2040222L;
      ORIG.slice[3].S[230] = 0xB6CBCF7CL; ORIG.slice[3].S[231] = 0xCD769C2BL;
      ORIG.slice[3].S[232] = 0x53113EC0L; ORIG.slice[3].S[233] = 0x1640E3D3L;
      ORIG.slice[3].S[234] = 0x38ABBD60L; ORIG.slice[3].S[235] = 0x2547ADF0L;
      ORIG.slice[3].S[236] = 0xBA38209CL; ORIG.slice[3].S[237] = 0xF746CE76L;
      ORIG.slice[3].S[238] = 0x77AFA1C5L; ORIG.slice[3].S[239] = 0x20756060L;
      ORIG.slice[3].S[240] = 0x85CBFE4EL; ORIG.slice[3].S[241] = 0x8AE88DD8L;
      ORIG.slice[3].S[242] = 0x7AAAF9B0L; ORIG.slice[3].S[243] = 0x4CF9AA7EL;
      ORIG.slice[3].S[244] = 0x1948C25CL; ORIG.slice[3].S[245] = 0x02FB8A8CL;
      ORIG.slice[3].S[246] = 0x01C36AE4L; ORIG.slice[3].S[247] = 0xD6EBE1F9L;
      ORIG.slice[3].S[248] = 0x90D4F869L; ORIG.slice[3].S[249] = 0xA65CDEA0L;
      ORIG.slice[3].S[250] = 0x3F09252DL; ORIG.slice[3].S[251] = 0xC208E69FL;
      ORIG.slice[3].S[252] = 0xB74E6132L; ORIG.slice[3].S[253] = 0xCE77E25BL;
      ORIG.slice[3].S[254] = 0x578FDFE3L; ORIG.slice[3].S[255] = 0x3AC372E6L;
        
      ORIG.P[ 0] = 0x243F6A88L;
      ORIG.P[ 1] = 0x85A308D3L;
      ORIG.P[ 2] = 0x13198A2EL;
      ORIG.P[ 3] = 0x03707344L;
      ORIG.P[ 4] = 0xA4093822L;
      ORIG.P[ 5] = 0x299F31D0L;
      ORIG.P[ 6] = 0x082EFA98L;
      ORIG.P[ 7] = 0xEC4E6C89L;
      ORIG.P[ 8] = 0x452821E6L;
      ORIG.P[ 9] = 0x38D01377L;
      ORIG.P[10] = 0xBE5466CFL;
      ORIG.P[11] = 0x34E90C6CL;
      ORIG.P[12] = 0xC0AC29B7L;
      ORIG.P[13] = 0xC97C50DDL;
      ORIG.P[14] = 0x3F84D5B5L;
      ORIG.P[15] = 0xB5470917L;
      ORIG.P[16] = 0x9216D5D9L;
      ORIG.P[17] = 0x8979FB1BL;
}

Blowfish::~Blowfish()
{
}

void Blowfish::setKey(ExampleNetworkPacket packet){
  sc_bv< 56 > key_bits;
  sc_bv< 3 > used_bytes;
  int keys_to_go = 8 - this->key_parts_already_processed; // 8 keys for blowfish
  
  for(int i=0; i < PACKET_PAYLOAD && keys_to_go > 0; i++, keys_to_go--, key_parts_already_processed++){
    key_bits = packet.payload[i].range(55, 0);
    used_bytes = sc_bv<3>( packet.payload[i].range(58, 56) );
    this->setKeyBits(this->key_parts_already_processed, key_bits, used_bytes);
  }

  // all parts of key received
  if(this->key_parts_already_processed == 8){
    this->initialize();
    this->key_parts_already_processed = 0;
  }
}

sc_uint<32> Blowfish::F(sc_bv<32> x)
{
  sc_uint<8> a, b, c, d;

  d = static_cast< sc_bv<8> >(x.range( 7,  0));
  c = static_cast< sc_bv<8> >(x.range(15,  8));
  b = static_cast< sc_bv<8> >(x.range(23, 16));
  a = static_cast< sc_bv<8> >(x.range(31, 24));

  sc_uint<32> s0a, s1b, s2c, s3d;
  s0a = ctx.slice[0].S[a];
  s1b = ctx.slice[1].S[b];
  s2c = ctx.slice[2].S[c];
  s3d = ctx.slice[3].S[d];

  sc_uint<32> y;
  y = s0a + s1b;
  y = y ^ s2c;
  y = y + s3d;

  return y;
}


void
Blowfish::encrypt64(sc_bv<64> & data)
{
  sc_bv<32>  Xl;
  sc_bv<32>  Xr;
  sc_bv<32>  temp;
  sc_uint<5> i;

  Xl = data.range(63, 32);
  Xr = data.range(31,  0);

  for (i = 0; i < BLOWFISH_N; ++i) {
    Xl = Xl ^ ctx.P[i];
    Xr = F(Xl) ^ Xr;
    temp = Xl;
    Xl = Xr;
    Xr = temp;
    //wait(SC_ZERO_TIME);//wait();
  }

  temp = Xl;
  Xl = Xr;
  Xr = temp;
  Xr = Xr ^ ctx.P[BLOWFISH_N];
  Xl = Xl ^ ctx.P[BLOWFISH_N + 1];

  data.range(63, 32) = Xl;
  data.range(31,  0) = Xr;
}

void
Blowfish::decrypt64(sc_bv<64> & data)
{
  sc_bv<32>  Xl;
  sc_bv<32>  Xr;
  sc_bv<32>  temp;
  sc_uint<5> i;

  Xl = data.range(63, 32);
  Xr = data.range(31,  0);

  for (i = BLOWFISH_N + 1; i > 1; --i) {
    Xl = Xl ^ ctx.P[i];
    Xr = F(Xl) ^ Xr;
    temp = Xl;
    Xl = Xr;
    Xr = temp;
    //wait(SC_ZERO_TIME);//wait();
  }

  temp = Xl;
  Xl = Xr;
  Xr = temp;

  Xr = Xr ^ ctx.P[1];
  Xl = Xl ^ ctx.P[0];

  data.range(63, 32) = Xl;
  data.range(31,  0) = Xr;
}

void
Blowfish::encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  sc_bv<64> part = data.range(127, 64);
  encrypt64(part);
  data.range(127, 64) = part;
  if (length_in_bytes > 4)
  {
    part = data.range(63, 0);
    encrypt64(part);
  }
  else
  {
    part = "00000000000000000000000000000000";
  }
  data.range(63, 0) = part;  
}

void
Blowfish::decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes)
{
  sc_bv<64> part = data.range(127, 64);
  decrypt64(part);
  data.range(127, 64) = part;
  if (length_in_bytes > 4)
  {
    part = data.range(63, 0);
    decrypt64(part);
  }
  else
  {
    part = "00000000000000000000000000000000";
  }
  data.range(63, 0) = part;  
}

void
Blowfish::initialize()
{
  unsigned int i, j, k;
  sc_bv<32> data, datal, datar;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 256; j++)
      ctx.slice[i].S[j] = ORIG.slice[i].S[j];
  }

  j = 0;
  for (i = 0; i < BLOWFISH_N + 2; ++i) {
    data = 0x00000000;
    for (k = 0; k < 4; ++k) {
      data = (data << 8) | key.character[j];
      j = j + 1;
      if (j >= key.length)
        j = 0;
    }
    ctx.P[i] = ORIG.P[i] ^ data;
    //wait(SC_ZERO_TIME);//wait();
  }

  datal = 0x00000000;
  datar = 0x00000000;

  sc_bv<64> dataword = (datal, datar);
  for (i = 0; i < BLOWFISH_N + 2; i += 2) {
    encrypt64(dataword);
    ctx.P[i]     = dataword.range(63, 32);
    ctx.P[i + 1] = dataword.range(31, 0);
  }

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 256; j += 2) {
      encrypt64(dataword);
      ctx.slice[i].S[j]     = dataword.range(63, 32);
      ctx.slice[i].S[j + 1] = dataword.range(31, 0);
    }
  }

}

void
Blowfish::setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key)
{
  for (sc_uint<9> char_nr = 0; char_nr < 7; char_nr++)
  {
    sc_bv<8> one_byte = bits.range(char_nr * 8 + 7,
                                   char_nr * 8 + 0);
    int index = static_cast< sc_uint<9> >(part) * 7 + char_nr;
    key.character[ index ] = one_byte;
  }
  if (part == 0) { key.length = 0; }
  key.length = key.length + used_bytes_in_key;
  //wait(SC_ZERO_TIME);//wait();
}
