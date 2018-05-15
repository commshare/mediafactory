CXX = g++
CXXFLAGS = -g
LIBS =  -L/usr/local/lib 

TARGET4 = h264tots

TARGETS = $(TARGET0) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) \
		$(TARGET6) $(TARGET7) $(TARGET8)

OBJS = libh26x/h264demux.o libh26x/spspps.o \
		libaac/aacdemux.o libmpeg/pes.o \
		libmpeg/tsmux.o \


$(TARGET4):$(OBJS) $(TARGET4).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET4).o $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS)
