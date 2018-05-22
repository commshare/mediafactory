#include "video_generator.h"
#include <string>

struct video_generator_tag_t
{
	std::string video_frame;
	std::string frameY;
	std::string frameU;
	std::string frameV;
    int linesizeY;
    int linesizeU;
    int linesizeV;

    int frame_index;
    int width;
    int height;
    int pixel_format;
};

void *video_generator_alloc(int width, int height, int pixel_format)
{
	video_generator_tag_t *inst = new video_generator_tag_t;

	inst->frame_index = 0;
	inst->width = height;
	inst->height = width;
	inst->pixel_format = pixel_format;

	inst->linesizeY = width * height;
	inst->linesizeU = width * height / 4;
	inst->linesizeV = width * height / 4;
	inst->frameY.assign(inst->linesizeY);
	inst->frameU.assign(inst->linesizeU);
	inst->frameV.assign(inst->linesizeV);
	return inst;
}

int video_generator_get_yuv420p_frame(void* handle, const char** frame, int *Length)
{
	video_generator_tag_t *inst = (video_generator_tag_t*)handle;

	char *dataY = inst->frameY.data();
    int x, y, i = inst->frame_index;
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            dataY[y * inst->linesizeY + x] = x + y + i * 3;

    /* Cb and Cr */
	char *dataU = inst->frameU.data();
	char *dataV = inst->frameV.data();
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            dataU[y * inst->linesizeU + x] = 128 + y + i * 2;
            dataV[y * inst->linesizeV + x] = 64 + x + i * 5;
        }
    }

    inst->frame_index++;
	inst->video_frame.clear();
	inst->video_frame.append(inst->frameY.data(), inst->linesizeY);
	inst->video_frame.append(inst->frameU.data(), inst->linesizeU);
	inst->video_frame.append(inst->frameV.data(), inst->linesizeV);
    *frame = inst->video_frame.data();
    *length = inst->video_frame.size();
    
	return 0;
}

int video_generator_destroy(void *handle)
{
	video_generator_tag_t *inst = (video_generator_tag_t*)handle;
	
	delete inst;
	return 0;	
}
