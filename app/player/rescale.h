#ifndef __RESCALE_H__
#define __RESCALE_H__

struct image_rescaled
{
	uint8_t **data;
	int *linesize;
};

void* rescale_open(int width, int height, int pix_fmt);

int rescale_image(void* handle, uint8_t **data, int linesize[], image_rescaled *image);

int rescale_close(void* handle);

#endif