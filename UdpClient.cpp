#include <sys/types.h>  /* for type definitions */
#include <winsock2.h>   /* for win socket API calls */
#include <ws2tcpip.h>   /* for win socket structs */
#include <stdio.h>      /* for printf() and fprintf() */
#include <signal.h>
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for strlen() */

#include "mex.h"
#include "matrix.h"
#include "UdpClient.h"

#pragma comment(lib,"wsock32.lib")


UdpClient::UdpClient(char *mc_addr_str, unsigned short mc_port, DWORD recvTimeout)
{
    flag_on = 1;
    
    /* Load Winsock 2.0 DLL */
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        mexErrMsgTxt("WSAStartup() failed");
    }
    
    /* create socket to join multicast group on */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        mexErrMsgTxt("socket() failed");
    }
    
    /* set reuse port to on to allow multiple binds per host */
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_on, sizeof(flag_on))) < 0) {
        mexErrMsgTxt("setsockopt() failed");
    }
    
    /* set timeout value */
    if ((setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&recvTimeout, sizeof(recvTimeout))) < 0) {
        mexErrMsgTxt("setsockopt() failed");
    }
    
    /* construct a multicast address structure */
    memset(&mc_addr, 0, sizeof(mc_addr));
    mc_addr.sin_family      = AF_INET;
    mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mc_addr.sin_port        = htons(mc_port);
    
    /* bind to multicast address to socket */
    if ((bind(sock, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) < 0) {
        mexErrMsgTxt("bind() failed");
    }
    
    /* construct an IGMP join request structure */
    mc_req.imr_multiaddr.s_addr = inet_addr(mc_addr_str);
    mc_req.imr_interface.s_addr = htonl(INADDR_ANY);
    
    /* send an ADD MEMBERSHIP message via setsockopt */
    if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mc_req, sizeof(mc_req))) < 0) {
        mexErrMsgTxt("setsockopt() failed");
    }
}

/* Close connection and cleanup */
UdpClient::~UdpClient()
{
    /* send a DROP MEMBERSHIP message via setsockopt */
    if ((setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*) &mc_req, sizeof(mc_req))) < 0) {
        mexErrMsgTxt("setsockopt() failed");
    }
    
    closesocket(sock);
    WSACleanup();  /* Cleanup Winsock */
}

/* Wait for received packet */
void UdpClient::receivePacket(mxArray **out)
{
    /* create a single element return for now */
    *out = mxCreateNumericMatrix(MAX_LEN,1,mxUINT8_CLASS,mxREAL);
    
    /* clear the receive buffers & structs */
    memset(recv_str, 0, sizeof(recv_str));
    from_len = sizeof(from_addr);
    memset(&from_addr, 0, from_len);
    
    /* timeout based waiting to receive a packet */
    recv_len = recvfrom(sock, recv_str, MAX_LEN, 0, (struct sockaddr*)&from_addr, &from_len);
    
    /* output received string */
    if (recv_len>0) {
        mxSetM(*out,recv_len);
        recv_bytes = (char*)mxGetPr(*out);
        memcpy(recv_bytes,recv_str,sizeof(char)*recv_len);
        
        // printf("Received %d bytes from %s: ", recv_len, inet_ntoa(from_addr.sin_addr));
        // for (int ii=0; ii<recv_len; ii++)
        //     printf("%c", recv_str[ii]);
        // printf("\n");
    }
    else
    {
        mxSetM(*out,0);
        // printf("Timeout\n");
    }
    
}
