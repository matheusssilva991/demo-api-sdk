#pragma once
#ifndef XUDPEX_WIN_H
#define XUDPEX_WIN_H
#include <stdint.h>
#include "Winsock2.h"
#include <MSWSock.h>
#include <queue>
#include "IUDPSocket.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define XUDP_RCVBUF_SIZE	8*1024*1024	//socket receive buffer 
#define XUDP_RCV_TIMEOUT	100			//socket receive timeout 
#define XUDP_BUF_SIZE		1024*9		//client buffer size

enum COMPLETION_KEY
{
	CK_STOP = 0,
	CK_SEND = 1,
	CK_RECV = 2
};

enum OPERATION_TYPE
{
	OP_NONE = 0,
	OP_RECV = 1,
	OP_SEND = 2
};

struct EXTENDED_RIO_BUF : public RIO_BUF
{
	OPERATION_TYPE operation;
};

/*
UDP socket wrapper class. If use it as client,Connect() Send() and
Recv()should be used. If use it as server, Bind(), Recvfrom() and Sendto()
should be used.
*/
class XUDPExSocket : public IUDPSocket
{
public:
	XUDPExSocket();
	~XUDPExSocket();
	
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
//	int32_t	Recv(unsigned char* recv_buf_, int32_t buf_len); 
	int32_t Recv(unsigned char* recv_buf_, int32_t buf_len); 

	void SetRIORecvBufSize(const uint32_t size);

private:
	XUDPExSocket(const XUDPExSocket&);
	XUDPExSocket& operator = (const XUDPExSocket&);
	SOCKET _socket;
	sockaddr_in _peer_serv;
	sockaddr_in _local_serv;
	bool _is_peer_set;
	bool _is_open;
	bool _is_bind;

private: //RIO Definitions
	EXTENDED_RIO_BUF* rioReceiveRequestBuffers = nullptr;
	
	CRITICAL_SECTION g_criticalSection;
	RIO_EXTENSION_FUNCTION_TABLE rio;
	RIO_CQ g_sendcompletionQueue;
	RIO_CQ g_recvcompletionQueue;
	RIO_RQ g_requestQueue;
	HANDLE g_hIOCP;

	RIO_BUFFERID g_sendBufferId;
	RIO_BUFFERID g_recvBufferId;
	RIO_BUFFERID g_addrBufferId;

	char* g_sendBufferPointer;
	char* g_recvBufferPointer;
	char* g_addrBufferPointer;

	DWORD RIO_PENDING_RECVS;
	DWORD RIO_PENDING_SENDS;

	DWORD RECV_BUFFER_SIZE;
	DWORD SEND_BUFFER_SIZE;

	DWORD ADDR_BUFFER_SIZE;

	DWORD NUM_IOCP_THREADS;

	//static const DWORD RIO_MAX_RESULTS = 1000;
	//RIORESULT results[RIO_MAX_RESULTS];

	/// SEND RIO_BUF
	EXTENDED_RIO_BUF* g_sendRioBufs;
	DWORD g_sendRioBufTotalCount;
	__int64 g_sendRioBufIndex;

	/// ADDR RIO_BUF pointer
	EXTENDED_RIO_BUF* g_addrRioBufs;
	DWORD g_addrRioBufTotalCount;
	__int64 g_addrRioBufIndex;

	queue<EXTENDED_RIO_BUF*> all_results;

	int InitSocket();
	char *AllocateBufferSpace(const DWORD bufSize, const DWORD bufCount, DWORD& totalBufferSize, DWORD& totalBufferCount);

	template <typename TV, typename TM>
	inline TV RoundDown(TV Value, TM Multiple)
	{
		return((Value / Multiple) * Multiple);
	}

	template <typename TV, typename TM>
	inline TV RoundUp(TV Value, TM Multiple)
	{
		return(RoundDown(Value, Multiple) + (((Value % Multiple) > 0) ? Multiple : 0));
	}
};

#endif