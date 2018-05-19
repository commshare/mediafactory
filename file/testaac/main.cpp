#include <stdio.h>  
#include <sstream>

#include "aacdemux.h"
#include "tsfile2.h"
#include "h264demux.h"
#include "Mux.h"
#include "ffmpeg_mux.h"

int testaac1()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 1);
	if( !h264handle )
	{
		printf("H264Framer_Init error\n");
		return -1;
	}

	tsfilewriter2 tsfile;
	tsfile.open_file("aac0.ts");

	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	int filecounter = 0;
	int keyframecounter = 0;
	std::stringstream ss;

	uint64_t pts = 0;
	int ret = 0;
	while( 1 )
	{
		ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			break;
		tsfile.write_aac_pes(frame, length, pts);

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		int frametype = h264frame[4]&0x1F;
		printf("frametype %d\n", frametype);
		if( frametype == 7 )
		{
			keyframecounter++;
			if( keyframecounter > 5 )
			{
				keyframecounter = 0;
				pts = 0;
				
				printf("new file \n");
				tsfile.close_file();
				filecounter++;
				ss<<"aac"<<filecounter<<".ts";
				tsfile.open_file(ss.str().c_str());
				ss.str("");
			}
		}
		tsfile.write_h264_pes(h264frame, framelength, pts);

		pts += 3600;
		
		usleep(100 * 1000);
	}

	return 0;
}

int testaac2()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);
	pVideo_Audio_Ts_File = OpenFile("h264_aac.ts", "wb");
	
	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	uint64_t pts = 0;
	while( 1 )
	{
		int ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			continue;

		WriteBuf2TsFile(25, 0, (unsigned char*)frame, length, pts);
		
		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
//		WriteBuf2TsFile(25, 1, (unsigned char*)h264frame, framelength, pts);

		pts += 3600;
		usleep(100 * 1000);
	}

	return 0;
}

int testaac3()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);

	void* muxhandle = ffmpegmux_alloc("mux.ts");
	ffmpegmux_addvideostream(muxhandle, 352, 288);
	ffmpegmux_addaudiostream(muxhandle, 352, 288);
	printf("ffmpegmux_addvideostream \n");
	ffmpegmux_open(muxhandle);
	printf("AACDemux_GetFrame 1\n");
	printf("ffmpegmux_open \n");
	
	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	uint64_t pts = 0;
	while( 1 )
	{
		int ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			continue;
		printf("ffmpegmux_write_audio frame \n");
		ffmpegmux_write_frame(muxhandle, 1, frame, length, pts);
		
		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		printf("ffmpegmux_write_video frame \n");
		ffmpegmux_write_frame(muxhandle, 0, h264frame, framelength, pts);

		pts += 3600;
		usleep(100 * 1000);
	}

	return 0;
}

int main()
{
//	return testaac1();

//	return testaac2();

	return testaac3();
}