
#include "CImg.h"

#define T unsigned int

#define IN_FILE "01275.tif"
#define OUT_FILE "out.png"

int main() {
	
	cimg_library::CImg<T> input_image(IN_FILE);
	
	const unsigned size_x(input_image.dimx());
	const unsigned size_y(input_image.dimy());

	cimg_library::CImg<T> output_image(size_x,size_y);

	T *buffer = new T[size_x*size_y];

	//copy to buffer
	for(unsigned y = 0; y < size_y; y++){
		for(unsigned x = 0; x < size_x; x++){
			buffer[y*size_x+x] = input_image(x,y);
		}
	}

	//copy from buffer
	for(unsigned y = 0; y < size_y; y++){
		for(unsigned x = 0; x < size_x; x++){
			output_image(x,y) = buffer[y*size_x+x];
		}
	}
	
	output_image.save_png(OUT_FILE);
	
	return 0;
	
}
