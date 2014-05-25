all: manager

manager: manager.o ImageProcessing.o Comms.o
	g++ ImageProcessing.o Comms.o manager.o -o manager `pkg-config --cflags --libs opencv`

manager.o: manager.cpp ImageProcessing.h Comms.h
	g++ -I mavlink/v1.0 -c manager.cpp `pkg-config --cflags --libs opencv`

ImageProcessing.o: ImageProcessing.h ImageProcessing.cpp
	g++ -c ImageProcessing.cpp `pkg-config --cflags --libs opencv`
	
Comms.o: Comms.h Comms.cpp
	g++ -c Comms.cpp
	

clean:
	rm -f *o manager
