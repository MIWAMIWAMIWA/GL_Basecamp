
ifeq ($(UNAME), Win)
CXX_LIBS = -lpthread -lws2_32
else
CXX_LIBS = -lpthread 
endif



libtcp.a:
	g++ -c -I. network/source/*.cpp -std=c++17
	ar cr libtcp.a ./*.o
	rm -rf ./*.o

libmouse.a:
	g++ -c -I. platform/*.cpp -std=c++17
	ar cr libmouse.a ./*.o
	rm -rf ./*.o

client: libtcp.a libmouse.a
	g++ mainclient.cpp -I. -L. -o test_client -L./ -ltcp $(CXX_LIBS) -std=c++17 -lX11

server: libtcp.a
	g++ mainserver.cpp -I. -L. -o test_server -L./ -ltcp $(CXX_LIBS) -std=c++17

lib: libtcp.a libmouse.a

all: client server

clean:
	rm -rf ./test_* ./*.a *.o