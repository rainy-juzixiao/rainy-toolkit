#include <rainy/core/core.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#ifdef RAINY_USING_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <io.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

#else
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif

namespace rainy::foundation::system::network {
	int get_socket_errno() noexcept {
#if RAINY_USING_WINDOWS
		return WSAGetLastError();
#else
		return errno;
#endif
	}

	enum ws_status {
#if RAINY_USING_WINDOWS
		eagin_einprogress = WSAEINPROGRESS,
		ewouldblock = WSAEWOULDBLOCK
#else
		eagin_einprogress = EAGAIN,
		ewouldblock = EWOULDBLOCK
#endif
	};
}