#include <fstream>
#include "audio_generator.h"
#include "video_generator.h"
#include "ffmpeg_enc.h"
#include "ffmpegresample.h"
#include "ffmpeg_mux.h"
#include "ffmpeg_filter.h"

//ffplay -ar 16000 -channels 1 -f s16le -i xxx.pcm
int testaac3()
{
	void* handle = audio_generator_alloc(44100, 16, 2, 1024);
	std::fstream fs;
	fs.open("1.pcm", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		audio_generator_get_audio_frame(handle, &frame, &length);
		if( length > 0 )
		{
			fs.write(frame,length);
		}

		usleep(100 * 1000);
	}

	return 0;
}

//ffplay -f rawvideo -video_size 1280x720 xxx.yuv
int testaac4()
{
	void* handle = video_generator_alloc(352, 288, 1);
	std::fstream fs;
	fs.open("1.yuv", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		video_generator_get_yuv420p_frame(handle, &frame, &length);
		if( length > 0 )
		{
			fs.write(frame,length);
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac5()
{
	void* handle = audio_generator_alloc(44100, 16, 2, 1024);
	void *resamplehandle = resample_open(2, 1, 64000);

	void* enchandle = ffmpeg_enc_alloc();
	ffmpeg_enc_set_audio(enchandle, "libfdk_aac", 44100, 1024, 2);

	std::fstream fs;
	fs.open("1.aac", std::ios::binary|std::ios::out);

	sound_resampled resample;	
	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		audio_generator_get_audio_frame(handle, &frame, &length);
		if( length > 0 )
		{
			const char *packet = NULL;
			int packetlength = 0;
			ffmpeg_enc_encode_audio(enchandle, (const char*)frame, length, &packet, &packetlength);
			if( packetlength > 0 )
				fs.write(packet,packetlength);				
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac6()
{
	void* handle = video_generator_alloc(352, 288, 1);

	void* enchandle = ffmpeg_enc_alloc();
	ffmpeg_enc_set_video(enchandle, "libx264", 352, 288, 2);

	std::fstream fs;
	fs.open("1.h264", std::ios::binary|std::ios::out);

	sound_resampled resample;	
	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		video_generator_get_yuv420p_frame(handle, &frame, &length);
		if( length > 0 )
		{
			const char *packet = NULL;
			int packetlength = 0;
			ffmpeg_enc_encode_video(enchandle, frame, length, &packet, &packetlength);
			if( packetlength > 0 )
				fs.write(packet,packetlength);				
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac7()
{
	void* filterhandle = ffmpeg_filter_alloc();
	void *filter1 = ffmpeg_filter_alloc_filter(filterhandle, "testsrc", "testsrc", NULL);
	if( !filter1 )
	{
		printf("alloc testsrc error \n");
		return -1;
	}
	void *filter2 = ffmpeg_filter_alloc_filter(filterhandle, "buffersink", "buffersink", NULL);
	ffmpeg_filter_link_filter(filter1, filter2);
	ffmpeg_filter_set_video_sink_filter(filterhandle, filter2);
	ffmpeg_filter_open(filterhandle);

	std::fstream fs;
	fs.open("1.h264", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		printf("ffmpeg_filter_get_video_data \n");
		while( ffmpeg_filter_get_video_data(filterhandle, &frame, &length) >= 0 )
		{
			if( length > 0 )
			{
				fs.write(frame,length);				
			}			
		}

		usleep(100 * 1000);
	}

	return 0;
}