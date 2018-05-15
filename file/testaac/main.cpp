#include <stdio.h>  
#include "aacdemux.h"
#include "tsfile2.h"
#include "MuxTs.h"
#include "tsmux.h"
#include "Mux.h"
#include "h264demux.h"

int testaac1()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);
	if( !h264handle )
	{
		printf("H264Framer_Init error\n");
		return -1;
	}

	tsfilewriter2 tsfile;
	tsfile.open_file("aac.ts");

	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	uint64_t pts = 0;
	while( 1 )
	{
		int ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			break;
		tsfile.write_aac_pes(frame, length, pts);

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
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

#define BUF_SIZE (1<<20)

static int g_is_ps = 0;
static FILE *g_out_fp;
static TsProgramInfo g_prog_info;
static int g_frame_count = 0;
static uint8_t g_outbuf[BUF_SIZE] = {0};
static TDemux g_demux;
int testaac3()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);

	g_out_fp = fopen("aac.ts", "wb");
	memset(&g_prog_info, 0, sizeof(g_prog_info));
	g_prog_info.program_num = 1;
	g_prog_info.prog[0].stream_num = 2;
	g_prog_info.prog[0].stream[0].type = STREAM_TYPE_AUDIO_AAC;
	g_prog_info.prog[0].stream[1].type = STREAM_TYPE_VIDEO_H264;
	
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

		TEsFrame es = {0};
		es.program_number = 0;
		es.stream_number = 0;
		es.frame = (uint8_t*)frame;
		es.length = length;
		es.is_key = 0;//framelength < 100? 1:0;					// 这里简单处理，认为信息帧（非数据帧）为关键帧。
		if( g_frame_count%100 == 0 )
			es.is_key = 1;

		es.pts = 3600 * g_frame_count;		// 示例中按帧率为25fps累计时间戳。正式使用应根据帧实际的时间戳填写。
		es.ps_pes_length = 8000;

		printf("framelength %d \n",length);
		int outlen = lts_ts_stream(&es, g_outbuf, BUF_SIZE, &g_prog_info);
		if (outlen > 0)
		{
			fwrite(g_outbuf, 1, outlen, g_out_fp);
		}
		
		es.is_key = 0;
		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		es.stream_number = 1;
		es.frame = (uint8_t*)h264frame;
		es.length = framelength;
/*		outlen = lts_ts_stream(&es, g_outbuf, BUF_SIZE, &g_prog_info);
		if (outlen > 0)
		{
			fwrite(g_outbuf, 1, outlen, g_out_fp);
		}
*/

		g_frame_count++;
		usleep(100 * 1000);
	}

	return 0;
}

int main()
{
	return testaac1();

	return testaac2();

	return testaac3();
}