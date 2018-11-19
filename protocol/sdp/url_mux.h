#ifndef __SDP_MUX_H__
#define __SDP_MUX_H__
             
#include <stdint.h>

int sdp_mux_alloc(void** inst);

int url_mux_set_header(void* inst, char* headername, char* headervalue);

int url_mux_get_buffer(void* inst, char** buffer, int *length);
int url_mux_flush_buffer(void* inst);

int sdp_mux_free(void* inst);

#endif