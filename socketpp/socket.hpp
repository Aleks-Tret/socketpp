#pragma once

#if defined(_WIN32)
# if !defined(__INTIME__)
#  include <WinSock2.h>
#  include <ws2tcpip.h>
#  include <io.h>
   static const int BOTH_DIRECTION=SD_BOTH;
#  define CHECK_STATUS(st) if ((st) != 0) goto error;
# endif
#  pragma warning(disable:4290)
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
# define closesocket(s) close((s))
#endif
#define CHECK_SOCKET(so) if ((so) == INVALID_SOCKET) goto error;

#include <socketpp/exception.hpp>
#include <socketpp/unique_handler.hpp>

#include <string>
#include <mutex>
#include <memory>


struct SocketDeleter
{
  typedef UniqueHandle<SOCKET, INVALID_SOCKET> pointer;
  void operator()(pointer p) {
    closesocket(p);
    shutdown(p, BOTH_DIRECTION);
    p = INVALID_SOCKET;
  }
};

typedef std::unique_ptr<SOCKET, SocketDeleter> unique_SOCKET;

namespace socketpp {

  class Socket {
    public:
      explicit Socket(SOCKET const & socket = INVALID_SOCKET);
      Socket(int const port, int const type);
      Socket(Socket const& ) = delete;
      Socket& operator=(Socket const &) = delete;
      virtual ~Socket() = default;

      void write(std::string&&);
      std::string read();

      SOCKET accept();

    protected:
      std::mutex socket_mutex_;
      unique_SOCKET socket_;
  };
}

