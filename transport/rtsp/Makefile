CXX = g++
OBJS = rtph264.o service.o rtsp.o sps_pps.o
CXXFLAGS = -g -W -Wall -O2 -g -I.
LIBS = lib/libevent.mingw32.a -lws2_32

service: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf *.o service