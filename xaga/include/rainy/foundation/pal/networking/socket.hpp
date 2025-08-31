#ifndef RAINY_FOUNDATION_PAL_NETWORKING_SOCKET_HPP
#define RAINY_FOUNDATION_PAL_NETWORKING_SOCKET_HPP
#include <rainy/core/core.hpp>
#include <stdio.h>
#include <string.h>

namespace rainy::foundation::pal::networking::implements {
    struct sock_address {
        unsigned short sa_family;
        char sa_data[14];
    };

     struct inte_addr {
         std::uint32_t address;
     };

     using sock_address_family_t = short;
     using inte_port_t = short;

     struct sock_address_in {
         sock_address_family_t sin_family; // 地址族（AF_INET）
         inte_port_t sin_port; // 端口号（网络字节序）
         inte_addr sin_addr; // IP地址
         char sin_zero[8]; // 填充字节，未使用，一般置0
     };

    RAINY_TOOLKIT_API int sock_init();
    RAINY_TOOLKIT_API void sock_cleanup();
    RAINY_TOOLKIT_API core::handle sock_create(int domain, int type, int protocol);
    RAINY_TOOLKIT_API int sock_close(core::handle sock);
    RAINY_TOOLKIT_API int sock_bind(core::handle sock, const sock_address *addr, int addrlen);
    RAINY_TOOLKIT_API int sock_listen(core::handle sock, int backlog);
    RAINY_TOOLKIT_API core::handle sock_accept(core::handle sock, sock_address *addr, int *addrlen);
    RAINY_TOOLKIT_API int sock_connect(core::handle sock, const sock_address *addr, int addrlen);
    RAINY_TOOLKIT_API int sock_send(core::handle sock, const void *buf, int len, int flags);
    RAINY_TOOLKIT_API int sock_recv(core::handle sock, void *buf, int len, int flags);
    RAINY_TOOLKIT_API int sock_sendto(core::handle sock, const void *buf, int len, int flags, const sock_address *dest_addr,
                                      int addrlen);
    RAINY_TOOLKIT_API int sock_recvfrom(core::handle sock, void *buf, int len, int flags, sock_address *src_addr, int *addrlen);
    RAINY_TOOLKIT_API int sock_set_nonblocking(core::handle sock, int mode);
    RAINY_TOOLKIT_API int sock_get_last_error();
    RAINY_TOOLKIT_API int sock_shutdown(core::handle sock, int how);
}

#endif