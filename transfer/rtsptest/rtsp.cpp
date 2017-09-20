#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rtsp.h"

char sessionid[] = "55110";
char serverip[] = "127.0.0.1";
int serverport = 11002;
int clientport = 0;

std::string rtsp_parse(char* request, struct bufferevent *bev, int &canSend)
{
    canSend = 0;
    std::string strRequest = request;
    int pos = strRequest.find(' ');
    std::string head = strRequest.substr(0,pos), strResponse;

    pos = strRequest.find("CSeq: ");
    int pos1 = strRequest.find("\r\n", pos);

    std::string strCSeq = strRequest.substr(pos+6, pos1-pos-6);
    printf("CSeq=%s \n", strCSeq.c_str());

    if( head == "OPTIONS" )
    {
        strResponse = getResponse_OPTIONS("Robert RTSPServer", strCSeq);
    }
    else if( head == "DESCRIBE" )
    {
        strResponse = getResponse_DESCRIBE("554",serverip, strCSeq);
    }
    else if( head == "SETUP" )
    {
        pos = strRequest.find("RTP/AVP/TCP");
//        printf("setup pos = %d \n", pos);

        if(  pos > 0 )
        {
            strResponse = getResponse_SETUP_TCP(sessionid, strCSeq);            
        }
        else
        {
            pos = strRequest.find("client_port=");
            pos1 = strRequest.find(pos);
            std::string strTemp = strRequest.substr(pos+strlen("client_port="),pos1-pos);
            strResponse = getResponse_SETUP_UDP(sessionid, atoi(strTemp.c_str()), serverport, strCSeq);            
        }
    }
    else if( head == "PLAY" )
    {
        strResponse = getResponse_PLAY(sessionid, strCSeq);        
        canSend = 1;
    }
    else if( head == "PAUSE" )
    {
        strResponse = getResponse_PAUSE(sessionid, strCSeq);        
    }
    else if( head == "TEARDOWN" )
    {
        strResponse = getResponse_TEARDOWN(sessionid, strCSeq);        
    }
    else
    {
        printf("unknown head\n");
    }
    
    return strResponse;
}

std::string getResponse_OPTIONS(const char *server, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n" \
                    "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS, ANNOUNCE, RECORD\r\n" \
                    "\r\n";

    return strResponse;
}

std::string getResponse_DESCRIBE(const char *serverport, const char* clientIP, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK\r\n" \
                        "Cseq: " + seq + " \r\n" \
                        "Content-Type: application/sdp \r\n";
    
    std::string strSDP = "v=0\r\n" \
            "m=video 0 RTP/AVP 96\r\n" \
            "a=rtpmap:96 H264/90000\r\n" \
//            "fmtp:96 packetization-mode=1;profile-level-id=64000D;sprop-parameter-sets=Z2QADaw06BQfoQAAAwABAAADADKPFCqg,aO4BLyw=\r\n" \

            "c=IN IP4 0.0.0.0";
    strSDP += "\r\n";

    headers += "Content-length: ";
    std::stringstream ss;
    ss<<strSDP.size();
    headers += ss.str();
    headers += "\r\n\r\n";

    std::string strResponse = headers + strSDP;
    return strResponse;
}

std::string getResponse_SETUP_UDP(const char *sessionID, int clientPort, int serverPort, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";
    
    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";

    std::string strTransport = "Transport: RTP/AVP;unicast;client_port=";
    std::stringstream ss;

    clientport = clientPort;
    
    ss<<clientPort<<"-"<<clientPort+1;
    strTransport += ss.str();
    strTransport += ";server_port=";
    ss.str("");
    ss<<serverPort<<"-"<<serverPort+1;
    strTransport += ss.str() + "\r\n";

    std::string strResponse = headers + strSession;
    strResponse += strTransport;
    strResponse += "\r\n";

    return strResponse;
}

std::string getResponse_SETUP_TCP(const char *sessionID, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK \r\n" \
                     "Cseq: " + seq + " \r\n";
   
    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";

    std::string strTransport = "Transport: RTP/AVP/TCP;unicast;interleaved=0-1;mode=play \r\n";

    std::string strResponse = headers + strSession;
    strResponse += strTransport;
    strResponse += "\r\n";

    return strResponse;
}

std::string getResponse_PLAY(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}   

std::string getResponse_PAUSE(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += "\r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}

std::string getResponse_TEARDOWN(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += "\r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}