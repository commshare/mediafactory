/* Example code: an echo server. */
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <pthread.h>

#include "rtsp.h"
#include "rtph264.h"

extern int clientport;
extern char serverip[];
void* taskproc(void *ptr)
{
    printf(" clientport = %d \n", clientport);

    udp_proc(serverip, clientport, (char*)"./111.264");
}

static void echo_read_cb(struct bufferevent *bev, void *ctx)
{
    /* 获取bufferevent中的读和写的指针 */
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);
    /* 把读入的数据全部复制到写内存中 */
    /* Copy all the data from the input buffer to the output buffer. */
    int len = evbuffer_get_length(input);
    char data[2000] = {0};
    bufferevent_read(bev, data, len);

    printf("%s \n", data);
    int canSend = 0;
    std::string response = rtsp_parse(data, bev, canSend);

    printf("%s \n", response.c_str());
    bufferevent_write(bev, response.c_str(), response.size());

    if( canSend )
    {
        pthread_t thread_id;
        pthread_create(&thread_id, 0, taskproc, NULL);
    }
}

static void echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

static void accept_conn_cb(struct evconnlistener *listener,evutil_socket_t fd, struct sockaddr *address, int socklen,void *ctx)
{
    printf("one client connected \n");

    /* 初始化一个bufferevent用于数据的写入和读取，首先需要从Listerner中获取event_base */
    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    /* 设置buferevent的回调函数，这里设置了读和事件的回调函数 */
    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
    /* 启用该bufevent写和读 */
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. ""Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv)
{
#ifdef WINNT
    WSADATA wsaData;
    int iResult = -1;
    if( 0 != (iResult = WSAStartup(MAKEWORD(2,2), &wsaData)) )
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }
#endif

    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    int port = 554;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (port<=0 || port>65535) {
        puts("Invalid port");
        return -1;
    }

    base = event_base_new(); /* 初始化event_base */
    if (!base) {
        puts("Couldn't open event base");
        return -1;
    }

    /* 初始化绑定地址 */
    /* Clear the sockaddr before using it, in case there are extra
    * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(port);

    /* 初始化evconnlistener(绑定地址、设置回调函数以及连接属性) */
    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,(struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        perror("Couldn't create listener");
        return -1;
    }

    /* 设置Listen错误回调函数 */
    evconnlistener_set_error_cb(listener, accept_error_cb);

    /* 开始accept进入循环 */
    event_base_dispatch(base);
    return 0;
}            