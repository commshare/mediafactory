CXX = g++
OBJS = ../../protocol/librtp/rtph264.o \
		../../protocol/librtsp/rtspdemux.o \
		../../file/libh26x/h264demux.o \
		../../file/libh26x/spspps.o \
		rtspserver.o \
		service.o

CXXFLAGS = -g -W -Wall -O2 -g -Ithirdparty/lib/libst
LIBS = thirdparty/lib/libst/libst.a

service: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf $(OBJS) service