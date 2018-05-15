C 			 = c
CPP 		 = cpp
CXX = gcc
CXXFLAGS = -g -I ../tsmux/
LIBS =  -lstdc++ -lpthread

TARGET0 = testaac

TARGETS = $(TARGET0) 

CPPOBJS = aacdemux.o main.o tsfile2.o  MuxTs.o pes.o tsmux.o \
		h264demux.o \
		spspps.o \
		../tsmux/Mux.o \
		../tsmux/Audio.o \
		../tsmux/Video.o \
		../tsmux/FileIo.o \
		../tsmux/Mybs.o \
		../tsmux/Mycrc_32.o \
		../tsmux/Ts.o \


$(TARGET0):$(CPPOBJS) $(COBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CPPOBJS)  $(COBJS) $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS) test.ts aac.ts
