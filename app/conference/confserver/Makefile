CXX = g++
OBJS = ../../../protocol/librtp/rtpmux_h264.o \
		../../../protocol/librtp/rtpdemux_h264.o \
		../../../protocol/librtp/rtpmux_aac.o \
		../../../protocol/librtp/rtpmux_raw.o \
		../../../protocol/librtp/rtp.o \
		../../../protocol/librtsp/rtspdemux.o \
		../../../file/libh26x/spspps.o \
		../../../file/libaac/aacdemux.o \
		../../../transport/udp/udpclient.o \
		../../../transport/udp/udpselectserver.o \
		../../../util/filewritter.o \
		muxer.o \
		main.o

CXXFLAGS = -g -W -Wall -std=c++11 -lpthread -Ithirdparty/lib/libst
LIBS = -lstdc++
ifeq ($(OS),Windows_NT)
	LIBS = -lws2_32
endif

confserver: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf $(OBJS) confserver
