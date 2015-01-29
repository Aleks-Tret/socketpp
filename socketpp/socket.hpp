#pragma once

#if defined(_WIN32) && !defined(__INTIME__)
# include <WinSock2.h>
# include <ws2tcpip.h>
# include <io.h>
  static const int BOTH_DIRECTION=SD_BOTH;
# define CHECK_STATUS(st) if ((st) != 0) goto error;
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
  static const int BOTH_DIRECTION=SHUT_RDWR;
  static int const INVALID_SOCKET = -1;
  typedef int SOCKET;
# define CHECK_STATUS(s) if ((s) < 0 ) goto error;
# ifndef __INTIME__
#   define closesocket(s) ::close((s))
# endif
#endif
#define CHECK_SOCKET(so) if ((so) == INVALID_SOCKET) goto error;

#include <socketpp/exception.hpp>

#include <string>
#include <atomic>

#ifdef _WIN32
# pragma warning(disable:4290)
#endif

namespace socketpp {

  class Socket {
    public:
      explicit Socket(SOCKET const & socket = INVALID_SOCKET);
      Socket(int const port, int const type);
      Socket(Socket const& ) = delete;
      Socket& operator=(Socket const &) = delete;
      virtual ~Socket();

      void write(std::string&&);
      std::string read();

      

    protected:
      // We do not protect access to Socket commands using mutex (only to socket value) because the only command that
      // can be used from other thread is close, in order to stop thread used for communication
      std::atomic<SOCKET> socket_;

      void close();
  };

  int report_socket_error();
}

