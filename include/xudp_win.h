/*
Copyright (c) 2014, Detection Technology Inc.
All rights reserved.

This is the UDP socket wrapper class for Windows.

Author: Zhang Xu, 2014-2-24
*/
#ifndef XUDP_WIN_H
#define XUDP_WIN_H
#include <stdint.h>
#include "Winsock2.h"
#include "IUDPSocket.h"
#pragma comment(lib, "ws2_32.lib")

/*
UDP socket wrapper class. If use it as client,Connect() Send() and
Recv()should be used. If use it as server, Bind(), Recvfrom() and Sendto()
should be used.
*/
class XUDPSocket : public IUDPSocket
{
public:
	XUDPSocket();
	~XUDPSocket();
	
	bool Open(uint32_t recv_buf_size = XUDP_RCVBUF_SIZE);
	void Close();
	void SetPeer(const char* peer_ip_, uint16_t peer_port);
	bool Bind(const char* local_ip_, uint16_t local_port);
	void CleanRevBuffer();
	bool IsPeerSet();
	bool IsOpen();
	bool IsBind();
	int32_t IsDataAvailable(int32_t millisecond);

	int32_t Send(const unsigned char* send_buf_, int32_t buf_len);
	int32_t	Recv(unsigned char* recv_buf_, int32_t buf_len); 

private:
	XUDPSocket(const XUDPSocket&);
	XUDPSocket& operator = (const XUDPSocket&);
	void SetRIORecvBufSize(const uint32_t size) { 0; }
	SOCKET _socket;
	sockaddr_in _peer_serv;
	sockaddr_in _local_serv;
	bool _is_peer_set;
	bool _is_open;
	bool _is_bind;
};
#endif