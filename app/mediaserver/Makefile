CXX = g++
OBJS = ../../protocol/librtp/rtph264.o \
		../../protocol/librtsp/rtspdemux.o \
		../../file/libh26x/h264demux.o \
		../../file/libh26x/spspps.o \
		../../transport/tcp/tcpserver.o \
		../../transport/udp/udpclient.o \
		rtspserver.o \
		main.o

CXXFLAGS = -g -W -Wall -O2 -g -std=c++11 -lpthread -Ithirdparty/lib/libst
LIBS = thirdparty/lib/libst/libst.a -lstdc++

service: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf $(OBJS) service
