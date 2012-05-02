export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
export LDLIBRARY_PATH=/usr

CXX = g++
OPENCVINC = -bind_at_load `pkg-config --cflags opencv`
OPENCVLIB = `pkg-config --libs opencv`
WFFTINC = `pkg-config --cflags fftw3`
WFFTLIB = `pkg-config --libs fftw3`

all: mosaic

#Compiling Gist Desc stuff
gist.o: gist.c gist.h standalone_image.h
	gcc -c -Wall -g $< $(WFFTINC) -DUSE_GIST -DSTANDALONE_GIST

standalone_image.o: standalone_image.c standalone_image.h
	gcc -c -Wall -g $<

#Compiling All Mosaic Specific

MSquare.o: MSquare.cpp MSquare.h
	g++ -c -Wall -g $< $(OPENCVINC)

ProcessorPipeline.o: ProcessorPipeline.cpp ProcessorPipeline.h gist.o InputImage.o
	g++ -c -Wall -g $< $(OPENCVINC)

InputImage.o: InputImage.cpp InputImage.h
	g++ -c -Wall -g $< $(OPENCVINC)

ImagePool.o: ImagePool.cpp ImagePool.h InputImage.o
	g++ -c -Wall -g $< $(OPENCVINC)

mosaic: main.cpp MSquare.o InputImage.o ImagePool.o gist.o standalone_image.o ProcessorPipeline.o
	g++ -Wall -g -o $@ $^ $(OPENCVINC) $(OPENCVLIB)-I/opt/local/include -L/opt/local/lib -lfftw3f

clean:
	rm -rf *.o mosaic
