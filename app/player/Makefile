CXX = g++
CXXFLAGS = -g -w
LIBS =-lpthread -lSDL2 -lavutil -lavformat -lavcodec -lswresample -lswscale

TARGET0 = main
TARGETS = $(TARGET0)

OBJS0 = sdlplay.o player.o ffmpegscale.o ffmpegresample.o \
		ffmpegdemux.o ffmpegdec.o

$(TARGET0):$(OBJS0) $(TARGET0).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS0) $(TARGET0).o $(LIBS) 


clean:
	clear
	rm -rf $(OBJS0) *.o $(TARGETS)
