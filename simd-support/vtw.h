/* auto-generated */
#if defined(REQ_VTW1)
#if defined(VTW_SIZE) && VTW_SIZE == 1
#warning "using VTW1 with 1"
#define VTW1(v,x) {TW_CEXP, v+0, x} 
#endif // VTW_SIZE == 1
#if defined(VTW_SIZE) && VTW_SIZE == 2
#warning "using VTW1 with 2"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x} 
#endif // VTW_SIZE == 2
#if defined(VTW_SIZE) && VTW_SIZE == 4
#warning "using VTW1 with 4"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x} 
#endif // VTW_SIZE == 4
#if defined(VTW_SIZE) && VTW_SIZE == 8
#warning "using VTW1 with 8"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x} 
#endif // VTW_SIZE == 8
#if defined(VTW_SIZE) && VTW_SIZE == 16
#warning "using VTW1 with 16"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
	{TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
	{TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x} 
#endif // VTW_SIZE == 16
#if defined(VTW_SIZE) && VTW_SIZE == 32
#warning "using VTW1 with 32"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
	{TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
	{TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x}, \
	{TW_CEXP, v+16, x}, {TW_CEXP, v+17, x}, {TW_CEXP, v+18, x}, {TW_CEXP, v+19, x}, \
	{TW_CEXP, v+20, x}, {TW_CEXP, v+21, x}, {TW_CEXP, v+22, x}, {TW_CEXP, v+23, x}, \
	{TW_CEXP, v+24, x}, {TW_CEXP, v+25, x}, {TW_CEXP, v+26, x}, {TW_CEXP, v+27, x}, \
	{TW_CEXP, v+28, x}, {TW_CEXP, v+29, x}, {TW_CEXP, v+30, x}, {TW_CEXP, v+31, x} 
#endif // VTW_SIZE == 32
#if defined(VTW_SIZE) && VTW_SIZE == 64
#warning "using VTW1 with 64"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
	{TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
	{TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x}, \
	{TW_CEXP, v+16, x}, {TW_CEXP, v+17, x}, {TW_CEXP, v+18, x}, {TW_CEXP, v+19, x}, \
	{TW_CEXP, v+20, x}, {TW_CEXP, v+21, x}, {TW_CEXP, v+22, x}, {TW_CEXP, v+23, x}, \
	{TW_CEXP, v+24, x}, {TW_CEXP, v+25, x}, {TW_CEXP, v+26, x}, {TW_CEXP, v+27, x}, \
	{TW_CEXP, v+28, x}, {TW_CEXP, v+29, x}, {TW_CEXP, v+30, x}, {TW_CEXP, v+31, x}, \
	{TW_CEXP, v+32, x}, {TW_CEXP, v+33, x}, {TW_CEXP, v+34, x}, {TW_CEXP, v+35, x}, \
	{TW_CEXP, v+36, x}, {TW_CEXP, v+37, x}, {TW_CEXP, v+38, x}, {TW_CEXP, v+39, x}, \
	{TW_CEXP, v+40, x}, {TW_CEXP, v+41, x}, {TW_CEXP, v+42, x}, {TW_CEXP, v+43, x}, \
	{TW_CEXP, v+44, x}, {TW_CEXP, v+45, x}, {TW_CEXP, v+46, x}, {TW_CEXP, v+47, x}, \
	{TW_CEXP, v+48, x}, {TW_CEXP, v+49, x}, {TW_CEXP, v+50, x}, {TW_CEXP, v+51, x}, \
	{TW_CEXP, v+52, x}, {TW_CEXP, v+53, x}, {TW_CEXP, v+54, x}, {TW_CEXP, v+55, x}, \
	{TW_CEXP, v+56, x}, {TW_CEXP, v+57, x}, {TW_CEXP, v+58, x}, {TW_CEXP, v+59, x}, \
	{TW_CEXP, v+60, x}, {TW_CEXP, v+61, x}, {TW_CEXP, v+62, x}, {TW_CEXP, v+63, x} 
#endif // VTW_SIZE == 64
#if defined(VTW_SIZE) && VTW_SIZE == 128
#warning "using VTW1 with 128"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
	{TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
	{TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x}, \
	{TW_CEXP, v+16, x}, {TW_CEXP, v+17, x}, {TW_CEXP, v+18, x}, {TW_CEXP, v+19, x}, \
	{TW_CEXP, v+20, x}, {TW_CEXP, v+21, x}, {TW_CEXP, v+22, x}, {TW_CEXP, v+23, x}, \
	{TW_CEXP, v+24, x}, {TW_CEXP, v+25, x}, {TW_CEXP, v+26, x}, {TW_CEXP, v+27, x}, \
	{TW_CEXP, v+28, x}, {TW_CEXP, v+29, x}, {TW_CEXP, v+30, x}, {TW_CEXP, v+31, x}, \
	{TW_CEXP, v+32, x}, {TW_CEXP, v+33, x}, {TW_CEXP, v+34, x}, {TW_CEXP, v+35, x}, \
	{TW_CEXP, v+36, x}, {TW_CEXP, v+37, x}, {TW_CEXP, v+38, x}, {TW_CEXP, v+39, x}, \
	{TW_CEXP, v+40, x}, {TW_CEXP, v+41, x}, {TW_CEXP, v+42, x}, {TW_CEXP, v+43, x}, \
	{TW_CEXP, v+44, x}, {TW_CEXP, v+45, x}, {TW_CEXP, v+46, x}, {TW_CEXP, v+47, x}, \
	{TW_CEXP, v+48, x}, {TW_CEXP, v+49, x}, {TW_CEXP, v+50, x}, {TW_CEXP, v+51, x}, \
	{TW_CEXP, v+52, x}, {TW_CEXP, v+53, x}, {TW_CEXP, v+54, x}, {TW_CEXP, v+55, x}, \
	{TW_CEXP, v+56, x}, {TW_CEXP, v+57, x}, {TW_CEXP, v+58, x}, {TW_CEXP, v+59, x}, \
	{TW_CEXP, v+60, x}, {TW_CEXP, v+61, x}, {TW_CEXP, v+62, x}, {TW_CEXP, v+63, x}, \
	{TW_CEXP, v+64, x}, {TW_CEXP, v+65, x}, {TW_CEXP, v+66, x}, {TW_CEXP, v+67, x}, \
	{TW_CEXP, v+68, x}, {TW_CEXP, v+69, x}, {TW_CEXP, v+70, x}, {TW_CEXP, v+71, x}, \
	{TW_CEXP, v+72, x}, {TW_CEXP, v+73, x}, {TW_CEXP, v+74, x}, {TW_CEXP, v+75, x}, \
	{TW_CEXP, v+76, x}, {TW_CEXP, v+77, x}, {TW_CEXP, v+78, x}, {TW_CEXP, v+79, x}, \
	{TW_CEXP, v+80, x}, {TW_CEXP, v+81, x}, {TW_CEXP, v+82, x}, {TW_CEXP, v+83, x}, \
	{TW_CEXP, v+84, x}, {TW_CEXP, v+85, x}, {TW_CEXP, v+86, x}, {TW_CEXP, v+87, x}, \
	{TW_CEXP, v+88, x}, {TW_CEXP, v+89, x}, {TW_CEXP, v+90, x}, {TW_CEXP, v+91, x}, \
	{TW_CEXP, v+92, x}, {TW_CEXP, v+93, x}, {TW_CEXP, v+94, x}, {TW_CEXP, v+95, x}, \
	{TW_CEXP, v+96, x}, {TW_CEXP, v+97, x}, {TW_CEXP, v+98, x}, {TW_CEXP, v+99, x}, \
	{TW_CEXP, v+100, x}, {TW_CEXP, v+101, x}, {TW_CEXP, v+102, x}, {TW_CEXP, v+103, x}, \
	{TW_CEXP, v+104, x}, {TW_CEXP, v+105, x}, {TW_CEXP, v+106, x}, {TW_CEXP, v+107, x}, \
	{TW_CEXP, v+108, x}, {TW_CEXP, v+109, x}, {TW_CEXP, v+110, x}, {TW_CEXP, v+111, x}, \
	{TW_CEXP, v+112, x}, {TW_CEXP, v+113, x}, {TW_CEXP, v+114, x}, {TW_CEXP, v+115, x}, \
	{TW_CEXP, v+116, x}, {TW_CEXP, v+117, x}, {TW_CEXP, v+118, x}, {TW_CEXP, v+119, x}, \
	{TW_CEXP, v+120, x}, {TW_CEXP, v+121, x}, {TW_CEXP, v+122, x}, {TW_CEXP, v+123, x}, \
	{TW_CEXP, v+124, x}, {TW_CEXP, v+125, x}, {TW_CEXP, v+126, x}, {TW_CEXP, v+127, x} 
#endif // VTW_SIZE == 128
#if defined(VTW_SIZE) && VTW_SIZE == 256
#warning "using VTW1 with 256"
#define VTW1(v,x) {TW_CEXP, v+0, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
	{TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
	{TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
	{TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x}, \
	{TW_CEXP, v+16, x}, {TW_CEXP, v+17, x}, {TW_CEXP, v+18, x}, {TW_CEXP, v+19, x}, \
	{TW_CEXP, v+20, x}, {TW_CEXP, v+21, x}, {TW_CEXP, v+22, x}, {TW_CEXP, v+23, x}, \
	{TW_CEXP, v+24, x}, {TW_CEXP, v+25, x}, {TW_CEXP, v+26, x}, {TW_CEXP, v+27, x}, \
	{TW_CEXP, v+28, x}, {TW_CEXP, v+29, x}, {TW_CEXP, v+30, x}, {TW_CEXP, v+31, x}, \
	{TW_CEXP, v+32, x}, {TW_CEXP, v+33, x}, {TW_CEXP, v+34, x}, {TW_CEXP, v+35, x}, \
	{TW_CEXP, v+36, x}, {TW_CEXP, v+37, x}, {TW_CEXP, v+38, x}, {TW_CEXP, v+39, x}, \
	{TW_CEXP, v+40, x}, {TW_CEXP, v+41, x}, {TW_CEXP, v+42, x}, {TW_CEXP, v+43, x}, \
	{TW_CEXP, v+44, x}, {TW_CEXP, v+45, x}, {TW_CEXP, v+46, x}, {TW_CEXP, v+47, x}, \
	{TW_CEXP, v+48, x}, {TW_CEXP, v+49, x}, {TW_CEXP, v+50, x}, {TW_CEXP, v+51, x}, \
	{TW_CEXP, v+52, x}, {TW_CEXP, v+53, x}, {TW_CEXP, v+54, x}, {TW_CEXP, v+55, x}, \
	{TW_CEXP, v+56, x}, {TW_CEXP, v+57, x}, {TW_CEXP, v+58, x}, {TW_CEXP, v+59, x}, \
	{TW_CEXP, v+60, x}, {TW_CEXP, v+61, x}, {TW_CEXP, v+62, x}, {TW_CEXP, v+63, x}, \
	{TW_CEXP, v+64, x}, {TW_CEXP, v+65, x}, {TW_CEXP, v+66, x}, {TW_CEXP, v+67, x}, \
	{TW_CEXP, v+68, x}, {TW_CEXP, v+69, x}, {TW_CEXP, v+70, x}, {TW_CEXP, v+71, x}, \
	{TW_CEXP, v+72, x}, {TW_CEXP, v+73, x}, {TW_CEXP, v+74, x}, {TW_CEXP, v+75, x}, \
	{TW_CEXP, v+76, x}, {TW_CEXP, v+77, x}, {TW_CEXP, v+78, x}, {TW_CEXP, v+79, x}, \
	{TW_CEXP, v+80, x}, {TW_CEXP, v+81, x}, {TW_CEXP, v+82, x}, {TW_CEXP, v+83, x}, \
	{TW_CEXP, v+84, x}, {TW_CEXP, v+85, x}, {TW_CEXP, v+86, x}, {TW_CEXP, v+87, x}, \
	{TW_CEXP, v+88, x}, {TW_CEXP, v+89, x}, {TW_CEXP, v+90, x}, {TW_CEXP, v+91, x}, \
	{TW_CEXP, v+92, x}, {TW_CEXP, v+93, x}, {TW_CEXP, v+94, x}, {TW_CEXP, v+95, x}, \
	{TW_CEXP, v+96, x}, {TW_CEXP, v+97, x}, {TW_CEXP, v+98, x}, {TW_CEXP, v+99, x}, \
	{TW_CEXP, v+100, x}, {TW_CEXP, v+101, x}, {TW_CEXP, v+102, x}, {TW_CEXP, v+103, x}, \
	{TW_CEXP, v+104, x}, {TW_CEXP, v+105, x}, {TW_CEXP, v+106, x}, {TW_CEXP, v+107, x}, \
	{TW_CEXP, v+108, x}, {TW_CEXP, v+109, x}, {TW_CEXP, v+110, x}, {TW_CEXP, v+111, x}, \
	{TW_CEXP, v+112, x}, {TW_CEXP, v+113, x}, {TW_CEXP, v+114, x}, {TW_CEXP, v+115, x}, \
	{TW_CEXP, v+116, x}, {TW_CEXP, v+117, x}, {TW_CEXP, v+118, x}, {TW_CEXP, v+119, x}, \
	{TW_CEXP, v+120, x}, {TW_CEXP, v+121, x}, {TW_CEXP, v+122, x}, {TW_CEXP, v+123, x}, \
	{TW_CEXP, v+124, x}, {TW_CEXP, v+125, x}, {TW_CEXP, v+126, x}, {TW_CEXP, v+127, x}, \
	{TW_CEXP, v+128, x}, {TW_CEXP, v+129, x}, {TW_CEXP, v+130, x}, {TW_CEXP, v+131, x}, \
	{TW_CEXP, v+132, x}, {TW_CEXP, v+133, x}, {TW_CEXP, v+134, x}, {TW_CEXP, v+135, x}, \
	{TW_CEXP, v+136, x}, {TW_CEXP, v+137, x}, {TW_CEXP, v+138, x}, {TW_CEXP, v+139, x}, \
	{TW_CEXP, v+140, x}, {TW_CEXP, v+141, x}, {TW_CEXP, v+142, x}, {TW_CEXP, v+143, x}, \
	{TW_CEXP, v+144, x}, {TW_CEXP, v+145, x}, {TW_CEXP, v+146, x}, {TW_CEXP, v+147, x}, \
	{TW_CEXP, v+148, x}, {TW_CEXP, v+149, x}, {TW_CEXP, v+150, x}, {TW_CEXP, v+151, x}, \
	{TW_CEXP, v+152, x}, {TW_CEXP, v+153, x}, {TW_CEXP, v+154, x}, {TW_CEXP, v+155, x}, \
	{TW_CEXP, v+156, x}, {TW_CEXP, v+157, x}, {TW_CEXP, v+158, x}, {TW_CEXP, v+159, x}, \
	{TW_CEXP, v+160, x}, {TW_CEXP, v+161, x}, {TW_CEXP, v+162, x}, {TW_CEXP, v+163, x}, \
	{TW_CEXP, v+164, x}, {TW_CEXP, v+165, x}, {TW_CEXP, v+166, x}, {TW_CEXP, v+167, x}, \
	{TW_CEXP, v+168, x}, {TW_CEXP, v+169, x}, {TW_CEXP, v+170, x}, {TW_CEXP, v+171, x}, \
	{TW_CEXP, v+172, x}, {TW_CEXP, v+173, x}, {TW_CEXP, v+174, x}, {TW_CEXP, v+175, x}, \
	{TW_CEXP, v+176, x}, {TW_CEXP, v+177, x}, {TW_CEXP, v+178, x}, {TW_CEXP, v+179, x}, \
	{TW_CEXP, v+180, x}, {TW_CEXP, v+181, x}, {TW_CEXP, v+182, x}, {TW_CEXP, v+183, x}, \
	{TW_CEXP, v+184, x}, {TW_CEXP, v+185, x}, {TW_CEXP, v+186, x}, {TW_CEXP, v+187, x}, \
	{TW_CEXP, v+188, x}, {TW_CEXP, v+189, x}, {TW_CEXP, v+190, x}, {TW_CEXP, v+191, x}, \
	{TW_CEXP, v+192, x}, {TW_CEXP, v+193, x}, {TW_CEXP, v+194, x}, {TW_CEXP, v+195, x}, \
	{TW_CEXP, v+196, x}, {TW_CEXP, v+197, x}, {TW_CEXP, v+198, x}, {TW_CEXP, v+199, x}, \
	{TW_CEXP, v+200, x}, {TW_CEXP, v+201, x}, {TW_CEXP, v+202, x}, {TW_CEXP, v+203, x}, \
	{TW_CEXP, v+204, x}, {TW_CEXP, v+205, x}, {TW_CEXP, v+206, x}, {TW_CEXP, v+207, x}, \
	{TW_CEXP, v+208, x}, {TW_CEXP, v+209, x}, {TW_CEXP, v+210, x}, {TW_CEXP, v+211, x}, \
	{TW_CEXP, v+212, x}, {TW_CEXP, v+213, x}, {TW_CEXP, v+214, x}, {TW_CEXP, v+215, x}, \
	{TW_CEXP, v+216, x}, {TW_CEXP, v+217, x}, {TW_CEXP, v+218, x}, {TW_CEXP, v+219, x}, \
	{TW_CEXP, v+220, x}, {TW_CEXP, v+221, x}, {TW_CEXP, v+222, x}, {TW_CEXP, v+223, x}, \
	{TW_CEXP, v+224, x}, {TW_CEXP, v+225, x}, {TW_CEXP, v+226, x}, {TW_CEXP, v+227, x}, \
	{TW_CEXP, v+228, x}, {TW_CEXP, v+229, x}, {TW_CEXP, v+230, x}, {TW_CEXP, v+231, x}, \
	{TW_CEXP, v+232, x}, {TW_CEXP, v+233, x}, {TW_CEXP, v+234, x}, {TW_CEXP, v+235, x}, \
	{TW_CEXP, v+236, x}, {TW_CEXP, v+237, x}, {TW_CEXP, v+238, x}, {TW_CEXP, v+239, x}, \
	{TW_CEXP, v+240, x}, {TW_CEXP, v+241, x}, {TW_CEXP, v+242, x}, {TW_CEXP, v+243, x}, \
	{TW_CEXP, v+244, x}, {TW_CEXP, v+245, x}, {TW_CEXP, v+246, x}, {TW_CEXP, v+247, x}, \
	{TW_CEXP, v+248, x}, {TW_CEXP, v+249, x}, {TW_CEXP, v+250, x}, {TW_CEXP, v+251, x}, \
	{TW_CEXP, v+252, x}, {TW_CEXP, v+253, x}, {TW_CEXP, v+254, x}, {TW_CEXP, v+255, x} 
#endif // VTW_SIZE == 256
#endif // REQ_VTW1
#if defined(REQ_VTW2)
#if defined(VTW_SIZE) && VTW_SIZE == 1
#warning "using VTW2 with 1"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_SIN, v+0, -x} 
#endif // VTW_SIZE == 1
#if defined(VTW_SIZE) && VTW_SIZE == 2
#warning "using VTW2 with 2"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_SIN, v+0, -x}, {TW_SIN, v+0, x} 
#endif // VTW_SIZE == 2
#if defined(VTW_SIZE) && VTW_SIZE == 4
#warning "using VTW2 with 4"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x} 
#endif // VTW_SIZE == 4
#if defined(VTW_SIZE) && VTW_SIZE == 8
#warning "using VTW2 with 8"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x} 
#endif // VTW_SIZE == 8
#if defined(VTW_SIZE) && VTW_SIZE == 16
#warning "using VTW2 with 16"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
	{TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
	{TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x} 
#endif // VTW_SIZE == 16
#if defined(VTW_SIZE) && VTW_SIZE == 32
#warning "using VTW2 with 32"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
	{TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+9, x}, \
	{TW_COS, v+10, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+13, x}, \
	{TW_COS, v+14, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, {TW_COS, v+15, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
	{TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, -x}, {TW_SIN, v+8, x}, {TW_SIN, v+9, -x}, {TW_SIN, v+9, x}, \
	{TW_SIN, v+10, -x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, -x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, -x}, {TW_SIN, v+12, x}, {TW_SIN, v+13, -x}, {TW_SIN, v+13, x}, \
	{TW_SIN, v+14, -x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, -x}, {TW_SIN, v+15, x} 
#endif // VTW_SIZE == 32
#if defined(VTW_SIZE) && VTW_SIZE == 64
#warning "using VTW2 with 64"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
	{TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+9, x}, \
	{TW_COS, v+10, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+13, x}, \
	{TW_COS, v+14, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+17, x}, \
	{TW_COS, v+18, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+21, x}, \
	{TW_COS, v+22, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+25, x}, \
	{TW_COS, v+26, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+29, x}, \
	{TW_COS, v+30, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, {TW_COS, v+31, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
	{TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, -x}, {TW_SIN, v+8, x}, {TW_SIN, v+9, -x}, {TW_SIN, v+9, x}, \
	{TW_SIN, v+10, -x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, -x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, -x}, {TW_SIN, v+12, x}, {TW_SIN, v+13, -x}, {TW_SIN, v+13, x}, \
	{TW_SIN, v+14, -x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, -x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, -x}, {TW_SIN, v+16, x}, {TW_SIN, v+17, -x}, {TW_SIN, v+17, x}, \
	{TW_SIN, v+18, -x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, -x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, -x}, {TW_SIN, v+20, x}, {TW_SIN, v+21, -x}, {TW_SIN, v+21, x}, \
	{TW_SIN, v+22, -x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, -x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, -x}, {TW_SIN, v+24, x}, {TW_SIN, v+25, -x}, {TW_SIN, v+25, x}, \
	{TW_SIN, v+26, -x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, -x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, -x}, {TW_SIN, v+28, x}, {TW_SIN, v+29, -x}, {TW_SIN, v+29, x}, \
	{TW_SIN, v+30, -x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, -x}, {TW_SIN, v+31, x} 
#endif // VTW_SIZE == 64
#if defined(VTW_SIZE) && VTW_SIZE == 128
#warning "using VTW2 with 128"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
	{TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+9, x}, \
	{TW_COS, v+10, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+13, x}, \
	{TW_COS, v+14, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+17, x}, \
	{TW_COS, v+18, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+21, x}, \
	{TW_COS, v+22, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+25, x}, \
	{TW_COS, v+26, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+29, x}, \
	{TW_COS, v+30, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, {TW_COS, v+31, x}, \
	{TW_COS, v+32, x}, {TW_COS, v+32, x}, {TW_COS, v+33, x}, {TW_COS, v+33, x}, \
	{TW_COS, v+34, x}, {TW_COS, v+34, x}, {TW_COS, v+35, x}, {TW_COS, v+35, x}, \
	{TW_COS, v+36, x}, {TW_COS, v+36, x}, {TW_COS, v+37, x}, {TW_COS, v+37, x}, \
	{TW_COS, v+38, x}, {TW_COS, v+38, x}, {TW_COS, v+39, x}, {TW_COS, v+39, x}, \
	{TW_COS, v+40, x}, {TW_COS, v+40, x}, {TW_COS, v+41, x}, {TW_COS, v+41, x}, \
	{TW_COS, v+42, x}, {TW_COS, v+42, x}, {TW_COS, v+43, x}, {TW_COS, v+43, x}, \
	{TW_COS, v+44, x}, {TW_COS, v+44, x}, {TW_COS, v+45, x}, {TW_COS, v+45, x}, \
	{TW_COS, v+46, x}, {TW_COS, v+46, x}, {TW_COS, v+47, x}, {TW_COS, v+47, x}, \
	{TW_COS, v+48, x}, {TW_COS, v+48, x}, {TW_COS, v+49, x}, {TW_COS, v+49, x}, \
	{TW_COS, v+50, x}, {TW_COS, v+50, x}, {TW_COS, v+51, x}, {TW_COS, v+51, x}, \
	{TW_COS, v+52, x}, {TW_COS, v+52, x}, {TW_COS, v+53, x}, {TW_COS, v+53, x}, \
	{TW_COS, v+54, x}, {TW_COS, v+54, x}, {TW_COS, v+55, x}, {TW_COS, v+55, x}, \
	{TW_COS, v+56, x}, {TW_COS, v+56, x}, {TW_COS, v+57, x}, {TW_COS, v+57, x}, \
	{TW_COS, v+58, x}, {TW_COS, v+58, x}, {TW_COS, v+59, x}, {TW_COS, v+59, x}, \
	{TW_COS, v+60, x}, {TW_COS, v+60, x}, {TW_COS, v+61, x}, {TW_COS, v+61, x}, \
	{TW_COS, v+62, x}, {TW_COS, v+62, x}, {TW_COS, v+63, x}, {TW_COS, v+63, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
	{TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, -x}, {TW_SIN, v+8, x}, {TW_SIN, v+9, -x}, {TW_SIN, v+9, x}, \
	{TW_SIN, v+10, -x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, -x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, -x}, {TW_SIN, v+12, x}, {TW_SIN, v+13, -x}, {TW_SIN, v+13, x}, \
	{TW_SIN, v+14, -x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, -x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, -x}, {TW_SIN, v+16, x}, {TW_SIN, v+17, -x}, {TW_SIN, v+17, x}, \
	{TW_SIN, v+18, -x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, -x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, -x}, {TW_SIN, v+20, x}, {TW_SIN, v+21, -x}, {TW_SIN, v+21, x}, \
	{TW_SIN, v+22, -x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, -x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, -x}, {TW_SIN, v+24, x}, {TW_SIN, v+25, -x}, {TW_SIN, v+25, x}, \
	{TW_SIN, v+26, -x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, -x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, -x}, {TW_SIN, v+28, x}, {TW_SIN, v+29, -x}, {TW_SIN, v+29, x}, \
	{TW_SIN, v+30, -x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, -x}, {TW_SIN, v+31, x}, \
	{TW_SIN, v+32, -x}, {TW_SIN, v+32, x}, {TW_SIN, v+33, -x}, {TW_SIN, v+33, x}, \
	{TW_SIN, v+34, -x}, {TW_SIN, v+34, x}, {TW_SIN, v+35, -x}, {TW_SIN, v+35, x}, \
	{TW_SIN, v+36, -x}, {TW_SIN, v+36, x}, {TW_SIN, v+37, -x}, {TW_SIN, v+37, x}, \
	{TW_SIN, v+38, -x}, {TW_SIN, v+38, x}, {TW_SIN, v+39, -x}, {TW_SIN, v+39, x}, \
	{TW_SIN, v+40, -x}, {TW_SIN, v+40, x}, {TW_SIN, v+41, -x}, {TW_SIN, v+41, x}, \
	{TW_SIN, v+42, -x}, {TW_SIN, v+42, x}, {TW_SIN, v+43, -x}, {TW_SIN, v+43, x}, \
	{TW_SIN, v+44, -x}, {TW_SIN, v+44, x}, {TW_SIN, v+45, -x}, {TW_SIN, v+45, x}, \
	{TW_SIN, v+46, -x}, {TW_SIN, v+46, x}, {TW_SIN, v+47, -x}, {TW_SIN, v+47, x}, \
	{TW_SIN, v+48, -x}, {TW_SIN, v+48, x}, {TW_SIN, v+49, -x}, {TW_SIN, v+49, x}, \
	{TW_SIN, v+50, -x}, {TW_SIN, v+50, x}, {TW_SIN, v+51, -x}, {TW_SIN, v+51, x}, \
	{TW_SIN, v+52, -x}, {TW_SIN, v+52, x}, {TW_SIN, v+53, -x}, {TW_SIN, v+53, x}, \
	{TW_SIN, v+54, -x}, {TW_SIN, v+54, x}, {TW_SIN, v+55, -x}, {TW_SIN, v+55, x}, \
	{TW_SIN, v+56, -x}, {TW_SIN, v+56, x}, {TW_SIN, v+57, -x}, {TW_SIN, v+57, x}, \
	{TW_SIN, v+58, -x}, {TW_SIN, v+58, x}, {TW_SIN, v+59, -x}, {TW_SIN, v+59, x}, \
	{TW_SIN, v+60, -x}, {TW_SIN, v+60, x}, {TW_SIN, v+61, -x}, {TW_SIN, v+61, x}, \
	{TW_SIN, v+62, -x}, {TW_SIN, v+62, x}, {TW_SIN, v+63, -x}, {TW_SIN, v+63, x} 
#endif // VTW_SIZE == 128
#if defined(VTW_SIZE) && VTW_SIZE == 256
#warning "using VTW2 with 256"
#define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
	{TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
	{TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+9, x}, \
	{TW_COS, v+10, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+13, x}, \
	{TW_COS, v+14, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+17, x}, \
	{TW_COS, v+18, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+21, x}, \
	{TW_COS, v+22, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+25, x}, \
	{TW_COS, v+26, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+29, x}, \
	{TW_COS, v+30, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, {TW_COS, v+31, x}, \
	{TW_COS, v+32, x}, {TW_COS, v+32, x}, {TW_COS, v+33, x}, {TW_COS, v+33, x}, \
	{TW_COS, v+34, x}, {TW_COS, v+34, x}, {TW_COS, v+35, x}, {TW_COS, v+35, x}, \
	{TW_COS, v+36, x}, {TW_COS, v+36, x}, {TW_COS, v+37, x}, {TW_COS, v+37, x}, \
	{TW_COS, v+38, x}, {TW_COS, v+38, x}, {TW_COS, v+39, x}, {TW_COS, v+39, x}, \
	{TW_COS, v+40, x}, {TW_COS, v+40, x}, {TW_COS, v+41, x}, {TW_COS, v+41, x}, \
	{TW_COS, v+42, x}, {TW_COS, v+42, x}, {TW_COS, v+43, x}, {TW_COS, v+43, x}, \
	{TW_COS, v+44, x}, {TW_COS, v+44, x}, {TW_COS, v+45, x}, {TW_COS, v+45, x}, \
	{TW_COS, v+46, x}, {TW_COS, v+46, x}, {TW_COS, v+47, x}, {TW_COS, v+47, x}, \
	{TW_COS, v+48, x}, {TW_COS, v+48, x}, {TW_COS, v+49, x}, {TW_COS, v+49, x}, \
	{TW_COS, v+50, x}, {TW_COS, v+50, x}, {TW_COS, v+51, x}, {TW_COS, v+51, x}, \
	{TW_COS, v+52, x}, {TW_COS, v+52, x}, {TW_COS, v+53, x}, {TW_COS, v+53, x}, \
	{TW_COS, v+54, x}, {TW_COS, v+54, x}, {TW_COS, v+55, x}, {TW_COS, v+55, x}, \
	{TW_COS, v+56, x}, {TW_COS, v+56, x}, {TW_COS, v+57, x}, {TW_COS, v+57, x}, \
	{TW_COS, v+58, x}, {TW_COS, v+58, x}, {TW_COS, v+59, x}, {TW_COS, v+59, x}, \
	{TW_COS, v+60, x}, {TW_COS, v+60, x}, {TW_COS, v+61, x}, {TW_COS, v+61, x}, \
	{TW_COS, v+62, x}, {TW_COS, v+62, x}, {TW_COS, v+63, x}, {TW_COS, v+63, x}, \
	{TW_COS, v+64, x}, {TW_COS, v+64, x}, {TW_COS, v+65, x}, {TW_COS, v+65, x}, \
	{TW_COS, v+66, x}, {TW_COS, v+66, x}, {TW_COS, v+67, x}, {TW_COS, v+67, x}, \
	{TW_COS, v+68, x}, {TW_COS, v+68, x}, {TW_COS, v+69, x}, {TW_COS, v+69, x}, \
	{TW_COS, v+70, x}, {TW_COS, v+70, x}, {TW_COS, v+71, x}, {TW_COS, v+71, x}, \
	{TW_COS, v+72, x}, {TW_COS, v+72, x}, {TW_COS, v+73, x}, {TW_COS, v+73, x}, \
	{TW_COS, v+74, x}, {TW_COS, v+74, x}, {TW_COS, v+75, x}, {TW_COS, v+75, x}, \
	{TW_COS, v+76, x}, {TW_COS, v+76, x}, {TW_COS, v+77, x}, {TW_COS, v+77, x}, \
	{TW_COS, v+78, x}, {TW_COS, v+78, x}, {TW_COS, v+79, x}, {TW_COS, v+79, x}, \
	{TW_COS, v+80, x}, {TW_COS, v+80, x}, {TW_COS, v+81, x}, {TW_COS, v+81, x}, \
	{TW_COS, v+82, x}, {TW_COS, v+82, x}, {TW_COS, v+83, x}, {TW_COS, v+83, x}, \
	{TW_COS, v+84, x}, {TW_COS, v+84, x}, {TW_COS, v+85, x}, {TW_COS, v+85, x}, \
	{TW_COS, v+86, x}, {TW_COS, v+86, x}, {TW_COS, v+87, x}, {TW_COS, v+87, x}, \
	{TW_COS, v+88, x}, {TW_COS, v+88, x}, {TW_COS, v+89, x}, {TW_COS, v+89, x}, \
	{TW_COS, v+90, x}, {TW_COS, v+90, x}, {TW_COS, v+91, x}, {TW_COS, v+91, x}, \
	{TW_COS, v+92, x}, {TW_COS, v+92, x}, {TW_COS, v+93, x}, {TW_COS, v+93, x}, \
	{TW_COS, v+94, x}, {TW_COS, v+94, x}, {TW_COS, v+95, x}, {TW_COS, v+95, x}, \
	{TW_COS, v+96, x}, {TW_COS, v+96, x}, {TW_COS, v+97, x}, {TW_COS, v+97, x}, \
	{TW_COS, v+98, x}, {TW_COS, v+98, x}, {TW_COS, v+99, x}, {TW_COS, v+99, x}, \
	{TW_COS, v+100, x}, {TW_COS, v+100, x}, {TW_COS, v+101, x}, {TW_COS, v+101, x}, \
	{TW_COS, v+102, x}, {TW_COS, v+102, x}, {TW_COS, v+103, x}, {TW_COS, v+103, x}, \
	{TW_COS, v+104, x}, {TW_COS, v+104, x}, {TW_COS, v+105, x}, {TW_COS, v+105, x}, \
	{TW_COS, v+106, x}, {TW_COS, v+106, x}, {TW_COS, v+107, x}, {TW_COS, v+107, x}, \
	{TW_COS, v+108, x}, {TW_COS, v+108, x}, {TW_COS, v+109, x}, {TW_COS, v+109, x}, \
	{TW_COS, v+110, x}, {TW_COS, v+110, x}, {TW_COS, v+111, x}, {TW_COS, v+111, x}, \
	{TW_COS, v+112, x}, {TW_COS, v+112, x}, {TW_COS, v+113, x}, {TW_COS, v+113, x}, \
	{TW_COS, v+114, x}, {TW_COS, v+114, x}, {TW_COS, v+115, x}, {TW_COS, v+115, x}, \
	{TW_COS, v+116, x}, {TW_COS, v+116, x}, {TW_COS, v+117, x}, {TW_COS, v+117, x}, \
	{TW_COS, v+118, x}, {TW_COS, v+118, x}, {TW_COS, v+119, x}, {TW_COS, v+119, x}, \
	{TW_COS, v+120, x}, {TW_COS, v+120, x}, {TW_COS, v+121, x}, {TW_COS, v+121, x}, \
	{TW_COS, v+122, x}, {TW_COS, v+122, x}, {TW_COS, v+123, x}, {TW_COS, v+123, x}, \
	{TW_COS, v+124, x}, {TW_COS, v+124, x}, {TW_COS, v+125, x}, {TW_COS, v+125, x}, \
	{TW_COS, v+126, x}, {TW_COS, v+126, x}, {TW_COS, v+127, x}, {TW_COS, v+127, x}, \
	{TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
	{TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
	{TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, -x}, {TW_SIN, v+8, x}, {TW_SIN, v+9, -x}, {TW_SIN, v+9, x}, \
	{TW_SIN, v+10, -x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, -x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, -x}, {TW_SIN, v+12, x}, {TW_SIN, v+13, -x}, {TW_SIN, v+13, x}, \
	{TW_SIN, v+14, -x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, -x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, -x}, {TW_SIN, v+16, x}, {TW_SIN, v+17, -x}, {TW_SIN, v+17, x}, \
	{TW_SIN, v+18, -x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, -x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, -x}, {TW_SIN, v+20, x}, {TW_SIN, v+21, -x}, {TW_SIN, v+21, x}, \
	{TW_SIN, v+22, -x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, -x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, -x}, {TW_SIN, v+24, x}, {TW_SIN, v+25, -x}, {TW_SIN, v+25, x}, \
	{TW_SIN, v+26, -x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, -x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, -x}, {TW_SIN, v+28, x}, {TW_SIN, v+29, -x}, {TW_SIN, v+29, x}, \
	{TW_SIN, v+30, -x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, -x}, {TW_SIN, v+31, x}, \
	{TW_SIN, v+32, -x}, {TW_SIN, v+32, x}, {TW_SIN, v+33, -x}, {TW_SIN, v+33, x}, \
	{TW_SIN, v+34, -x}, {TW_SIN, v+34, x}, {TW_SIN, v+35, -x}, {TW_SIN, v+35, x}, \
	{TW_SIN, v+36, -x}, {TW_SIN, v+36, x}, {TW_SIN, v+37, -x}, {TW_SIN, v+37, x}, \
	{TW_SIN, v+38, -x}, {TW_SIN, v+38, x}, {TW_SIN, v+39, -x}, {TW_SIN, v+39, x}, \
	{TW_SIN, v+40, -x}, {TW_SIN, v+40, x}, {TW_SIN, v+41, -x}, {TW_SIN, v+41, x}, \
	{TW_SIN, v+42, -x}, {TW_SIN, v+42, x}, {TW_SIN, v+43, -x}, {TW_SIN, v+43, x}, \
	{TW_SIN, v+44, -x}, {TW_SIN, v+44, x}, {TW_SIN, v+45, -x}, {TW_SIN, v+45, x}, \
	{TW_SIN, v+46, -x}, {TW_SIN, v+46, x}, {TW_SIN, v+47, -x}, {TW_SIN, v+47, x}, \
	{TW_SIN, v+48, -x}, {TW_SIN, v+48, x}, {TW_SIN, v+49, -x}, {TW_SIN, v+49, x}, \
	{TW_SIN, v+50, -x}, {TW_SIN, v+50, x}, {TW_SIN, v+51, -x}, {TW_SIN, v+51, x}, \
	{TW_SIN, v+52, -x}, {TW_SIN, v+52, x}, {TW_SIN, v+53, -x}, {TW_SIN, v+53, x}, \
	{TW_SIN, v+54, -x}, {TW_SIN, v+54, x}, {TW_SIN, v+55, -x}, {TW_SIN, v+55, x}, \
	{TW_SIN, v+56, -x}, {TW_SIN, v+56, x}, {TW_SIN, v+57, -x}, {TW_SIN, v+57, x}, \
	{TW_SIN, v+58, -x}, {TW_SIN, v+58, x}, {TW_SIN, v+59, -x}, {TW_SIN, v+59, x}, \
	{TW_SIN, v+60, -x}, {TW_SIN, v+60, x}, {TW_SIN, v+61, -x}, {TW_SIN, v+61, x}, \
	{TW_SIN, v+62, -x}, {TW_SIN, v+62, x}, {TW_SIN, v+63, -x}, {TW_SIN, v+63, x}, \
	{TW_SIN, v+64, -x}, {TW_SIN, v+64, x}, {TW_SIN, v+65, -x}, {TW_SIN, v+65, x}, \
	{TW_SIN, v+66, -x}, {TW_SIN, v+66, x}, {TW_SIN, v+67, -x}, {TW_SIN, v+67, x}, \
	{TW_SIN, v+68, -x}, {TW_SIN, v+68, x}, {TW_SIN, v+69, -x}, {TW_SIN, v+69, x}, \
	{TW_SIN, v+70, -x}, {TW_SIN, v+70, x}, {TW_SIN, v+71, -x}, {TW_SIN, v+71, x}, \
	{TW_SIN, v+72, -x}, {TW_SIN, v+72, x}, {TW_SIN, v+73, -x}, {TW_SIN, v+73, x}, \
	{TW_SIN, v+74, -x}, {TW_SIN, v+74, x}, {TW_SIN, v+75, -x}, {TW_SIN, v+75, x}, \
	{TW_SIN, v+76, -x}, {TW_SIN, v+76, x}, {TW_SIN, v+77, -x}, {TW_SIN, v+77, x}, \
	{TW_SIN, v+78, -x}, {TW_SIN, v+78, x}, {TW_SIN, v+79, -x}, {TW_SIN, v+79, x}, \
	{TW_SIN, v+80, -x}, {TW_SIN, v+80, x}, {TW_SIN, v+81, -x}, {TW_SIN, v+81, x}, \
	{TW_SIN, v+82, -x}, {TW_SIN, v+82, x}, {TW_SIN, v+83, -x}, {TW_SIN, v+83, x}, \
	{TW_SIN, v+84, -x}, {TW_SIN, v+84, x}, {TW_SIN, v+85, -x}, {TW_SIN, v+85, x}, \
	{TW_SIN, v+86, -x}, {TW_SIN, v+86, x}, {TW_SIN, v+87, -x}, {TW_SIN, v+87, x}, \
	{TW_SIN, v+88, -x}, {TW_SIN, v+88, x}, {TW_SIN, v+89, -x}, {TW_SIN, v+89, x}, \
	{TW_SIN, v+90, -x}, {TW_SIN, v+90, x}, {TW_SIN, v+91, -x}, {TW_SIN, v+91, x}, \
	{TW_SIN, v+92, -x}, {TW_SIN, v+92, x}, {TW_SIN, v+93, -x}, {TW_SIN, v+93, x}, \
	{TW_SIN, v+94, -x}, {TW_SIN, v+94, x}, {TW_SIN, v+95, -x}, {TW_SIN, v+95, x}, \
	{TW_SIN, v+96, -x}, {TW_SIN, v+96, x}, {TW_SIN, v+97, -x}, {TW_SIN, v+97, x}, \
	{TW_SIN, v+98, -x}, {TW_SIN, v+98, x}, {TW_SIN, v+99, -x}, {TW_SIN, v+99, x}, \
	{TW_SIN, v+100, -x}, {TW_SIN, v+100, x}, {TW_SIN, v+101, -x}, {TW_SIN, v+101, x}, \
	{TW_SIN, v+102, -x}, {TW_SIN, v+102, x}, {TW_SIN, v+103, -x}, {TW_SIN, v+103, x}, \
	{TW_SIN, v+104, -x}, {TW_SIN, v+104, x}, {TW_SIN, v+105, -x}, {TW_SIN, v+105, x}, \
	{TW_SIN, v+106, -x}, {TW_SIN, v+106, x}, {TW_SIN, v+107, -x}, {TW_SIN, v+107, x}, \
	{TW_SIN, v+108, -x}, {TW_SIN, v+108, x}, {TW_SIN, v+109, -x}, {TW_SIN, v+109, x}, \
	{TW_SIN, v+110, -x}, {TW_SIN, v+110, x}, {TW_SIN, v+111, -x}, {TW_SIN, v+111, x}, \
	{TW_SIN, v+112, -x}, {TW_SIN, v+112, x}, {TW_SIN, v+113, -x}, {TW_SIN, v+113, x}, \
	{TW_SIN, v+114, -x}, {TW_SIN, v+114, x}, {TW_SIN, v+115, -x}, {TW_SIN, v+115, x}, \
	{TW_SIN, v+116, -x}, {TW_SIN, v+116, x}, {TW_SIN, v+117, -x}, {TW_SIN, v+117, x}, \
	{TW_SIN, v+118, -x}, {TW_SIN, v+118, x}, {TW_SIN, v+119, -x}, {TW_SIN, v+119, x}, \
	{TW_SIN, v+120, -x}, {TW_SIN, v+120, x}, {TW_SIN, v+121, -x}, {TW_SIN, v+121, x}, \
	{TW_SIN, v+122, -x}, {TW_SIN, v+122, x}, {TW_SIN, v+123, -x}, {TW_SIN, v+123, x}, \
	{TW_SIN, v+124, -x}, {TW_SIN, v+124, x}, {TW_SIN, v+125, -x}, {TW_SIN, v+125, x}, \
	{TW_SIN, v+126, -x}, {TW_SIN, v+126, x}, {TW_SIN, v+127, -x}, {TW_SIN, v+127, x} 
#endif // VTW_SIZE == 256
#endif // REQ_VTW2
#if defined(REQ_VTWS)
#if defined(VTW_SIZE) && VTW_SIZE == 1
#warning "using VTWS with 1"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_SIN, v+0, x} 
#endif // VTW_SIZE == 1
#if defined(VTW_SIZE) && VTW_SIZE == 2
#warning "using VTWS with 2"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, x} 
#endif // VTW_SIZE == 2
#if defined(VTW_SIZE) && VTW_SIZE == 4
#warning "using VTWS with 4"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x} 
#endif // VTW_SIZE == 4
#if defined(VTW_SIZE) && VTW_SIZE == 8
#warning "using VTWS with 8"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x} 
#endif // VTW_SIZE == 8
#if defined(VTW_SIZE) && VTW_SIZE == 16
#warning "using VTWS with 16"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x} 
#endif // VTW_SIZE == 16
#if defined(VTW_SIZE) && VTW_SIZE == 32
#warning "using VTWS with 32"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, x}, {TW_SIN, v+17, x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, x}, {TW_SIN, v+21, x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, x}, {TW_SIN, v+25, x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, x}, {TW_SIN, v+29, x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, x} 
#endif // VTW_SIZE == 32
#if defined(VTW_SIZE) && VTW_SIZE == 64
#warning "using VTWS with 64"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, \
	{TW_COS, v+32, x}, {TW_COS, v+33, x}, {TW_COS, v+34, x}, {TW_COS, v+35, x}, \
	{TW_COS, v+36, x}, {TW_COS, v+37, x}, {TW_COS, v+38, x}, {TW_COS, v+39, x}, \
	{TW_COS, v+40, x}, {TW_COS, v+41, x}, {TW_COS, v+42, x}, {TW_COS, v+43, x}, \
	{TW_COS, v+44, x}, {TW_COS, v+45, x}, {TW_COS, v+46, x}, {TW_COS, v+47, x}, \
	{TW_COS, v+48, x}, {TW_COS, v+49, x}, {TW_COS, v+50, x}, {TW_COS, v+51, x}, \
	{TW_COS, v+52, x}, {TW_COS, v+53, x}, {TW_COS, v+54, x}, {TW_COS, v+55, x}, \
	{TW_COS, v+56, x}, {TW_COS, v+57, x}, {TW_COS, v+58, x}, {TW_COS, v+59, x}, \
	{TW_COS, v+60, x}, {TW_COS, v+61, x}, {TW_COS, v+62, x}, {TW_COS, v+63, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, x}, {TW_SIN, v+17, x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, x}, {TW_SIN, v+21, x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, x}, {TW_SIN, v+25, x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, x}, {TW_SIN, v+29, x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, x}, \
	{TW_SIN, v+32, x}, {TW_SIN, v+33, x}, {TW_SIN, v+34, x}, {TW_SIN, v+35, x}, \
	{TW_SIN, v+36, x}, {TW_SIN, v+37, x}, {TW_SIN, v+38, x}, {TW_SIN, v+39, x}, \
	{TW_SIN, v+40, x}, {TW_SIN, v+41, x}, {TW_SIN, v+42, x}, {TW_SIN, v+43, x}, \
	{TW_SIN, v+44, x}, {TW_SIN, v+45, x}, {TW_SIN, v+46, x}, {TW_SIN, v+47, x}, \
	{TW_SIN, v+48, x}, {TW_SIN, v+49, x}, {TW_SIN, v+50, x}, {TW_SIN, v+51, x}, \
	{TW_SIN, v+52, x}, {TW_SIN, v+53, x}, {TW_SIN, v+54, x}, {TW_SIN, v+55, x}, \
	{TW_SIN, v+56, x}, {TW_SIN, v+57, x}, {TW_SIN, v+58, x}, {TW_SIN, v+59, x}, \
	{TW_SIN, v+60, x}, {TW_SIN, v+61, x}, {TW_SIN, v+62, x}, {TW_SIN, v+63, x} 
#endif // VTW_SIZE == 64
#if defined(VTW_SIZE) && VTW_SIZE == 128
#warning "using VTWS with 128"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, \
	{TW_COS, v+32, x}, {TW_COS, v+33, x}, {TW_COS, v+34, x}, {TW_COS, v+35, x}, \
	{TW_COS, v+36, x}, {TW_COS, v+37, x}, {TW_COS, v+38, x}, {TW_COS, v+39, x}, \
	{TW_COS, v+40, x}, {TW_COS, v+41, x}, {TW_COS, v+42, x}, {TW_COS, v+43, x}, \
	{TW_COS, v+44, x}, {TW_COS, v+45, x}, {TW_COS, v+46, x}, {TW_COS, v+47, x}, \
	{TW_COS, v+48, x}, {TW_COS, v+49, x}, {TW_COS, v+50, x}, {TW_COS, v+51, x}, \
	{TW_COS, v+52, x}, {TW_COS, v+53, x}, {TW_COS, v+54, x}, {TW_COS, v+55, x}, \
	{TW_COS, v+56, x}, {TW_COS, v+57, x}, {TW_COS, v+58, x}, {TW_COS, v+59, x}, \
	{TW_COS, v+60, x}, {TW_COS, v+61, x}, {TW_COS, v+62, x}, {TW_COS, v+63, x}, \
	{TW_COS, v+64, x}, {TW_COS, v+65, x}, {TW_COS, v+66, x}, {TW_COS, v+67, x}, \
	{TW_COS, v+68, x}, {TW_COS, v+69, x}, {TW_COS, v+70, x}, {TW_COS, v+71, x}, \
	{TW_COS, v+72, x}, {TW_COS, v+73, x}, {TW_COS, v+74, x}, {TW_COS, v+75, x}, \
	{TW_COS, v+76, x}, {TW_COS, v+77, x}, {TW_COS, v+78, x}, {TW_COS, v+79, x}, \
	{TW_COS, v+80, x}, {TW_COS, v+81, x}, {TW_COS, v+82, x}, {TW_COS, v+83, x}, \
	{TW_COS, v+84, x}, {TW_COS, v+85, x}, {TW_COS, v+86, x}, {TW_COS, v+87, x}, \
	{TW_COS, v+88, x}, {TW_COS, v+89, x}, {TW_COS, v+90, x}, {TW_COS, v+91, x}, \
	{TW_COS, v+92, x}, {TW_COS, v+93, x}, {TW_COS, v+94, x}, {TW_COS, v+95, x}, \
	{TW_COS, v+96, x}, {TW_COS, v+97, x}, {TW_COS, v+98, x}, {TW_COS, v+99, x}, \
	{TW_COS, v+100, x}, {TW_COS, v+101, x}, {TW_COS, v+102, x}, {TW_COS, v+103, x}, \
	{TW_COS, v+104, x}, {TW_COS, v+105, x}, {TW_COS, v+106, x}, {TW_COS, v+107, x}, \
	{TW_COS, v+108, x}, {TW_COS, v+109, x}, {TW_COS, v+110, x}, {TW_COS, v+111, x}, \
	{TW_COS, v+112, x}, {TW_COS, v+113, x}, {TW_COS, v+114, x}, {TW_COS, v+115, x}, \
	{TW_COS, v+116, x}, {TW_COS, v+117, x}, {TW_COS, v+118, x}, {TW_COS, v+119, x}, \
	{TW_COS, v+120, x}, {TW_COS, v+121, x}, {TW_COS, v+122, x}, {TW_COS, v+123, x}, \
	{TW_COS, v+124, x}, {TW_COS, v+125, x}, {TW_COS, v+126, x}, {TW_COS, v+127, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, x}, {TW_SIN, v+17, x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, x}, {TW_SIN, v+21, x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, x}, {TW_SIN, v+25, x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, x}, {TW_SIN, v+29, x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, x}, \
	{TW_SIN, v+32, x}, {TW_SIN, v+33, x}, {TW_SIN, v+34, x}, {TW_SIN, v+35, x}, \
	{TW_SIN, v+36, x}, {TW_SIN, v+37, x}, {TW_SIN, v+38, x}, {TW_SIN, v+39, x}, \
	{TW_SIN, v+40, x}, {TW_SIN, v+41, x}, {TW_SIN, v+42, x}, {TW_SIN, v+43, x}, \
	{TW_SIN, v+44, x}, {TW_SIN, v+45, x}, {TW_SIN, v+46, x}, {TW_SIN, v+47, x}, \
	{TW_SIN, v+48, x}, {TW_SIN, v+49, x}, {TW_SIN, v+50, x}, {TW_SIN, v+51, x}, \
	{TW_SIN, v+52, x}, {TW_SIN, v+53, x}, {TW_SIN, v+54, x}, {TW_SIN, v+55, x}, \
	{TW_SIN, v+56, x}, {TW_SIN, v+57, x}, {TW_SIN, v+58, x}, {TW_SIN, v+59, x}, \
	{TW_SIN, v+60, x}, {TW_SIN, v+61, x}, {TW_SIN, v+62, x}, {TW_SIN, v+63, x}, \
	{TW_SIN, v+64, x}, {TW_SIN, v+65, x}, {TW_SIN, v+66, x}, {TW_SIN, v+67, x}, \
	{TW_SIN, v+68, x}, {TW_SIN, v+69, x}, {TW_SIN, v+70, x}, {TW_SIN, v+71, x}, \
	{TW_SIN, v+72, x}, {TW_SIN, v+73, x}, {TW_SIN, v+74, x}, {TW_SIN, v+75, x}, \
	{TW_SIN, v+76, x}, {TW_SIN, v+77, x}, {TW_SIN, v+78, x}, {TW_SIN, v+79, x}, \
	{TW_SIN, v+80, x}, {TW_SIN, v+81, x}, {TW_SIN, v+82, x}, {TW_SIN, v+83, x}, \
	{TW_SIN, v+84, x}, {TW_SIN, v+85, x}, {TW_SIN, v+86, x}, {TW_SIN, v+87, x}, \
	{TW_SIN, v+88, x}, {TW_SIN, v+89, x}, {TW_SIN, v+90, x}, {TW_SIN, v+91, x}, \
	{TW_SIN, v+92, x}, {TW_SIN, v+93, x}, {TW_SIN, v+94, x}, {TW_SIN, v+95, x}, \
	{TW_SIN, v+96, x}, {TW_SIN, v+97, x}, {TW_SIN, v+98, x}, {TW_SIN, v+99, x}, \
	{TW_SIN, v+100, x}, {TW_SIN, v+101, x}, {TW_SIN, v+102, x}, {TW_SIN, v+103, x}, \
	{TW_SIN, v+104, x}, {TW_SIN, v+105, x}, {TW_SIN, v+106, x}, {TW_SIN, v+107, x}, \
	{TW_SIN, v+108, x}, {TW_SIN, v+109, x}, {TW_SIN, v+110, x}, {TW_SIN, v+111, x}, \
	{TW_SIN, v+112, x}, {TW_SIN, v+113, x}, {TW_SIN, v+114, x}, {TW_SIN, v+115, x}, \
	{TW_SIN, v+116, x}, {TW_SIN, v+117, x}, {TW_SIN, v+118, x}, {TW_SIN, v+119, x}, \
	{TW_SIN, v+120, x}, {TW_SIN, v+121, x}, {TW_SIN, v+122, x}, {TW_SIN, v+123, x}, \
	{TW_SIN, v+124, x}, {TW_SIN, v+125, x}, {TW_SIN, v+126, x}, {TW_SIN, v+127, x} 
#endif // VTW_SIZE == 128
#if defined(VTW_SIZE) && VTW_SIZE == 256
#warning "using VTWS with 256"
#define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
	{TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
	{TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
	{TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
	{TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, \
	{TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, \
	{TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, \
	{TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, \
	{TW_COS, v+32, x}, {TW_COS, v+33, x}, {TW_COS, v+34, x}, {TW_COS, v+35, x}, \
	{TW_COS, v+36, x}, {TW_COS, v+37, x}, {TW_COS, v+38, x}, {TW_COS, v+39, x}, \
	{TW_COS, v+40, x}, {TW_COS, v+41, x}, {TW_COS, v+42, x}, {TW_COS, v+43, x}, \
	{TW_COS, v+44, x}, {TW_COS, v+45, x}, {TW_COS, v+46, x}, {TW_COS, v+47, x}, \
	{TW_COS, v+48, x}, {TW_COS, v+49, x}, {TW_COS, v+50, x}, {TW_COS, v+51, x}, \
	{TW_COS, v+52, x}, {TW_COS, v+53, x}, {TW_COS, v+54, x}, {TW_COS, v+55, x}, \
	{TW_COS, v+56, x}, {TW_COS, v+57, x}, {TW_COS, v+58, x}, {TW_COS, v+59, x}, \
	{TW_COS, v+60, x}, {TW_COS, v+61, x}, {TW_COS, v+62, x}, {TW_COS, v+63, x}, \
	{TW_COS, v+64, x}, {TW_COS, v+65, x}, {TW_COS, v+66, x}, {TW_COS, v+67, x}, \
	{TW_COS, v+68, x}, {TW_COS, v+69, x}, {TW_COS, v+70, x}, {TW_COS, v+71, x}, \
	{TW_COS, v+72, x}, {TW_COS, v+73, x}, {TW_COS, v+74, x}, {TW_COS, v+75, x}, \
	{TW_COS, v+76, x}, {TW_COS, v+77, x}, {TW_COS, v+78, x}, {TW_COS, v+79, x}, \
	{TW_COS, v+80, x}, {TW_COS, v+81, x}, {TW_COS, v+82, x}, {TW_COS, v+83, x}, \
	{TW_COS, v+84, x}, {TW_COS, v+85, x}, {TW_COS, v+86, x}, {TW_COS, v+87, x}, \
	{TW_COS, v+88, x}, {TW_COS, v+89, x}, {TW_COS, v+90, x}, {TW_COS, v+91, x}, \
	{TW_COS, v+92, x}, {TW_COS, v+93, x}, {TW_COS, v+94, x}, {TW_COS, v+95, x}, \
	{TW_COS, v+96, x}, {TW_COS, v+97, x}, {TW_COS, v+98, x}, {TW_COS, v+99, x}, \
	{TW_COS, v+100, x}, {TW_COS, v+101, x}, {TW_COS, v+102, x}, {TW_COS, v+103, x}, \
	{TW_COS, v+104, x}, {TW_COS, v+105, x}, {TW_COS, v+106, x}, {TW_COS, v+107, x}, \
	{TW_COS, v+108, x}, {TW_COS, v+109, x}, {TW_COS, v+110, x}, {TW_COS, v+111, x}, \
	{TW_COS, v+112, x}, {TW_COS, v+113, x}, {TW_COS, v+114, x}, {TW_COS, v+115, x}, \
	{TW_COS, v+116, x}, {TW_COS, v+117, x}, {TW_COS, v+118, x}, {TW_COS, v+119, x}, \
	{TW_COS, v+120, x}, {TW_COS, v+121, x}, {TW_COS, v+122, x}, {TW_COS, v+123, x}, \
	{TW_COS, v+124, x}, {TW_COS, v+125, x}, {TW_COS, v+126, x}, {TW_COS, v+127, x}, \
	{TW_COS, v+128, x}, {TW_COS, v+129, x}, {TW_COS, v+130, x}, {TW_COS, v+131, x}, \
	{TW_COS, v+132, x}, {TW_COS, v+133, x}, {TW_COS, v+134, x}, {TW_COS, v+135, x}, \
	{TW_COS, v+136, x}, {TW_COS, v+137, x}, {TW_COS, v+138, x}, {TW_COS, v+139, x}, \
	{TW_COS, v+140, x}, {TW_COS, v+141, x}, {TW_COS, v+142, x}, {TW_COS, v+143, x}, \
	{TW_COS, v+144, x}, {TW_COS, v+145, x}, {TW_COS, v+146, x}, {TW_COS, v+147, x}, \
	{TW_COS, v+148, x}, {TW_COS, v+149, x}, {TW_COS, v+150, x}, {TW_COS, v+151, x}, \
	{TW_COS, v+152, x}, {TW_COS, v+153, x}, {TW_COS, v+154, x}, {TW_COS, v+155, x}, \
	{TW_COS, v+156, x}, {TW_COS, v+157, x}, {TW_COS, v+158, x}, {TW_COS, v+159, x}, \
	{TW_COS, v+160, x}, {TW_COS, v+161, x}, {TW_COS, v+162, x}, {TW_COS, v+163, x}, \
	{TW_COS, v+164, x}, {TW_COS, v+165, x}, {TW_COS, v+166, x}, {TW_COS, v+167, x}, \
	{TW_COS, v+168, x}, {TW_COS, v+169, x}, {TW_COS, v+170, x}, {TW_COS, v+171, x}, \
	{TW_COS, v+172, x}, {TW_COS, v+173, x}, {TW_COS, v+174, x}, {TW_COS, v+175, x}, \
	{TW_COS, v+176, x}, {TW_COS, v+177, x}, {TW_COS, v+178, x}, {TW_COS, v+179, x}, \
	{TW_COS, v+180, x}, {TW_COS, v+181, x}, {TW_COS, v+182, x}, {TW_COS, v+183, x}, \
	{TW_COS, v+184, x}, {TW_COS, v+185, x}, {TW_COS, v+186, x}, {TW_COS, v+187, x}, \
	{TW_COS, v+188, x}, {TW_COS, v+189, x}, {TW_COS, v+190, x}, {TW_COS, v+191, x}, \
	{TW_COS, v+192, x}, {TW_COS, v+193, x}, {TW_COS, v+194, x}, {TW_COS, v+195, x}, \
	{TW_COS, v+196, x}, {TW_COS, v+197, x}, {TW_COS, v+198, x}, {TW_COS, v+199, x}, \
	{TW_COS, v+200, x}, {TW_COS, v+201, x}, {TW_COS, v+202, x}, {TW_COS, v+203, x}, \
	{TW_COS, v+204, x}, {TW_COS, v+205, x}, {TW_COS, v+206, x}, {TW_COS, v+207, x}, \
	{TW_COS, v+208, x}, {TW_COS, v+209, x}, {TW_COS, v+210, x}, {TW_COS, v+211, x}, \
	{TW_COS, v+212, x}, {TW_COS, v+213, x}, {TW_COS, v+214, x}, {TW_COS, v+215, x}, \
	{TW_COS, v+216, x}, {TW_COS, v+217, x}, {TW_COS, v+218, x}, {TW_COS, v+219, x}, \
	{TW_COS, v+220, x}, {TW_COS, v+221, x}, {TW_COS, v+222, x}, {TW_COS, v+223, x}, \
	{TW_COS, v+224, x}, {TW_COS, v+225, x}, {TW_COS, v+226, x}, {TW_COS, v+227, x}, \
	{TW_COS, v+228, x}, {TW_COS, v+229, x}, {TW_COS, v+230, x}, {TW_COS, v+231, x}, \
	{TW_COS, v+232, x}, {TW_COS, v+233, x}, {TW_COS, v+234, x}, {TW_COS, v+235, x}, \
	{TW_COS, v+236, x}, {TW_COS, v+237, x}, {TW_COS, v+238, x}, {TW_COS, v+239, x}, \
	{TW_COS, v+240, x}, {TW_COS, v+241, x}, {TW_COS, v+242, x}, {TW_COS, v+243, x}, \
	{TW_COS, v+244, x}, {TW_COS, v+245, x}, {TW_COS, v+246, x}, {TW_COS, v+247, x}, \
	{TW_COS, v+248, x}, {TW_COS, v+249, x}, {TW_COS, v+250, x}, {TW_COS, v+251, x}, \
	{TW_COS, v+252, x}, {TW_COS, v+253, x}, {TW_COS, v+254, x}, {TW_COS, v+255, x}, \
	{TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
	{TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
	{TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
	{TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}, \
	{TW_SIN, v+16, x}, {TW_SIN, v+17, x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, x}, \
	{TW_SIN, v+20, x}, {TW_SIN, v+21, x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, x}, \
	{TW_SIN, v+24, x}, {TW_SIN, v+25, x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, x}, \
	{TW_SIN, v+28, x}, {TW_SIN, v+29, x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, x}, \
	{TW_SIN, v+32, x}, {TW_SIN, v+33, x}, {TW_SIN, v+34, x}, {TW_SIN, v+35, x}, \
	{TW_SIN, v+36, x}, {TW_SIN, v+37, x}, {TW_SIN, v+38, x}, {TW_SIN, v+39, x}, \
	{TW_SIN, v+40, x}, {TW_SIN, v+41, x}, {TW_SIN, v+42, x}, {TW_SIN, v+43, x}, \
	{TW_SIN, v+44, x}, {TW_SIN, v+45, x}, {TW_SIN, v+46, x}, {TW_SIN, v+47, x}, \
	{TW_SIN, v+48, x}, {TW_SIN, v+49, x}, {TW_SIN, v+50, x}, {TW_SIN, v+51, x}, \
	{TW_SIN, v+52, x}, {TW_SIN, v+53, x}, {TW_SIN, v+54, x}, {TW_SIN, v+55, x}, \
	{TW_SIN, v+56, x}, {TW_SIN, v+57, x}, {TW_SIN, v+58, x}, {TW_SIN, v+59, x}, \
	{TW_SIN, v+60, x}, {TW_SIN, v+61, x}, {TW_SIN, v+62, x}, {TW_SIN, v+63, x}, \
	{TW_SIN, v+64, x}, {TW_SIN, v+65, x}, {TW_SIN, v+66, x}, {TW_SIN, v+67, x}, \
	{TW_SIN, v+68, x}, {TW_SIN, v+69, x}, {TW_SIN, v+70, x}, {TW_SIN, v+71, x}, \
	{TW_SIN, v+72, x}, {TW_SIN, v+73, x}, {TW_SIN, v+74, x}, {TW_SIN, v+75, x}, \
	{TW_SIN, v+76, x}, {TW_SIN, v+77, x}, {TW_SIN, v+78, x}, {TW_SIN, v+79, x}, \
	{TW_SIN, v+80, x}, {TW_SIN, v+81, x}, {TW_SIN, v+82, x}, {TW_SIN, v+83, x}, \
	{TW_SIN, v+84, x}, {TW_SIN, v+85, x}, {TW_SIN, v+86, x}, {TW_SIN, v+87, x}, \
	{TW_SIN, v+88, x}, {TW_SIN, v+89, x}, {TW_SIN, v+90, x}, {TW_SIN, v+91, x}, \
	{TW_SIN, v+92, x}, {TW_SIN, v+93, x}, {TW_SIN, v+94, x}, {TW_SIN, v+95, x}, \
	{TW_SIN, v+96, x}, {TW_SIN, v+97, x}, {TW_SIN, v+98, x}, {TW_SIN, v+99, x}, \
	{TW_SIN, v+100, x}, {TW_SIN, v+101, x}, {TW_SIN, v+102, x}, {TW_SIN, v+103, x}, \
	{TW_SIN, v+104, x}, {TW_SIN, v+105, x}, {TW_SIN, v+106, x}, {TW_SIN, v+107, x}, \
	{TW_SIN, v+108, x}, {TW_SIN, v+109, x}, {TW_SIN, v+110, x}, {TW_SIN, v+111, x}, \
	{TW_SIN, v+112, x}, {TW_SIN, v+113, x}, {TW_SIN, v+114, x}, {TW_SIN, v+115, x}, \
	{TW_SIN, v+116, x}, {TW_SIN, v+117, x}, {TW_SIN, v+118, x}, {TW_SIN, v+119, x}, \
	{TW_SIN, v+120, x}, {TW_SIN, v+121, x}, {TW_SIN, v+122, x}, {TW_SIN, v+123, x}, \
	{TW_SIN, v+124, x}, {TW_SIN, v+125, x}, {TW_SIN, v+126, x}, {TW_SIN, v+127, x}, \
	{TW_SIN, v+128, x}, {TW_SIN, v+129, x}, {TW_SIN, v+130, x}, {TW_SIN, v+131, x}, \
	{TW_SIN, v+132, x}, {TW_SIN, v+133, x}, {TW_SIN, v+134, x}, {TW_SIN, v+135, x}, \
	{TW_SIN, v+136, x}, {TW_SIN, v+137, x}, {TW_SIN, v+138, x}, {TW_SIN, v+139, x}, \
	{TW_SIN, v+140, x}, {TW_SIN, v+141, x}, {TW_SIN, v+142, x}, {TW_SIN, v+143, x}, \
	{TW_SIN, v+144, x}, {TW_SIN, v+145, x}, {TW_SIN, v+146, x}, {TW_SIN, v+147, x}, \
	{TW_SIN, v+148, x}, {TW_SIN, v+149, x}, {TW_SIN, v+150, x}, {TW_SIN, v+151, x}, \
	{TW_SIN, v+152, x}, {TW_SIN, v+153, x}, {TW_SIN, v+154, x}, {TW_SIN, v+155, x}, \
	{TW_SIN, v+156, x}, {TW_SIN, v+157, x}, {TW_SIN, v+158, x}, {TW_SIN, v+159, x}, \
	{TW_SIN, v+160, x}, {TW_SIN, v+161, x}, {TW_SIN, v+162, x}, {TW_SIN, v+163, x}, \
	{TW_SIN, v+164, x}, {TW_SIN, v+165, x}, {TW_SIN, v+166, x}, {TW_SIN, v+167, x}, \
	{TW_SIN, v+168, x}, {TW_SIN, v+169, x}, {TW_SIN, v+170, x}, {TW_SIN, v+171, x}, \
	{TW_SIN, v+172, x}, {TW_SIN, v+173, x}, {TW_SIN, v+174, x}, {TW_SIN, v+175, x}, \
	{TW_SIN, v+176, x}, {TW_SIN, v+177, x}, {TW_SIN, v+178, x}, {TW_SIN, v+179, x}, \
	{TW_SIN, v+180, x}, {TW_SIN, v+181, x}, {TW_SIN, v+182, x}, {TW_SIN, v+183, x}, \
	{TW_SIN, v+184, x}, {TW_SIN, v+185, x}, {TW_SIN, v+186, x}, {TW_SIN, v+187, x}, \
	{TW_SIN, v+188, x}, {TW_SIN, v+189, x}, {TW_SIN, v+190, x}, {TW_SIN, v+191, x}, \
	{TW_SIN, v+192, x}, {TW_SIN, v+193, x}, {TW_SIN, v+194, x}, {TW_SIN, v+195, x}, \
	{TW_SIN, v+196, x}, {TW_SIN, v+197, x}, {TW_SIN, v+198, x}, {TW_SIN, v+199, x}, \
	{TW_SIN, v+200, x}, {TW_SIN, v+201, x}, {TW_SIN, v+202, x}, {TW_SIN, v+203, x}, \
	{TW_SIN, v+204, x}, {TW_SIN, v+205, x}, {TW_SIN, v+206, x}, {TW_SIN, v+207, x}, \
	{TW_SIN, v+208, x}, {TW_SIN, v+209, x}, {TW_SIN, v+210, x}, {TW_SIN, v+211, x}, \
	{TW_SIN, v+212, x}, {TW_SIN, v+213, x}, {TW_SIN, v+214, x}, {TW_SIN, v+215, x}, \
	{TW_SIN, v+216, x}, {TW_SIN, v+217, x}, {TW_SIN, v+218, x}, {TW_SIN, v+219, x}, \
	{TW_SIN, v+220, x}, {TW_SIN, v+221, x}, {TW_SIN, v+222, x}, {TW_SIN, v+223, x}, \
	{TW_SIN, v+224, x}, {TW_SIN, v+225, x}, {TW_SIN, v+226, x}, {TW_SIN, v+227, x}, \
	{TW_SIN, v+228, x}, {TW_SIN, v+229, x}, {TW_SIN, v+230, x}, {TW_SIN, v+231, x}, \
	{TW_SIN, v+232, x}, {TW_SIN, v+233, x}, {TW_SIN, v+234, x}, {TW_SIN, v+235, x}, \
	{TW_SIN, v+236, x}, {TW_SIN, v+237, x}, {TW_SIN, v+238, x}, {TW_SIN, v+239, x}, \
	{TW_SIN, v+240, x}, {TW_SIN, v+241, x}, {TW_SIN, v+242, x}, {TW_SIN, v+243, x}, \
	{TW_SIN, v+244, x}, {TW_SIN, v+245, x}, {TW_SIN, v+246, x}, {TW_SIN, v+247, x}, \
	{TW_SIN, v+248, x}, {TW_SIN, v+249, x}, {TW_SIN, v+250, x}, {TW_SIN, v+251, x}, \
	{TW_SIN, v+252, x}, {TW_SIN, v+253, x}, {TW_SIN, v+254, x}, {TW_SIN, v+255, x} 
#endif // VTW_SIZE == 256
#endif // REQ_VTWS
