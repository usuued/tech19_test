#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

// Platform compatibility layer for Windows/Linux

#ifdef _WIN32
    // Windows includes
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    // Link with Winsock library
    #pragma comment(lib, "ws2_32.lib")

    // Windows compatibility types
    typedef int socklen_t;

    // Windows doesn't have these functions, provide alternatives
    #define htobe64(x) _byteswap_uint64(x)
    #define be64toh(x) _byteswap_uint64(x)
    #define htons(x) htons(x)
    #define ntohs(x) ntohs(x)

    // Close socket
    #define close(fd) closesocket(fd)

    // Error codes
    #define EINTR WSAEINTR

    // Initialize Winsock
    inline bool init_networking() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }

    inline void cleanup_networking() {
        WSACleanup();
    }

#else
    // Linux includes
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <endian.h>
    #include <cerrno>

    inline bool init_networking() { return true; }
    inline void cleanup_networking() {}
#endif

#endif // PLATFORM_COMPAT_H
