#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>

int ShowSDL(std::string strFileName,void* hwnd);

int OpenRtpStream(std::string strFileName, void* hwnd, std::string strProtocol, std::string strIP, int port, int localport);

#endif