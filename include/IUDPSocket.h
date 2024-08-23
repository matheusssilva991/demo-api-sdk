#pragma once

#include <stdint.h>
#define XUDP_RCVBUF_SIZE	8*1024*1024	//socket receive buffer size 8M
#define XUDP_RCV_TIMEOUT	100			//socket receive timeout 10ms
#define XUDP_BUF_SIZE		1024*9		//client buffer size

class IUDPSocket
{
public:
	IUDPSocket() {};
	~IUDPSocket() {};

	virtual bool Open(uint32_t recv_buf_size = XUDP_RCVBUF_SIZE) = 0;
	virtual void Close() = 0;
	virtual void SetPeer(const char* peer_ip_, uint16_t peer_port) = 0;
	virtual bool Bind(const char* local_ip_, uint16_t local_port) = 0;
	virtual void CleanRevBuffer() = 0;
	virtual bool IsPeerSet() = 0;
	virtual bool IsOpen() = 0;
	virtual bool IsBind() = 0;
	virtual int32_t IsDataAvailable(int32_t millisecond) = 0;

	virtual int32_t Send(const unsigned char* send_buf_, int32_t buf_len) = 0;
	virtual int32_t	Recv(unsigned char* recv_buf_, int32_t buf_len) = 0;
	virtual void SetRIORecvBufSize(const uint32_t size) = 0;
};

