#include "WrapperLibAV.h"

struct ffmpegdesc_t
{
	AVFormatContext *pFormatCtx;
};

int AV_Initialize()
{
	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	avcodec_register_all();
#if CONFIG_AVDEVICE
	avdevice_register_all();
#endif
#if CONFIG_AVFILTER
	avfilter_register_all();
#endif
	av_register_all();
	avformat_network_init();

	return 0;
}

AVFormatContext *AV_OpenVideoSource(const char *pstrFileNmae)
{
	//--------------------------open video source,get format infomation
	AVFormatContext *pFormatCtx = NULL;//avformat_alloc_context();
	int ret = avformat_open_input(&pFormatCtx, pstrFileNmae, NULL,NULL);
	if( ret != 0 ) 
		return NULL;

	AVDictionary *options=NULL;
	if(avformat_find_stream_info(pFormatCtx,&options)<0) 
		return NULL;

	av_dump_format(pFormatCtx, 0, pstrFileNmae, 0);

	return pFormatCtx;
}

AVCodecContext *AV_GetCodecContext(AVFormatContext *pFormatCtx,AVMediaType MediaType,int &iMediaCodecID)
{
	if( !pFormatCtx )
		return NULL;
	
	AVCodecContext *pCodecCtx=NULL;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++) 
	{
		AVStream *pStream=pFormatCtx->streams[i];
		if( pStream )
		{
			if( pStream->codec->codec_type==MediaType )
			{
				pCodecCtx=pFormatCtx->streams[i]->codec; 
				if( !pCodecCtx )
					return NULL;

				AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id); 
				if( !pCodec ) 
					return NULL;
				if(avcodec_open2(pCodecCtx, pCodec,NULL)<0) 
					return NULL;

				iMediaCodecID=i;
			}
		}
	}

	return pCodecCtx;
}

int AV_DecodeVideo(AVCodecContext *pCodecCtx, AVFrame *pFrameData, AVPacket *pPacket)
{
	if( !pCodecCtx || !pFrameData ||  !pPacket )
		return -1;

	int VideoDecodeFinished=0;
	avcodec_decode_video2(pCodecCtx, pFrameData, &VideoDecodeFinished, pPacket);
	if( VideoDecodeFinished )
	{
//		av_free_packet(pPacket);
		return 1;
	}
	return 0;
}

int AV_DecodeAudio(AVCodecContext *pCodecCtx,AVFrame *pFrameData,AVPacket *pPacket)
{
// 	int i=0,j=1;
// 	PRINT("i=%d,j=%d",i,j);

	if( !pCodecCtx || !pFrameData ||  !pPacket )
		return -1;

	int AudioDecodeFinished=0;
	avcodec_decode_audio4(pCodecCtx, pFrameData, &AudioDecodeFinished, pPacket);
	if( AudioDecodeFinished )
	{
		av_free_packet(pPacket);
		return 1;
	}
	return 0;
}

int AV_GetNextStreamFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iMediaStreamTypeID, AVMediaType MediaType, void *pFrameData, int &iDecodedLen)
{
	if( !pFormatCtx || !pCodecCtx )
		return 0;

	int iReslult=0;
	AVPacket packet; 
	int frameFinished=0;
	if(av_read_frame(pFormatCtx, &packet)>=0) 
	{
		if(packet.stream_index==iMediaStreamTypeID) 
		{
			if( MediaType==AVMEDIA_TYPE_VIDEO )
				iDecodedLen=avcodec_decode_video2(pCodecCtx, *(AVFrame**)pFrameData, &frameFinished, &packet);
			else if( MediaType==AVMEDIA_TYPE_AUDIO )
				iDecodedLen=avcodec_decode_audio4(pCodecCtx, *(AVFrame**)pFrameData, &frameFinished, &packet);

			if(frameFinished) 
			{
				iReslult=1;
			} 
		}
		av_free_packet(&packet); 
	}
	return iReslult;
}

int AV_GetNextPacket( AVFormatContext *pFormatCtx, AVPacket &avPacket )
{
	if( !pFormatCtx )
		return 0;

	int iResult=av_read_frame(pFormatCtx, &avPacket);

	return iResult;
}

int AV_GetClock(AVStream *pStream,int64_t iPacketPts,double *pfClock)
{
	if( !pStream || iPacketPts<0 || !pfClock )
		return 0;

	*pfClock = av_q2d(pStream->time_base)*iPacketPts;
	return 1;
}

bool AV_NeedTranscode(int format)
{
	return ( format != AV_SAMPLE_FMT_S16 &&
		format != AV_SAMPLE_FMT_S16P );
}

//////////////////////////////output////////////////////////////////////////////////////////
/* Add a video output stream. */
/* 5 seconds stream duration */
#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

static AVFormatContext *rtp_add_stream(AVStream *pStream)
{
	/* now we can open the relevant output stream */
	AVFormatContext *ctx = avformat_alloc_context();
	if (!ctx)
		return NULL;

	ctx->oformat = av_guess_format("rtp", NULL, NULL);
	AVStream *st = (AVStream *)av_mallocz(sizeof(AVStream));
	if (!st)
	{
		av_free(ctx);
		return NULL;
	}

	ctx->nb_streams = 1;
	ctx->streams = (AVStream **)av_mallocz(sizeof(AVStream *)* ctx->nb_streams);
	if (!ctx->streams)
	{
		av_free(ctx);
		av_free(st);
		return NULL;
	}

	ctx->streams[0] = st;
	memcpy(st, pStream, sizeof(AVStream));
	st->priv_data = NULL;

	return ctx;
}

static AVStream *add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id)
{
	AVCodecContext *c;
	AVStream *st;
	AVCodec *codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	st = avformat_new_stream(oc, codec);
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}

	c = st->codec;

	/* Put sample parameters. */
	c->bit_rate = 400000;
	/* Resolution must be a multiple of two. */
	c->width = 352;
	c->height = 288;
	/* timebase: This is the fundamental unit of time (in seconds) in terms
	* of which frame timestamps are represented. For fixed-fps content,
	* timebase should be 1/framerate and timestamp increments should be
	* identical to 1. */
	c->time_base.den = STREAM_FRAME_RATE;
	c->time_base.num = 1;
	c->gop_size = 12; /* emit one intra frame every twelve frames at most */
	c->pix_fmt = STREAM_PIX_FMT;
	if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		/* just for testing, we also add B frames */
		c->max_b_frames = 2;
	}
	if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		* This does not happen with normal video, it just happens here as
		* the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}
	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

//	avcodec_open2(st->codec, codec, NULL);
	return st;
}

static AVStream *add_audio_stream(AVFormatContext *oc, enum AVCodecID codec_id)
{
	AVCodecContext *c;
	AVStream *st;
	AVCodec *codec;

	/* find the audio encoder */
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	st = avformat_new_stream(oc, codec);
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}

	c = st->codec;

	/* put sample parameters */
	c->sample_fmt = AV_SAMPLE_FMT_S16;
	c->bit_rate = 64000;
	c->sample_rate = 44100;
	c->channels = 2;

	// some formats want stream headers to be separate
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

//	avcodec_open2(st->codec, codec, NULL);
	return st;
}
static void set_formatcontext(AVFormatContext *fmt)
{
	if( !fmt )
		return ;
	fmt->packet_size = 256;
}
static void set_codec(AVCodecContext *av)
{
	/* compute default parameters */
	switch (av->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		if (av->bit_rate == 0)
			av->bit_rate = 64000;
		if (av->sample_rate == 0)
			av->sample_rate = 22050;
		if (av->channels == 0)
			av->channels = 1;
		break;
	case AVMEDIA_TYPE_VIDEO:
		if (av->bit_rate == 0)
			av->bit_rate = 64000;
		if (av->time_base.num == 0){
			av->time_base.den = 5;
			av->time_base.num = 1;
		}
		if (av->width == 0 || av->height == 0) {
			av->width = 160;
			av->height = 128;
		}
		/* Bitrate tolerance is less for streaming */
		if (av->bit_rate_tolerance == 0)
			av->bit_rate_tolerance = FFMAX(av->bit_rate / 4,
			(int64_t)av->bit_rate*av->time_base.num / av->time_base.den);
		if (av->qmin == 0)
			av->qmin = 3;
		if (av->qmax == 0)
			av->qmax = 31;
		if (av->max_qdiff == 0)
			av->max_qdiff = 3;
		av->qcompress = 0.5;
		av->qblur = 0.5;

		if (!av->nsse_weight)
			av->nsse_weight = 8;

		av->frame_skip_cmp = FF_CMP_DCTMAX;
		if (!av->me_method)
			av->me_method = ME_EPZS;
		av->rc_buffer_aggressivity = 1.0;

		if (!av->rc_eq)
			av->rc_eq = "tex^qComp";
		if (!av->i_quant_factor)
			av->i_quant_factor = -0.8;
		if (!av->b_quant_factor)
			av->b_quant_factor = 1.25;
		if (!av->b_quant_offset)
			av->b_quant_offset = 1.25;
		if (!av->rc_max_rate)
			av->rc_max_rate = av->bit_rate * 2;

		if (av->rc_max_rate && !av->rc_buffer_size) {
			av->rc_buffer_size = av->rc_max_rate;
		}

		break;
	default:
		break;
	}
}

int AV_InitOutputFmt(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, AVFormatContext **fmtctx)
{
	av_register_all();
	avformat_network_init();

	AVOutputFormat *pOutputFmt = av_guess_format(protocol, NULL, NULL);
	if (!pOutputFmt)
		return -1;

	AVFormatContext *pFmtCtx = avformat_alloc_context();
	if (!pFmtCtx)
		return -1;
	pFmtCtx->oformat = pOutputFmt;
	set_formatcontext(pFmtCtx);

	AVStream *st = (AVStream *)av_mallocz(sizeof(AVStream));
	if (!st)
	{
		av_free(pFmtCtx);
		av_free(pOutputFmt);
		return -1;
	}

	pFmtCtx->nb_streams = 1;
	pFmtCtx->streams = (AVStream **)av_mallocz(sizeof(AVStream *)* pFmtCtx->nb_streams);
	if (!pFmtCtx->streams)
	{
		av_free(pFmtCtx);
		av_free(pOutputFmt);
		av_free(st);
		return -1;
	}

	pFmtCtx->streams[0] = st;
	memcpy(st, pStream, sizeof(AVStream));
	st->priv_data = NULL;
	set_codec(st->codec);

	sprintf(pFmtCtx->filename, "%s://%s:%d", protocol, ip, port);
	int ret;

	if (!(pOutputFmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&pFmtCtx->pb, pFmtCtx->filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			av_free(pFmtCtx);
			av_free(pOutputFmt);
			av_free(st);

			char error[1024];
			av_strerror(ret, error, sizeof(error));
			return -1;
		}
	}

	/* Write the stream header, if any. */
	ret = avformat_write_header(pFmtCtx, NULL);
	if (ret < 0)
	{
		av_free(pFmtCtx);
		av_free(pOutputFmt);
		av_free(st);

		char error[1024];
		av_strerror(ret, error, sizeof(error));
		return -1;
	}
	*fmtctx = pFmtCtx;
	return 0;
}

int AV_InitOutputFmt2(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, unsigned int localport, AVFormatContext **fmtctx)
{
	av_register_all();
	avformat_network_init();

	AVOutputFormat *pOutputFmt = av_guess_format(protocol, NULL, NULL);
	if (!pOutputFmt)
		return -1;

	AVFormatContext *pFmtCtx = avformat_alloc_context();
	if (!pFmtCtx)
		return -1;
	pFmtCtx->oformat = pOutputFmt;
	set_formatcontext(pFmtCtx);

	AVStream *out_stream = avformat_new_stream(pFmtCtx, (AVCodec*)pStream->codec->codec);
	if (!out_stream)
		return -1;

	int ret = avcodec_copy_context(out_stream->codec, pStream->codec);
	if (ret < 0)
		return -1;
	set_codec(out_stream->codec);

	out_stream->codec->codec_tag = 0;
	if (pFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

	if ( !strcmp(protocol, "rtp") )
		sprintf(pFmtCtx->filename, "%s://%s:%d?&localport=%d", protocol, ip, port, localport);
	else
		sprintf(pFmtCtx->filename, "%s://%s:%d", protocol, ip, port);

	if (!(pOutputFmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&pFmtCtx->pb, pFmtCtx->filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			char error[1024];
			av_strerror(ret, error, sizeof(error));
			return -1;
		}
	}

	/* Write the stream header, if any. */
	ret = avformat_write_header(pFmtCtx, NULL);
	if (ret < 0)
	{
		char error[1024];
		av_strerror(ret, error, sizeof(error));
		return -1;
	}
	*fmtctx = pFmtCtx;
	return 0;
}

int AV_InitOutputRtp(AVFormatContext *pFmtCtx, const char *protocol, const char *ip, unsigned int port)
{
	av_register_all();
	avformat_network_init();

	AVOutputFormat *pOutputFmt = av_guess_format(protocol, NULL, NULL);
	if (!pOutputFmt)
		return -1;

	pFmtCtx->oformat = pOutputFmt;
	char strFileName[512];
	sprintf(strFileName, "%s://%s:%d", protocol, ip, port);
	int ret = avio_open(&pFmtCtx->pb, strFileName, AVIO_FLAG_WRITE);
	if (ret < 0)
	{
		char error[1024];
		av_strerror(ret, error, sizeof(error));
		return -1;
	}
	/* Write the stream header, if any. */
	pFmtCtx->streams[0]->codec->codec_id = AV_CODEC_ID_MPEG2TS;
	ret = avformat_write_header(pFmtCtx, NULL);
	if (ret < 0)
	{
		char error[1024];
		av_strerror(ret, error, sizeof(error));
		return -1;
	}
	return 0;
}

int AV_SendOutputPacket(AVFormatContext *fmtctx, AVStream *pStreamIn, AVStream *pStreamOut, AVPacket *pkt)
{
	if (pkt->size <= 0)
		return -1;

//	if ( !(pkt->flags & AV_PKT_FLAG_KEY) )
//		return -1;

//	if (pkt->pts != AV_NOPTS_VALUE)
	pkt->pts = av_rescale_q(pkt->pts, pStreamIn->codec->time_base, pStreamOut->time_base);
//	if (pkt->dts != AV_NOPTS_VALUE)
	pkt->dts = av_rescale_q(pkt->dts, pStreamIn->codec->time_base, pStreamOut->time_base);
	pkt->duration = av_rescale_q(pkt->duration, pStreamIn->time_base, pStreamOut->time_base);
	pkt->stream_index = 0;

//	avio_open_dyn_buf(&fmtctx->pb);
	fmtctx->pb->seekable = 0;
//	uint8_t *dummy_buf;

	/* Write the compressed frame to the media file. */
//	printf("av_interleaved_write begin\n");
	int ret = av_write_frame(fmtctx, pkt);
//	int ret = av_interleaved_write_frame(fmtctx, pkt);
//	printf("av_interleaved_write end,return %d\n",ret);

//	int len = avio_close_dyn_buf(fmtctx->pb, &dummy_buf);
//	avio_write(fmtctx->pb, dummy_buf, len);
//	avio_flush(fmtctx->pb);
//	av_free(dummy_buf);

	return ret;
}
