#ifdef _WIN32 
    #include <Winsock2.h>
#elif defined __APPLE__
#elif defined __ANDROID__
#elif defined __linux__
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <map>
#include <string>
#include <string.h>
#include <mutex>
#include <thread>
#include <deque>
#include <iostream>

#include "udpselectserver.h"

typedef std::deque<std::string> UDPCLIENTDATAQUEUE;

struct udpclientdesc_t {
    int sockfd;
    std::string ip;
    int port;
    std::mutex recvmutex;
    std::mutex sendmutex;

    UDPCLIENTDATAQUEUE recvqueue;
    UDPCLIENTDATAQUEUE sendqueue;
};

typedef std::map<int, udpclientdesc_t*> UDPCLIENTMAP;

struct udpserverdesc_t {
    int sockfd;
    std::string localip;
    int localport;
    on_connect_callback connectcallback;
    on_close_callback closecallback;

    void* userdata;
    std::thread threadeventloop;

    UDPCLIENTMAP clients;
    fd_set fdread, fdwrite;
};

int setnonblocking(int sock)
{
#ifdef _WIN32
     unsigned long NonBlock = 1;   
     if(ioctlsocket(sock, FIONBIO, &NonBlock) == SOCKET_ERROR)   
     {   
         printf("ioctlsocket() failed with error %d\n", WSAGetLastError());   
         return -1;   
     }   
    return 0;
#else
    int opts = fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
    return 0;
#endif
}

int udp_socket_connect(void* handle,struct sockaddr_in  *client_addr)  
{        
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in my_addr, their_addr;  
    int fd=socket(PF_INET, SOCK_DGRAM, 0);  
      
    /*设置socket属性，端口可以重用*/  
    int opt=SO_REUSEADDR;  
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));  

    setnonblocking(fd);  
    memset(&my_addr, 0, sizeof(my_addr));  
    my_addr.sin_family = PF_INET;  
    my_addr.sin_port = htons(inst->localport);  
    my_addr.sin_addr.s_addr = INADDR_ANY;  
    if (bind(fd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)   
    {  
        perror("bind");  
        return -1;
    }   
    else  
    {  
        printf("IP and port bind success \n");  
    }  

    if(fd==-1)  
        return  -1;  

    if( connect(fd,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in)) < 0 )
    {
        perror("connect");  
        return -1;        
    }  

    return fd;     
}  
   
int accept_client(void* handle,int listenfd)  
{  
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in client_addr;  
    int addr_size = sizeof(client_addr);  
    char buf[2048] = {0};  
    while( 1 )
    {
        int ret = recvfrom(listenfd, buf,sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_size);  
        //check(ret > 0, "recvfrom error");  
        if( ret <= 0 )
        {
            perror("recvfrom error");
            break;;
        }        
        std::string clientip = inet_ntoa(client_addr.sin_addr);
        int clientport = ntohs(client_addr.sin_port);
        std::cout << "recved from " << buf<< ret << std::endl;

        if( !inst->clients.empty() )
        {
            UDPCLIENTMAP::iterator iter = inst->clients.begin();
            for( ;iter != inst->clients.end(); ++iter )
            {
                if( iter->second->ip == clientip &&
                    iter->second->port == clientport )
                {
                    iter->second->recvmutex.lock();
                    iter->second->recvqueue.push_back(std::string(buf, ret));
                    iter->second->recvmutex.unlock();

                    break;
                }
            }        
            if( iter != inst->clients.end() )
                continue;
        }

        std::cout << "accapt a connection from " << clientip<<" "<<clientport << std::endl;
        int new_sock=udp_socket_connect(handle,(struct sockaddr_in*)&client_addr);  
          
        udpclientdesc_t *client = new udpclientdesc_t;
        client->recvqueue.push_back(std::string(buf, ret));
        client->sockfd = new_sock;
        client->ip = clientip;
        client->port = clientport;
        inst->clients[new_sock] = client;

        if( inst->connectcallback )
            inst->connectcallback(inst, new_sock, inst->userdata);
    }

    return 0;  
}  

int select_fds(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    FD_ZERO (&inst->fdread);
    FD_ZERO (&inst->fdwrite);
    
    FD_SET (inst->sockfd, &inst->fdread);
    FD_SET (inst->sockfd, &inst->fdwrite);

    if( !inst->clients.empty() )
    {
        for( UDPCLIENTMAP::iterator iter = inst->clients.begin();
            iter != inst->clients.end();
            ++iter)
        {
            FD_SET (iter->second->sockfd, &inst->fdread);        
            FD_SET (iter->second->sockfd, &inst->fdwrite);        
        }        
    }

    struct timeval tout;
    tout.tv_sec = 0;
    tout.tv_usec = 1000 * 500;
    int tag = select (inst->clients.size() + 1 + 1, &inst->fdread, &inst->fdwrite, NULL, &tout);
    if (tag == 0)
        printf("select wait timeout !");

    return tag;
}

int fdread_is_selected(void* handle, int fd)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    return FD_ISSET (fd, &inst->fdread);
}

int fdwrite_is_selected(void* handle, int fd)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    return FD_ISSET (fd, &inst->fdwrite);
}

int udp_select_server_eventloop(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("listenfd = %d \n", listenfd);

    /*设置socket属性，端口可以重用*/  
    int opt=SO_REUSEADDR;  
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));  

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
#ifdef _WIN32
    serveraddr.sin_addr.s_addr =inet_addr(inst->localip.c_str());
#else
    inet_aton(inst->localip.c_str(),&(serveraddr.sin_addr));//htons(portnumber);
#endif

    serveraddr.sin_port=htons(inst->localport);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, 1024);
    inst->sockfd =listenfd;

    char buffer[1024 * 2] = {0};
    
    while( 1 ) 
    {
        int nfds = select_fds(handle);
//        std::cout<<"epoll_wait "<<nfds<<std::endl;

        if( fdread_is_selected(handle, listenfd) )
        {
            accept_client(handle, listenfd);
        }

        if( !inst->clients.empty() )
        {
            for( UDPCLIENTMAP::iterator iter = inst->clients.begin();
                iter != inst->clients.end();
                ++iter)
            {
                if( fdread_is_selected(handle, iter->second->sockfd) )
                {                
                    while( 1 )
                    {
                        int n = read(iter->second->sockfd, buffer, sizeof(buffer));
                        if (n < 0) 
                        {
                            if (errno != EAGAIN) 
                            {
                                if( inst->closecallback )
                                    inst->closecallback(inst, iter->second->sockfd, inst->userdata);
                            
                                close(iter->second->sockfd);
                            }
                            break;
                        } else if (n == 0) {
                            if( inst->closecallback )
                                inst->closecallback(inst, iter->second->sockfd, inst->userdata);

                            close(iter->second->sockfd);
                            break;
                        }
                        buffer[n] = '\0';
                        std::cout<<buffer<<n<<std::endl;

                        iter->second->recvmutex.lock();
                        iter->second->recvqueue.push_back(std::string(buffer, n));
                        iter->second->recvmutex.unlock();
                    }
                }

                if( fdwrite_is_selected(handle, iter->second->sockfd) )
                {
                    iter->second->sendmutex.lock();
                    if( iter->second->sendqueue.size() > 0 )
                    {
                        std::string data = iter->second->sendqueue.front();
                        write(iter->second->sockfd, data.data(), data.size());                    
                        iter->second->sendqueue.pop_front();
                    }
                    iter->second->sendmutex.unlock();
                }

            }        
        }
    }

    return 0;
}

////////////////////////////////////////////////////////
void* udp_select_server_new(const char* localip, int localport, on_connect_callback connectcallback, on_close_callback closecallback, void* userdata)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif  /*  WIN32  */

    udpserverdesc_t * inst = new udpserverdesc_t;
    if( !inst ) return NULL;

    inst->localip = localip;
    inst->localport = localport;
    inst->connectcallback = connectcallback;
    inst->closecallback = closecallback;
    inst->userdata = userdata;

    inst->threadeventloop = std::thread(udp_select_server_eventloop, (void*)inst);

    return inst;
}

int udp_select_server_read(void* handle, int clientid, char* data, int length)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    int reallength = 0;
    iter->second->recvmutex.lock();
    if( iter->second->recvqueue.size() <= 0 )
    {
        reallength = 0;
    }
    else
    {
        std::string frontdata = iter->second->recvqueue.front();
        reallength = frontdata.size();
        memcpy(data, frontdata.data(), frontdata.size());
        iter->second->recvqueue.pop_front();
    }
    iter->second->recvmutex.unlock();

    return reallength;
}

int udp_select_server_write(void* handle, int clientid, const char* data, int length)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    iter->second->sendmutex.lock();
    iter->second->sendqueue.push_back(std::string(data, length));
    iter->second->sendmutex.unlock();

    return 0;
}

int udp_select_server_close(void* handle, int clientid)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    close(clientid);
    udpclientdesc_t* client = iter->second;
    delete client;
    inst->clients.erase(iter);

    return 0;
}

int udp_select_server_free(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    if( !inst->clients.empty() )
    {
        for( UDPCLIENTMAP::iterator iter = inst->clients.begin();
            iter != inst->clients.end();
            ++iter)
            {
                close(iter->first);
                udpclientdesc_t* client = iter->second;
                delete client;
            }                
    }

    close(inst->sockfd);
    delete inst;

    return 0;
}