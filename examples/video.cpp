#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "headers/MLX90640_API.h"
#include "lib/fb.h"
#include "bcm2835.h"
#include <math.h>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#define MLX_I2C_ADDR 0x33

#define IMAGE_SCALE 5

// Valid frame rates are 1, 2, 4, 8, 16, 32 and 64
// The i2c baudrate is set to 1mhz to support these
#define FPS 16
#define FRAME_TIME_MICROS (1000000/FPS)

#define DURATION 5

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the FRAME_TIME_MICROS
// to account for this.
#define OFFSET_MICROS 850


// Struct to hold the output format details, audio codec, video codec, extension
// http://ffmpeg.org/doxygen/trunk/structAVOutputFormat.html
AVOutputFormat *avOutputFormat; // Was *fmt

// http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
AVFormatContext *avFormatContext; // was *oc

// Individual AVStreams for audio and video
// http://ffmpeg.org/doxygen/trunk/structAVStream.html
AVStream *video_avstream; // was *video_st and *st

// Audio and Video Timestamps (presentation time stamp)
double video_pts;
int codec_id = AV_CODEC_ID_H264;
AVCodec *codec;
AVCodecContext *c= NULL;
int i, ret, x, y, got_output;
int write_ret;
//FILE *f;
AVFrame *frame;
AVPacket pkt;
uint8_t endcode[] = { 0, 0, 1, 0xb7 };

static void video_encode_start(const char *outputfile, int fps, AVCodecID codec_id)
{
	// Intialize libavcodec - register all codecs and formats??????????
    av_register_all();

    // auto detect the output format from the name. default is mpeg.
    avOutputFormat = av_guess_format(NULL, outputfile, NULL);
    if (!avOutputFormat) {
        printf("Could not deduce output format from file extension: using apng.\n");
        avOutputFormat = av_guess_format("apng", NULL, NULL);
    }
    if (!avOutputFormat) {
        fprintf(stderr, "Could not find suitable output format\n");
        exit(1);
    }

    avOutputFormat->video_codec = codec_id;


	printf("Setting output format...\n");
    /// allocate the output media context, avFormatContext
    avFormatContext = avformat_alloc_context();
    if (!avFormatContext) {
        fprintf(stderr, "Memory error\n");
        exit(1);
    }
    // Set the output format of the newly allocated avFormatContext to our avOutputFormat
    avFormatContext->oformat = avOutputFormat;


	printf("Setting filename...\n");
    // Set the output filename to the outputfile
    snprintf(avFormatContext->filename, sizeof(avFormatContext->filename), "%s", outputfile);

    /* find the mpeg1 video encoder */
    //codec = avcodec_find_encoder(codec_id);
    codec = avcodec_find_encoder(avOutputFormat->video_codec);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

	
	printf("Add video streamt...\n");

    // add the video stream using the default format codec and initialize the codecs
    video_avstream = avformat_new_stream(avFormatContext, codec);
    if (!video_avstream) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    if (video_avstream->codec == NULL) {
		fprintf(stderr, "AVStream codec is NULL\n");
		exit(1);
    }

    //c = avcodec_alloc_context3(codec);
    c = video_avstream->codec;
    //c->codec_id = avOutputFormat->video_codec;
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 24;
    c->height = 32;
    /* frames per second */
	//video_avstream->time_base = (AVRational){1,FPS};
    c->time_base= (AVRational){1,FPS};

    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = AV_PIX_FMT_RGB24;

    if(codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

	
	printf("Opening codec...\n");
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
 //video_avstream->codec = c;
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */

	
	printf("Allocating image...\n");
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }

    /* open the output file, if needed */
    if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
        if (avio_open(&avFormatContext->pb, outputfile, AVIO_FLAG_WRITE) <0) {
            fprintf(stderr, "Could not open '%s'\n", outputfile);
        	exit(1);
        }
    }

    // some formats want stream headers to be separate
    if(avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	
	printf("Writing header...\n");

	AVDictionary * params = NULL;
	av_dict_set(&params, "plays", "0", 0);
	//av_dict_set(&params, "final_delay", 0, 0);

	avformat_write_header(avFormatContext, &params);
}

void video_encode_frame(int i)
{
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    fflush(stdout);

    frame->pts = i;
    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }
	if (got_output) {
		/* write the compressed frame in the media file */
        printf("Write frame %3d (size=%5d)\n", i, pkt.size);
		write_ret = av_write_frame(avFormatContext, &pkt);
		if (write_ret < 0) {
			fprintf(stderr, "Error writing frame\n");
			exit(1);
		}

		av_free_packet(&pkt);
	} else {
		printf("got_output false: %d", i);
	}
}

void video_encode_end()
{
  /* write the trailer, if any.  the trailer must be written
     * before you close the CodecContexts open when you wrote the
     * header; otherwise write_trailer may try to use memory that
     * was freed on av_codec_close() */
    av_write_trailer(avFormatContext);

    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);

    /* free the streams */
    for(i = 0; i < avFormatContext->nb_streams; i++) {
        av_freep(&avFormatContext->streams[i]->codec);
        av_freep(&avFormatContext->streams[i]);
    }

    if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
        /* close the output file */
        avio_close(avFormatContext->pb);
    }

    /* free the stream */
    av_free(avFormatContext);

    //printf("\n");
    //return 0;
}

void put_pixel_false_colour(int x, int y, double v) {
	// Heatmap code borrowed from: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
	const int NUM_COLORS = 7;
	static float color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1}, {1,1,1} };
	int idx1, idx2;
	float fractBetween = 0;
	float vmin = 5.0;
	float vmax = 50.0;
	float vrange = vmax-vmin;
	v -= vmin;
	v /= vrange;
	if(v <= 0) {idx1=idx2=0;}
	else if(v >= 1) {idx1=idx2=NUM_COLORS-1;}
	else
	{
		v *= (NUM_COLORS-1);
		idx1 = floor(v);
		idx2 = idx1+1;
		fractBetween = v - float(idx1);
	}

	int ir, ig, ib;

	ir = (int)((((color[idx2][0] - color[idx1][0]) * fractBetween) + color[idx1][0]) * 255.0);
	ig = (int)((((color[idx2][1] - color[idx1][1]) * fractBetween) + color[idx1][1]) * 255.0);
	ib = (int)((((color[idx2][2] - color[idx1][2]) * fractBetween) + color[idx1][2]) * 255.0);

	//printf("Putting pixel to frame\n");

	frame->data[0][y * frame->linesize[0] + (x*3) + 0] = ir;
	frame->data[0][y * frame->linesize[0] + (x*3) + 1] = ig;
	frame->data[0][y * frame->linesize[0] + (x*3) + 2] = ib;
	//frame->data[1][y * frame->linesize[1] + x] = ig;
	//frame->data[2][y * frame->linesize[2] + x] = ib;
	fb_put_pixel(x, y, ir, ig, ib);
}

void pulse(){
	bcm2835_gpio_write(RPI_BPLUS_GPIO_J8_07, 1);
	std::this_thread::sleep_for(std::chrono::nanoseconds(50));
	bcm2835_gpio_write(RPI_BPLUS_GPIO_J8_07, 0);
}

int main(){
	static uint16_t eeMLX90640[832];
	float emissivity = 1;
	uint16_t frame[834];
	static float image[768];
	static float mlx90640To[768];
	float eTa;
	static uint16_t data[768*sizeof(float)];

	auto frame_time = std::chrono::microseconds(FRAME_TIME_MICROS + OFFSET_MICROS);

	fb_init();
	bcm2835_init();
	bcm2835_gpio_fsel(RPI_BPLUS_GPIO_J8_07, BCM2835_GPIO_FSEL_OUTP);

	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
	switch(FPS){
		case 1:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
			break;
		case 2:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
			break;
		case 4:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
			break;
		case 8:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
			break;
		case 16:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
			break;
		case 32:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
			break;
		case 64:
			MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
			break;
		default:
			printf("Unsupported framerate: %d", FPS);
			return 1;
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);

	paramsMLX90640 mlx90640;
	MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
	MLX90640_ExtractParameters(eeMLX90640, &mlx90640);


	video_encode_start("video.apng", FPS, AV_CODEC_ID_APNG);

	printf("Encode started..\n");

	//int frame_index = 0;

	int skip_duration = 2;

	for(int frame_index = 0; frame_index < (FPS*skip_duration); frame_index++){
		auto start = std::chrono::system_clock::now();
		pulse();
		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
		eTa = MLX90640_GetTa(frame, &mlx90640);
		MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

		for(int x = 0; x < 24; x++){
			for(int y = 0; y < 32; y++){
				float val = mlx90640To[32 * (23-x) + y];
				put_pixel_false_colour(x, y, val);
			}
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
	}


	//while (1){
	for(int frame_index = 0; frame_index < (FPS*DURATION); frame_index++){
		auto start = std::chrono::system_clock::now();
		pulse();
		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
		eTa = MLX90640_GetTa(frame, &mlx90640);
		MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

		for(int x = 0; x < 24; x++){
			for(int y = 0; y < 32; y++){
				float val = mlx90640To[32 * (23-x) + y];
				put_pixel_false_colour(x, y, val);
			}
		}
		
		video_encode_frame(frame_index);

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
	}

	video_encode_end();
	fb_cleanup();
	bcm2835_close();
	return 0;
}
