#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "share.h"
#include "spdlog/spdlog.h"

void send_sync(){
    //sendto();
}

/*
* Normally, SYN Segment have MSS Option, which 
*-------------------------------------------------------------------------
*|version |protocal| Type of service |            Total length           |
*| 4bites | 4bites |     8bites      |          16bites                  |
*------------------------------------------------------------------------
*|          Identification           | flag |     Fragment offset        |
*|          16bites                  |3bites|         13bites            |
*------------------------------------------------------------------------
*/
int fake_tcp_keep_connect(){

    if(g_facktcp_state == ftcp_nothing){
        g_facktcp_state = ftcp_syn_send;
        spdlog::info("state changed from nothing to syn_sent\n");
		// retry_counter = 5;
        // IP header
        struct iphdr *iph = (struct iphdr *)g_sbuf_raw;
        // TCP header
        struct tcphdr *tcph = (struct tcphdr *) (g_sbuf_raw + sizeof (struct ip));
        
        iph->version = 4; // IPV4 Protocal code is 0x4. IPV6 Protocal code is 0x6.
        iph->ihl = 20/4; // IP header lenght is variable. Its minimal is 20. ihl = 20 / 4;
        iph->tos = 0; //  Type of service. Its is used to define service's priority. 00 is lowest priority.
        iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr); // IP package total lenght.
        iph->id = 0xffff;

		iph->frag_off = 0;
        iph->ttl = 255;
        iph->protocol = IPPROTO_TCP;
        iph->check = 0; //Set to 0 before calculating checksum
        iph->saddr = inet_addr("127.0.0.1");    //Spoof the source ip address
        iph->daddr = inet_addr("127.0.0.1");  

        // TCP Header
        tcph->source = htons(0x7777);
        tcph->dest = htons(0x8888);
        tcph->seq = htonl(0x4444);
        tcph->ack_seq = htonl(0x0);

        tcph->doff = 5;  //tcp header size
        tcph->fin = 0;
        tcph->syn = 1;
        tcph->rst = 0;
        tcph->psh = 0;
        tcph->ack = 0;
        tcph->urg=0;

        tcph->window = htons((uint16_t)(40000+random()%20000));
        tcph->check = 0;  //leave checksum 0 now, filled later by pseudo header
        tcph->urg_ptr = 0;


        psh.source_address = info.src_ip;
        psh.dest_address = sin.sin_addr.s_addr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(struct tcphdr) + payloadlen );

        int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payloadlen;

        memcpy(raw_send_buf2 , (char*) &psh , sizeof (struct pseudo_header));
        memcpy(raw_send_buf2 + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr) + payloadlen);

        tcph->check = csum( (unsigned short*) raw_send_buf2, psize);

        //Ip checksum
        iph->check = csum ((unsigned short *) raw_send_buf, iph->tot_len);


        if(prog_mode==client_mode&& payloadlen!=9  ||prog_mode==server_mode&& payloadlen!=5)
        printf("sent seq  ack_seq len<%u %u %d>\n",g_packet_info.seq,g_packet_info.ack_seq,payloadlen);

        int ret = sendto(raw_send_fd, raw_send_buf, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin));
        if(g_packet_info.syn==0&&g_packet_info.ack==1&&payloadlen!=0)
        {
    	    if(seq_mode==0)
    	    {

    		    g_packet_info.seq+=payloadlen;
    	    }
    	    else if(seq_mode==1)
    	    {
                
    	    }
    	    else if(seq_mode==3)
    	    {
    		    if(random()% 20==5 )
    			g_packet_info.seq+=payloadlen;
            }
     }
     if(debug_mode) printf("<ret:%d>\n",ret);
	 if(ret<0)
     {
    	 perror("raw send error");
     }
     return 0;
		send_sync();//send sync
    }else if (g_facktcp_state == ftcp_syn_send){

    }else if (g_facktcp_state == ftcp_ack_send){

    }else if (g_facktcp_state == ftcp_connect_ready){

    }

    return 0;
}

int recv_raw_package(){
    return 0;
}

int main(int argc, char* argv[]){
    spdlog::info("Riven's Udp2raw Start");
    // Provides a reliable datagram layer that does not guarantee ordering.
    g_raw_sfd = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
    if(g_raw_sfd == -1) {
        spdlog::error("Failed to create raw send socket(g_raw_sfd)");
        exit(1);
    }
    spdlog::info("Raw send socket(g_raw_sfd) is {:d}", g_raw_sfd);

    g_raw_rfd= socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if(g_raw_rfd == -1) {
        //socket creation failed, may be because of non-root privileges
        spdlog::error("Failed to create raw recv socket(g_raw_rfd)", g_raw_rfd);
        exit(1);
    }
    spdlog::info("Raw send socket(g_raw_rfd) is {:d}", g_raw_rfd);

    // 发送的数据包含IP数据包头IP_HDRINCL IP Header Include.
    int on = 1;
    if(setsockopt(g_raw_sfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        spdlog::error("For g_raw_sfd set IP_HDRINCL faild", g_raw_sfd);
    }
    spdlog::info("For g_raw_sfd set IP_HDRINCL succeed", g_raw_sfd);

    int fd_epool = epoll_create1(0);
    if(fd_epool <= 0){
        spdlog::error("Create epoll fd faild:{:d}", fd_epool);
        exit(1);
    }
    spdlog::info("Create epoll fd succeed:{:d}", fd_epool);
    
    itimerspec its;
    memset(&its, 0, sizeof(its));
    g_time_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    its.it_interval.tv_sec=1;
	its.it_value.tv_sec=1;
	timerfd_settime(g_time_fd, 0, &its, 0);

    epoll_event event;
    event.data.fd = g_time_fd;
    event.events = EPOLLIN;
    int iret = epoll_ctl(fd_epool, EPOLL_CTL_ADD, g_time_fd, &event);
    if(iret < 0){
        spdlog::error("epoll_ctl return %d", iret);
        exit(-1);
    }

    event.data.fd = g_raw_rfd;
    event.events = EPOLLIN;
    iret = epoll_ctl(fd_epool, EPOLL_CTL_ADD, g_raw_rfd, &event);
    if(iret < 0){
        spdlog::error("epoll_ctl return {:d}", iret);
        exit(-1);
    }

    while(1){
        int nfds = epoll_wait(fd_epool, g_epoll_evlist, MAX_EPOLL_EVENTS, 180 * 1000);
        if (nfds < 0) {  //allow zero
			spdlog::error("epoll_wait return {:d}", nfds);
			exit(-1);
		}

        for(int i = 0; i < nfds; ++i){
            if (g_epoll_evlist[i].data.fd == g_time_fd){
                spdlog::info("timer hit");
                uint64_t value = 0;
	            read(g_time_fd, &value, 8);
                spdlog::info("g_time_fd read {:d}", value);
                fake_tcp_keep_connect();
            }else if (g_epoll_evlist[i].data.fd == g_raw_rfd){
                spdlog::info("raw package recved awake");
                recv_raw_package();
            }
            else{
                spdlog::error("Unknown event");
			    exit(-1);
            }
        }
    }

    if(fd_epool >= 0){
        close(fd_epool);
    }
    return 0;
}