#ifndef SHARE_H
#define SHARE_H
extern int g_raw_sfd; // global raw send socket file handle
extern int g_raw_rfd; // global raw recv socket file handle
extern int g_time_fd;
#define MAX_EPOLL_EVENTS 4096
extern struct epoll_event g_epoll_evlist[MAX_EPOLL_EVENTS];
extern int g_facktcp_state;
enum facktcp_state{
    ftcp_nothing,
    ftcp_syn_send,
    ftcp_ack_send,
    ftcp_connect_ready,
};
extern int g_times_try_connect;
#define MAX_BUFF 4096
extern char g_sbuf_raw[MAX_BUFF];
#endif