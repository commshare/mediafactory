#include <string>

int url_getmsg(std::string &buffer, std::string &msg);

void* rtsp_demux_init(const char* sessionid, int local_rtp_port);

std::string rtsp_demux_parse(void* handle, const char* request, int &transport_proto, int &canSend);

int rtsp_demux_get_client_rtp_port(void* handle);

int rtsp_demux_close(void* handle);