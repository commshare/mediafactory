#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include "rtspdemux.h"

typedef struct 
{
    std::string client_ip;
    int client_rtp_port;
    std::string client_session_id;

}rtsp_demux_desc_t;

int serverport = 11002;

std::string getResponse_OPTIONS(const char *server, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n" \
                    "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS, ANNOUNCE, RECORD\r\n" \
                    "\r\n";

    return strResponse;
}

std::string getResponse_DESCRIBE(std::string &seq)
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

////////////////////////////////////////////
void* rtsp_demux_init(const char* sessionid)
{
    rtsp_demux_desc_t* handle = new rtsp_demux_desc_t;
    handle->client_session_id = sessionid;

    return (void*)handle;
}

std::string rtsp_demux_parse(void* handle, const char* request, int &canSend)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;

    canSend = 0;
    std::string strRequest = request;
    int pos = strRequest.find(' ');
    std::string head = strRequest.substr(0,pos), strResponse;

    pos = strRequest.find("CSeq: ");
    int pos1 = strRequest.find("\r\n", pos);

    std::string strCSeq = strRequest.substr(pos+6, pos1-pos-6);
//    printf("CSeq=%s \n", strCSeq.c_str());

    if( head == "OPTIONS" )
    {
        strResponse = getResponse_OPTIONS("Robert RTSPServer", strCSeq);
    }
    else if( head == "DESCRIBE" )
    {
        strResponse = getResponse_DESCRIBE(strCSeq);
    }
    else if( head == "SETUP" )
    {
        pos = strRequest.find("RTP/AVP/TCP");
//        printf("setup pos = %d \n", pos);

        if(  pos > 0 )
        {
            strResponse = getResponse_SETUP_TCP(rtsp_demux->client_session_id.c_str(), strCSeq);            
        }
        else
        {
            pos = strRequest.find("client_port=");
            pos1 = strRequest.find(pos);
            std::string strTemp = strRequest.substr(pos+strlen("client_port="),pos1-pos);
            rtsp_demux->client_rtp_port = atoi(strTemp.c_str());
            strResponse = getResponse_SETUP_UDP(rtsp_demux->client_session_id.c_str(), atoi(strTemp.c_str()), serverport, strCSeq);            
        }
    }
    else if( head == "PLAY" )
    {
        strResponse = getResponse_PLAY(rtsp_demux->client_session_id.c_str(), strCSeq);        
        canSend = 1;
    }
    else if( head == "PAUSE" )
    {
        strResponse = getResponse_PAUSE(rtsp_demux->client_session_id.c_str(), strCSeq);        
    }
    else if( head == "TEARDOWN" )
    {
        strResponse = getResponse_TEARDOWN(rtsp_demux->client_session_id.c_str(), strCSeq);        
    }
    else
    {
        printf("unknown head\n");
    }
    
    return strResponse;
}

int rtsp_demux_get_client_rtp_port(void* handle)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;

    return rtsp_demux->client_rtp_port;
}

int rtsp_demux_close(void* handle)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;
    delete rtsp_demux;

    return 0;
}