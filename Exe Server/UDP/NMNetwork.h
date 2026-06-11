#pragma once

#ifndef _WIN32

#include "../Win32Compat.h"
#include "Socket.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

inline int closesocket(SOCKET s) {
    return ::close(s);
}

inline int WSAStartup(WORD, void *) {
    return 0;
}

inline void WSACleanup() {}

inline bool NM_SameIPv4(const sockaddr_in &a, const sockaddr_in &b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr;
}

inline int NM_BindUdpSocket(const char *address, int port, NMSocket *&outSock, sockaddr_in &localAddr) {
    SOCKET fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == INVALID_SOCKET) {
        return -1;
    }
    int optval = 1;
    int bufsize = (1024 * 1024) * 10;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    ::setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
    ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    std::memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(static_cast<uint16_t>(port));
    localAddr.sin_addr.s_addr = ::inet_addr(address ? address : "0.0.0.0");
    if (::bind(fd, reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr)) != 0) {
        closesocket(fd);
        return -1;
    }
    outSock = new NMSocket();
    outSock->Init(fd);
    return 0;
}

#endif /* !_WIN32 */
