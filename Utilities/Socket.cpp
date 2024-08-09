#include "pch.h"
#include <cstring>
#include <thread>
#include "Utilities/Socket.h"
#include "Utilities/UPnPPortMapper.h"
using namespace std;

#ifdef _WIN32
	#pragma comment(lib,"ws2_32.lib") //Winsock Library
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <Ws2tcpip.h>
	#include <Windows.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
	#include <unistd.h>

	#define INVALID_SOCKET (uintptr_t)-1
	#define SOCKET_ERROR -1
	#define WSAGetLastError() errno
	#define SOCKADDR_IN sockaddr_in
	#define SOCKADDR sockaddr
	#define TIMEVAL timeval
	#define SD_SEND SHUT_WR
	#define closesocket close
	#define WSAEWOULDBLOCK EWOULDBLOCK
	#define ioctlsocket ioctl
#endif

Socket::Socket()
{
	#ifdef _WIN32	
		WSADATA wsaDat;
		if(WSAStartup(MAKEWORD(2, 2), &wsaDat) != 0) {
			std::cout << "WSAStartup failed." << std::endl;
			SetConnectionErrorFlag();
			return;
		}
		_cleanupWSA = true;
	#endif

	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_socket == INVALID_SOCKET) {
		std::cout << "Socket creation failed." << std::endl;
		SetConnectionErrorFlag();
	} else {
		SetSocketOptions();
	}
}

Socket::Socket(uintptr_t socket) 
{
	_socket = socket;

	if(socket == INVALID_SOCKET) {
		SetConnectionErrorFlag();
	} else {
		SetSocketOptions();
	}
}

Socket::~Socket()
{
	if(_UPnPPort != -1) {
		UPnPPortMapper::RemoveNATPortMapping(_UPnPPort, IPProtocol::TCP);
	}

	if(_socket != INVALID_SOCKET) {
		Close();
	}

	#ifdef _WIN32
		if(_cleanupWSA) {
			WSACleanup();
		}
	#endif
}

void Socket::SetSocketOptions()
{
	//Non-blocking mode
	u_long iMode = 1;
	ioctlsocket(_socket, FIONBIO, &iMode);
		
	//Set send/recv buffers to 256k
	int bufferSize = 0x40000;
	setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, sizeof(int));
	setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, sizeof(int));

	//Disable nagle's algorithm to improve latency
	u_long value = 1;
	setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value));	
}

void Socket::SetConnectionErrorFlag()
{
	_connectionError = true;
}

void Socket::Close()
{
	std::cout << "Socket closed." << std::endl;
	shutdown(_socket, SD_SEND);
	closesocket(_socket);
	SetConnectionErrorFlag();
}

bool Socket::ConnectionError()
{
	return _connectionError;
}

void Socket::Bind(uint16_t port)
{
	SOCKADDR_IN serverInf;
	serverInf.sin_family = AF_INET;
	serverInf.sin_addr.s_addr = INADDR_ANY;
	serverInf.sin_port = htons(port);

	if(UPnPPortMapper::AddNATPortMapping(port, port, IPProtocol::TCP)) {
		_UPnPPort = port;
	}

	if(::bind(_socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR) {
		std::cout << "Unable to bind socket." << std::endl;
		SetConnectionErrorFlag();
	}
}

bool Socket::Connect(const char* hostname, uint16_t port)
{
	// Resolve IP address for hostname
	bool result = false;
	addrinfo hint;
	memset((void*)&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_protocol = IPPROTO_TCP;
	hint.ai_socktype = SOCK_STREAM;
	addrinfo *addrInfo;

	if(getaddrinfo(hostname, std::to_string(port).c_str(), &hint, &addrInfo) != 0) {
		std::cout << "Failed to resolve hostname." << std::endl;
		SetConnectionErrorFlag();
	} else {
		//Set socket in non-blocking mode
		u_long iMode = 1;
		ioctlsocket(_socket, FIONBIO, &iMode);

		// Attempt to connect to server
		connect(_socket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);

		fd_set writeSockets;
		#ifdef _WIN32
			writeSockets.fd_count = 1;
			writeSockets.fd_array[0] = _socket;
		#else		
			FD_ZERO(&writeSockets);
    		FD_SET(_socket, &writeSockets);
		#endif		

		//Timeout after 3 seconds
		TIMEVAL timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		// check if the socket is ready
		int returnVal = select((int)_socket+1, nullptr, &writeSockets, nullptr, &timeout);
		if(returnVal > 0) {
			result = true;
		} else {
			//Could not connect
			if(returnVal == SOCKET_ERROR) {
				//int nError = WSAGetLastError();				
				//std::cout << "select failed: nError " << std::to_string(nError) << " returnVal" << std::to_string(returnVal) << std::endl;			
			}
			SetConnectionErrorFlag();
		}
		
		freeaddrinfo(addrInfo);
	}

	return result;
}

void Socket::Listen(int backlog)
{
	if(listen(_socket, backlog) == SOCKET_ERROR) {
		std::cout << "listen failed." << std::endl;
		SetConnectionErrorFlag();
	}
}

unique_ptr<Socket> Socket::Accept()
{
	uintptr_t socket = accept(_socket, nullptr, nullptr);
	return unique_ptr<Socket>(new Socket(socket));
}

bool WouldBlock(int nError)
{
	return nError == WSAEWOULDBLOCK || nError == EAGAIN;
}

int Socket::Send(char *buf, int len, int flags)
{
	int retryCount = 100;
	int nError = 0;
	int returnVal;
	do {
		//Loop until everything has been sent (shouldn't loop at all in the vast majority of cases)
		returnVal = send(_socket, buf, len, flags);

		if(returnVal > 0) {
			//Sent partial data, adjust pointer & length
			buf += returnVal;
			len -= returnVal;
		} else if(returnVal == SOCKET_ERROR) {
			nError = WSAGetLastError();
			if(nError != 0) {
				if(!WouldBlock(nError)) {
					SetConnectionErrorFlag();
				} else {
					retryCount--;
					if(retryCount == 0) {
						//Connection seems dead, close it.
						std::cout << "Unable to send data, closing socket." << std::endl;
						Close();
						return 0;
					}
					
					std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(20));
				}
			}
		}
	} while(WouldBlock(nError) && len > 0);
		
	return returnVal;
}

int Socket::Recv(char *buf, int len, int flags)
{
	int returnVal = recv(_socket, buf, len, flags);
	
	if(returnVal == SOCKET_ERROR) {
		int nError = WSAGetLastError();
		if(nError && !WouldBlock(nError)) {
			std::cout << "recv failed: nError " << std::to_string(nError) << " returnVal" << std::to_string(returnVal) << std::endl;			
			SetConnectionErrorFlag();
		}
	} else if(returnVal == 0) {
		//Socket closed
		std::cout << "Socket closed by peer." << std::endl;
		Close();
	}

	return returnVal;
}
