#pragma once

#if defined(_WIN32) && !defined(__INTIME__)
  #include <WinSock2.h>
  #include <ws2tcpip.h>
  #include <io.h>
  static const int BOTH_DIRECTION=SD_BOTH;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  static const int BOTH_DIRECTION=SHUT_RDWR;
  static int const INVALID_SOCKET = -1;
  typedef int SOCKET;

  #ifndef __INTIME__
    #define closesocket(s) ::close((s))
  #endif
#endif

#include <socketpp/exception.hpp>

#include <string>
#include <mutex>

#ifdef _WIN32
  #pragma warning(disable:4290)
#endif

namespace socketpp {

  class Socket {
    public:
      explicit Socket(SOCKET const & socket = INVALID_SOCKET);
      Socket(Socket const& ) = delete;
      Socket& operator=(Socket const &) = delete;
      virtual ~Socket();

      void close();
      void write(std::string) throw (SocketException);
    void set_non_blocking(bool v);
    std::string read();

      bool closed();

    protected:
      SOCKET socket_;
      std::mutex socket_mutex_;
  };
}
