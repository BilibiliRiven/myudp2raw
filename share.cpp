#include "share.h"
#include <sys/epoll.h>

int g_raw_sfd = -1; // global raw send socket file handle
int g_raw_rfd = -1; // global raw recv socket file handle
int g_time_fd = -1;
struct epoll_event g_epoll_evlist[MAX_EPOLL_EVENTS];
int g_facktcp_state = ftcp_nothing;
int g_times_try_connect = 0;
char g_sbuf_raw[MAX_BUFF];
