#include <string>

void* rtsp_demux_init(const char* sessionid, int local_rtp_port);

std::string rtsp_demux_parse(void* handle, const char* request, int &canSend);

int rtsp_demux_get_client_rtp_port(void* handle);

int rtsp_demux_close(void* handle);