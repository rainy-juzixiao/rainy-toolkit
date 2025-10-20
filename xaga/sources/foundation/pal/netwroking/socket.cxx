#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/networking/socket.hpp>
#if RAINY_USING_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define closesocket close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

namespace rainy::foundation::pal::networking::implements {
    int sock_init() {
#if RAINY_USING_WINDOWS
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(2, 2), &wsa_data);
#else
        return 0;
#endif
    }

    void sock_cleanup() {
#if RAINY_USING_WINDOWS
        WSACleanup();
#endif
    }

    core::handle sock_create(int domain, int type, int protocol) {
        return socket(domain, type, protocol);
    }

    int sock_close(core::handle sock) {
        return closesocket(sock);
    }

    int sock_bind(core::handle sock, const sock_address *addr, int addrlen) {
        return bind(sock, reinterpret_cast<const sockaddr *>(addr), addrlen);
    }

    int sock_listen(core::handle sock, int backlog) {
        return listen(sock, backlog);
    }

    core::handle sock_accept(core::handle sock, sock_address *addr, int *addrlen) {
#if RAINY_USING_WINDOWS
        return accept(sock, reinterpret_cast<sockaddr *>(addr), addrlen);
#else
        socklen_t len = *addrlen;
        core::handle result = accept(sock, reinterpret_cast<sockaddr *>(addr), &len);
        *addrlen = len;
        return result;
#endif
    }

    int sock_connect(core::handle sock, const sock_address *addr, int addrlen) {
        return connect(sock, reinterpret_cast<const sockaddr *>(addr), addrlen);
    }

    int sock_send(core::handle sock, const void *buf, int len, int flags) {
        return send(sock, reinterpret_cast<const char *>(buf), len, flags);
    }

    int sock_recv(core::handle sock, void *buf, int len, int flags) {
        return recv(sock, (char *) buf, len, flags);
    }

    int sock_sendto(core::handle sock, const void *buf, int len, int flags, const sock_address *dest_addr, int addrlen) {
        return sendto(sock, reinterpret_cast<const char *>(buf), len, flags, reinterpret_cast<const sockaddr *>(dest_addr), addrlen);
    }

    int sock_recvfrom(core::handle sock, void *buf, int len, int flags, sock_address *src_addr, int *addrlen) {
#if RAINY_USING_WINDOWS
        return recvfrom(sock, reinterpret_cast<char *>(buf), len, flags, reinterpret_cast<sockaddr *>(src_addr), addrlen);
#else
        socklen_t len_t = *addrlen;
        int result = recvfrom(sock, buf, len, flags, reinterpret_cast<sockaddr *>(src_addr), &len_t);
        *addrlen = len_t;
        return result;
#endif
    }

    int sock_set_nonblocking(core::handle sock, int mode) {
#if RAINY_USING_WINDOWS
        // mode: 1 为非阻塞，0 为阻塞
        return ioctlsocket(sock, FIONBIO, reinterpret_cast<unsigned long *>(&mode));
#else
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) {
            return -1;
        }
        if (mode) {
            return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        } else {
            return fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
        }
#endif
    }

    int sock_get_last_error() {
#if RAINY_USING_WINDOWS
        return WSAGetLastError();
#else
        return errno;
#endif
    }

    int sock_shutdown(core::handle sock, int how) {
#if RAINY_USING_WINDOWS
        return shutdown(sock, how);
#else
        switch (how) {
            case SHUT_WR:
                return shutdown(sock, SHUT_WR);
            case SHUT_RD:
                return shutdown(sock, SHUT_RD);
            case SHUT_RDWR:
                return shutdown(sock, SHUT_RDWR);
            default:
                return -1;
        }
#endif
    }
}
