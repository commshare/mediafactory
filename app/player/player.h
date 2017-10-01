#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>

void* player_open();

int player_play(void* handle, const char* url);

int player_close(void* handle);

#endif