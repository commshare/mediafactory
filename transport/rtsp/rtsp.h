#include <string>

std::string rtsp_parse(char* request, struct bufferevent *bev, int &canSend);

std::string getResponse_OPTIONS(const char *server, std::string &seq);

std::string getResponse_DESCRIBE(const char *serverport, const char* clientIP, std::string &seq);

std::string getResponse_SETUP_UDP(const char *sessionID, int clientPort, int serverPort, std::string &seq);
std::string getResponse_SETUP_TCP(const char *sessionID, std::string &seq);

std::string getResponse_PLAY(const char *sessionID, std::string &seq);

std::string getResponse_PAUSE(const char *sessionID, std::string &seq);

std::string getResponse_TEARDOWN(const char *sessionID, std::string &seq);
